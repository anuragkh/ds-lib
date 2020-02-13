#ifndef BITMAP_DICTIONARY_H_
#define BITMAP_DICTIONARY_H_

#include "bitmap.h"
#include "utils.h"

namespace bitmap {

static const uint32_t kCacheLineSize = 64;
static const uint64_t kL1BlockSize = 512ULL;
static const uint64_t kL2BlockSize = 2048ULL;
static const uint64_t kL3BlockSize = 4294967296ULL;
static const uint64_t kL1BlocksPerL2Block = 4ULL;
static const uint64_t kL1BlocksPerL3Block = 8388608ULL;
static const uint64_t kL2BlocksPerL3Block = 2097152ULL;

// Rank and select data structures based on "Poppy"
// "Space-Efficient, High-Performance Rank & Select Structures
// on Uncompressed Bit Sequences", Zhou et. al.
class Dictionary : public Bitmap {
 public:
  typedef uint64_t count_type;
  Dictionary() {
    rank_l12_ = NULL;
    rank_l3_ = NULL;
    pos_l12_ = NULL;
    pos_l3_ = NULL;
  }

  Dictionary(Bitmap& bitmap) {
    size_type l3_size = L3Size(bitmap.GetSizeInBits());
    size_type l2_size = L2Size(bitmap.GetSizeInBits());

    // Allocate rank data structures
    rank_l3_ = new data_type[l3_size];
    rank_l12_ = new data_type[l2_size];

    count_type l3_pop_count = 0;
    pos_type l2_id = 0, l1_id = 0;
    for (size_type i = 0; i < l3_size; i++) {
      rank_l3_[i] = l3_pop_count;

      count_type l2_pop_count = 0;
      for (uint64_t l2_idx = 0; l2_idx < kL2BlocksPerL3Block; l2_idx++) {
        rank_l12_[i] = l2_pop_count;
        count_type l1_pop_count;
        for (size_type l2_offset = 0; l2_offset < 30; l2_offset++) {
          l1_pop_count = Utils::Popcount512bit(bitmap.GetData() + (l1_id * 8));
          l2_pop_count += l1_pop_count;
          l1_id++;
          rank_l12_[i] |= ((uint64_t) l1_pop_count) << (32 + l2_offset);
        }
        l1_pop_count = Utils::Popcount512bit(bitmap.GetData() + (l1_id * 8));
        l2_pop_count += l1_pop_count;
        l1_id++;
        l2_id++;
        if (l2_id >= l2_size)
          break;
      }
      l3_pop_count += l2_pop_count;
    }
  }

  count_type rank1(pos_type i) {
    pos_type l3_id = i >> 32;
    pos_type l2_id = i >> 11;
    pos_type l1_id = (i & 0x7FF) >> 9;

    // Compute L3 & L2 ranks
    count_type rank_value = rank_l3_[l3_id]
        + (rank_l12_[l2_id] & low_bits_set[32]);

    // Compute L1 rank
    count_type l1_values = rank_l12_[l2_id] >> 32;
    for (uint64_t i = 0; i < l1_id; i++) {
      rank_value += l1_values & 0x3FF;
      l1_values >>= 10;
    }

    // Compute rank within L1 block
    // TODO:

    return rank_value;
  }

  count_type rank0(pos_type i) {
    return i - rank1(i) - 1;
  }

 private:
  size_type L3Size(size_type bitmap_size) {
    return (bitmap_size <= kL3BlockSize) ? 1 : bitmap_size >> 32;
  }

  size_type L2Size(size_type bitmap_size) {
    return bitmap_size >> 11;
  }

  // Rank data-structures
  data_type* rank_l12_;
  data_type* rank_l3_;
  data_type* pos_l12_;
  data_type* pos_l3_;
};

}
#endif // BITMAP_DICTIONARY_H_
