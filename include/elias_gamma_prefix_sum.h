#ifndef ELIAS_GAMMA_PREFIX_SUM_H
#define ELIAS_GAMMA_PREFIX_SUM_H

#include <cstdint>
#include "utils.h"

namespace bits {

static struct EliasGammaPrefixSum {
 public:
  typedef uint16_t block_type;

  EliasGammaPrefixSum() {
    for (uint64_t i = 0; i < 65536; i++) {
      auto val = (uint16_t) i;
      uint64_t count = 0, offset = 0, sum = 0;
      while (val && offset <= 16) {
        int N = 0;
        while (!GETBIT(val, offset)) {
          N++;
          offset++;
        }
        offset++;
        if (offset + N <= 16) {
          sum += ((val >> offset) & ((uint16_t) low_bits_set[N])) + (1 << N);
          offset += N;
          count++;
        } else {
          offset -= (N + 1);
          break;
        }
      }
      prefixsum_[i] = (offset << 24) | (count << 16) | sum;
    }
  }

  uint8_t offset(const block_type i) const {
    return ((prefixsum_[(i)] >> 24) & 0xFF);
  }

  uint8_t count(const block_type i) const {
    return ((prefixsum_[i] >> 16) & 0xFF);
  }

  uint16_t sum(const block_type i) const {
    return (prefixsum_[i] & 0xFFFF);
  }

 private:
  uint32_t prefixsum_[65536]{};
} elias_gamma_prefix_table;

}
#endif //ELIAS_GAMMA_PREFIX_SUM_H
