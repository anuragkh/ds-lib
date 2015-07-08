#ifndef DSL_BITMAP_ARRAY_H_
#define DSL_BITMAP_ARRAY_H_

#include "bitmap.h"

namespace dsl {

class BitmapArray : public Bitmap {
 public:
  BitmapArray();
  virtual ~BitmapArray();
  BitmapArray(uint64_t num_elements, uint8_t bit_width);
  BitmapArray(uint64_t *elements, uint64_t num_elements, uint8_t bit_width);

  inline void insert(uint64_t i, uint64_t value);
  inline uint64_t at(uint64_t i);
  virtual uint64_t operator[](uint64_t i) = 0;

 private:
  uint64_t num_elements_;
  uint8_t bit_width_;
};

}

#endif
