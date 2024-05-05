#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

unsigned char mandelbrot(double real, double imag, int max_iter) {
    double z_real = real;
    double z_imag = imag;
    for (int n = 0; n < max_iter; n++) {
        double r2 = z_real * z_real;
        double i2 = z_imag * z_imag;
        if (r2 + i2 > 4.0) return n;
        z_imag = 2.0 * z_real * z_imag + imag;
        z_real = r2 - i2 + real;
    }
    return max_iter;
}

int main(int argc, char *argv[]) {
    int mpi_provided_thread_level;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        printf("The threading support level is lesser than that demanded\n");
        MPI_Finalize();
        exit(1);
    }

    double global_start_time = MPI_Wtime();

    int width = 800, height = 600;
    double x_left = -2.0, x_right = 1.0, y_lower = -1.0, y_upper = 1.0;
    int max_iterations = 255;
    int world_size, world_rank, num_threads;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc == 9) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        x_left = atof(argv[3]);
        y_lower = atof(argv[4]);
        x_right = atof(argv[5]);
        y_upper = atof(argv[6]);
        max_iterations = atoi(argv[7]);
        num_threads = atoi(argv[8]);
        omp_set_num_threads(num_threads);
    }

    int rows_per_process = height / world_size;
    int remainder_rows = height % world_size;
    int start_row = world_rank * rows_per_process;
    int end_row = start_row + rows_per_process + (world_rank == world_size - 1 ? remainder_rows : 0);

    unsigned char* part_buffer = (unsigned char*)malloc(width * (end_row - start_row) * sizeof(unsigned char));

    #pragma omp parallel for schedule(dynamic)
    for (int j = start_row; j < end_row; j++) {
        for (int i = 0; i < width; i++) {
            double x = x_left + i * (x_right - x_left) / width;
            double y = y_lower + j * (y_upper - y_lower) / height;
            int index = (j - start_row) * width + i;
            part_buffer[index] = mandelbrot(x, y, max_iterations);
        }
    }

    // Prepare for MPI_Gatherv
    int *recvcounts = NULL;
    int *displs = NULL;
    unsigned char *image_buffer = NULL;
    if (world_rank == 0) {
        recvcounts = malloc(world_size * sizeof(int));
        displs = malloc(world_size * sizeof(int));
        int displacement = 0;
        for (int i = 0; i < world_size; i++) {
            recvcounts[i] = width * (rows_per_process + (i == world_size - 1 ? remainder_rows : 0));
            displs[i] = displacement;
            displacement += recvcounts[i];
        }
        image_buffer = (unsigned char*)malloc(displacement * sizeof(unsigned char));
    }

    MPI_Gatherv(part_buffer, width * (end_row - start_row), MPI_UNSIGNED_CHAR,
                image_buffer, recvcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);
    free(part_buffer);

    if (world_rank == 0) {
        // Root process final operations here
        FILE *file = fopen("image.pgm", "w");
        fprintf(file, "P5\n%d %d\n255\n", width, height);
        fwrite(image_buffer, sizeof(unsigned char), width * height, file);
        fclose(file);
        //printf("Mandelbrot set generated and saved to 'image.pgm'\n");
        free(image_buffer);
        free(recvcounts);
        free(displs);
    }

    double global_end_time = MPI_Wtime();
    if (world_rank == 0) {
        printf("%f", global_end_time - global_start_time);
    }

    MPI_Finalize();
    return 0;
}