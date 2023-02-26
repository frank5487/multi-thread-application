#include <iostream>
#include <string>
#include <cstdlib>
#include "qdbmp.h"

using std::cerr;
using std::endl;

RGB process_avg_color(BitMap& image, const int x, const int y, const int width, const int height, const int box_size);

int main(int argc, char** argv) {
  // TODO: write your sequential version of the blur program in this file. 
  // You can mimic negative.cc to get started.

    // Check input commands
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input file> <output file> <block size>" << endl;
        return EXIT_FAILURE;
    }

    // check box size is valid input
    const unsigned int box_size = atoi(argv[3]);
    if (box_size <= 0) {
        cerr << "invalid input, please make sure box size is greater than zero";
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
    BitMap blur_seq(width, height);

    // Check the command above succeed
    if (blur_seq.check_error() != BMP_OK) {
        perror("ERROR: Failed to open BMP file.");
        return EXIT_FAILURE;
    }


    // Loop through each pixel
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {

            RGB avg_color = process_avg_color(image, x, y, width, height, box_size);
            blur_seq.set_pixel(x, y, avg_color);
        }
    }

    // Output the negative image to disk
    blur_seq.write_file(argv[2]);

    if (blur_seq.check_error() != BMP_OK) {
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
