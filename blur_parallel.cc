#include <iostream>
#include <string>
#include <cstdlib>
#include "qdbmp.h"

using std::cerr;
using std::endl;

struct ThreadArgs {
    BitMap* image;
    BitMap* blur_par;
    unsigned int width;
    unsigned int height;
    unsigned int box_size;
    int start_x;
    int end_x;
    int start_y;
    int end_y;
};

void* process_thread(void* arg);

RGB process_avg_color(BitMap& image, const int x, const int y, const int width, const int height, const int box_size);

pthread_mutex_t lock;

int main(int argc, char** argv) {
    // TODO: write your parallel version of the blur program in this file.
    // You should write your sequential version first

    // Check input commands
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <input file> <output file> <block size> <thread_count>" << endl;
        return EXIT_FAILURE;
    }

    // check box size is valid input
    const unsigned int box_size = atoi(argv[3]);
    if (box_size <= 0) {
        cerr << "invalid input, please make sure box size is greater than zero";
        return EXIT_FAILURE;
    }

    const unsigned int thread_count = atoi(argv[4]);
    if (thread_count <= 0) {
        cerr << "invalid input, please make sure thread count is greater than zero";
        return EXIT_FAILURE;
    }

    // Construct a BitMap object using the input file specified
    BitMap image(argv[1]);

    // Check the command above succeed
    if (image.check_error() != BMP_OK) {
        perror("ERROR: Failed to open BMP file.");
        return EXIT_FAILURE;
    }

    // Create a new BitMap for output the blur image
    const unsigned int height = image.height();
    const unsigned int width = image.width();
    BitMap blur_par(width, height);

    // Check the command above succeed
    if (blur_par.check_error() != BMP_OK) {
        perror("ERROR: Failed to open BMP file.");
        return EXIT_FAILURE;
    }

    // Create threads
    pthread_t* threads = new pthread_t[thread_count];
    ThreadArgs* thread_args = new ThreadArgs[thread_count];

    // Initialize mutex and condition variable
    pthread_mutex_init(&lock, NULL);


    int block_size = height / thread_count;
    int leftover_rows = height % thread_count;
    int current_row = 0;

    // Launch the threads
    for (size_t i = 0; i < thread_count; i++) {
        int rows = block_size + (leftover_rows-- > 0 ? 1 : 0);
        thread_args[i] = ThreadArgs { &image, &blur_par, width, height, box_size, 0, (int)width, current_row, current_row + rows};
        current_row += rows;
        pthread_create(&threads[i], NULL, process_thread, &thread_args[i]);
    }

    // Wait for the threads to finish
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy mutex and condition variable
    pthread_mutex_destroy(&lock);

    delete[] threads;
    delete[] thread_args;

    // Output the blur image to disk
    blur_par.write_file(argv[2]);

    if (blur_par.check_error() != BMP_OK) {
        perror("ERROR: Failed to open BMP file.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

RGB process_avg_color(BitMap& image, const int x, const int y, const int width, const int height, const int box_size) {
    int len = 0;
    int total_r = 0;
    int total_g = 0;
    int total_b = 0;
    for (int i = y - box_size; i <= y + box_size; i++) {
        for (int j = x - box_size; j <= x + box_size; j++) {
            if (i < 0 || i >= height || j < 0 || j >= width) {
                continue;
            }
            RGB c = image.get_pixel(j, i);
            total_r += c.r;
            total_g += c.g;
            total_b += c.b;
            len++;
        }
    }

    RGB res(total_r/len, total_g/len, total_b/len);

    return res;
}

void* process_thread(void* arg) {
    ThreadArgs* data = (ThreadArgs*) arg;
    BitMap* image = data->image;
    BitMap* blur_par = data->blur_par;
    int width = data->width;
    int height = data->height;
    int box_size = data->box_size;
    int start_x = data->start_x;
    int end_x = data->end_x;
    int start_y = data->start_y;
    int end_y = data->end_y;

    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            RGB avg_color = process_avg_color(*image, x, y, width, height, box_size);

            pthread_mutex_lock(&lock);
            blur_par->set_pixel(x, y, avg_color);
            pthread_mutex_unlock(&lock);
        }
    }

    pthread_exit(NULL);
}