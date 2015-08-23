#ifndef BITMAP_UTILS_H_
#define BITMAP_UTILS_H_

namespace bitmap {

class Utils {
 public:
  static uint8_t BitWidth(uint64_t n) {
    uint8_t l = 1;
    while (n >>= 1)
      ++l;
    return l;
  }
};

}

#endif // BITMAP_UTILS_H_
