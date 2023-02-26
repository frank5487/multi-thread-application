#ifndef QDBMP_H_
#define QDBMP_H_

#include <string>
#include <iostream>

// extern "C" needed to use something written in C
extern "C" {
  #include "cqdbmp.h"
}

/**
 * A struct to represent a RGB color.
 */ 
struct RGB {
  UCHAR r, g, b;
  RGB(UCHAR r, UCHAR g, UCHAR b) : r(r), g(g), b(b) {}
  void print() { std::cout << "(" << std::to_string( r ) << ", "<< std::to_string( g ) << ", "<< std::to_string( b ) << ")" << std::endl; }
};

/**
 * A class that represent a .bmp image. 
 * 
 * This class uses cqdbmp library to interact with the image, such as load/save a .bmp file,
 *   get/set a pixel in the .bmp file, and error checking. 
 * 
 * Note that all methods in this class may read/set error code in the cqdbmp library, 
 *   so you need to modify this class to support multi-threaded program.
 */
class BitMap {
 public:
  // constructors
  BitMap(UINT width, UINT height);
  BitMap(std::string file);
  ~BitMap();

  // getters
  UINT width();
  UINT height();
  RGB get_pixel(UINT x, UINT y);

  // setters
  void set_pixel(UINT x, UINT y, RGB rgb);

  // I/O
  void write_file(std::string file);

  // error
  BMP_STATUS check_error();

  // The two lines below the comments make it so
  // that if you want to pass a BitMap as a parameter
  // you must use a reference. You also cannot
  // assign one bitmap to be equal to another.
  // USE REFERENCES OR POINTERS.
  //
  // If you want to know more, this is disabling the
  // copy constructor and the assignment operator.
  // These will be covered later in the class
  BitMap(const BitMap& other) = delete;
  BitMap& operator=(const BitMap other) = delete;
 private:
  BMP *m_bmpPtr;
};

#endif  // QDBMP_H_