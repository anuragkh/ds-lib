#ifndef UTILS_H_
#define UTILS_H_

#include <cstdint>

/* Check if n is a power of 2 */
#define ISPOWOF2(n)     ((n != 0) && ((n & (n - 1)) == 0))

/* Pop-count Constants */
#define m1   0x5555555555555555 //binary: 0101...
#define m2   0x3333333333333333 //binary: 00110011..
#define m4   0x0f0f0f0f0f0f0f0f //binary:  4 zeros,  4 ones ...
#define m8   0x00ff00ff00ff00ff //binary:  8 zeros,  8 ones ...
#define m16  0x0000ffff0000ffff //binary: 16 zeros, 16 ones ...
#define m32  0x00000000ffffffff //binary: 32 zeros, 32 ones
#define hff  0xffffffffffffffff //binary: all ones
#define h01  0x0101010101010101 //the sum of 256 to the power of 0,1,2,3...

class Utils {
 public:
  // Returns the number of set bits in a 64 bit integer
  static uint64_t Popcount(uint64_t n) {
    // TODO: Add support for hardware instruction
    n -= (n >> 1) & m1;
    n = (n & m2) + ((n >> 2) & m2);
    n = (n + (n >> 4)) & m4;
    return (n * h01) >> 56;
  }

  // Returns integer logarithm to the base 2
  static uint32_t IntegerLog2(uint64_t n) {
    // TODO: Add support for hardware instruction
    uint32_t l = ISPOWOF2(n) ? 0 : 1;
    while (n >>= 1)
      ++l;
    return l;
  }

  // Returns a modulo n
  static uint64_t Modulo(int64_t a, uint64_t n) {
    while (a < 0)
      a += n;
    return a % n;
  }

  static uint64_t NumBlocks(uint64_t val, uint64_t block_size) {
    return (val % block_size == 0) ? (val / block_size) : (val / block_size) + 1;
  }
};

#endif // UTILS_H
