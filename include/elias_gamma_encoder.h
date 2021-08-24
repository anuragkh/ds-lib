#ifndef ELIAS_GAMMA_ENCODED_ARRAY_H_
#define ELIAS_GAMMA_ENCODED_ARRAY_H_

#include <vector>
#include <iostream>

#include "bit_vector.h"
#include "compact_vector.h"
#include "utils.h"

namespace bits {

template<typename T>
class EliasGammaEncoder {
 public:
  typedef size_t size_type;
  typedef size_t pos_type;
  typedef uint8_t width_type;

  static width_type EncodingSize(T val) {
    return 2 * (Utils::BitWidth(val) - 1) + 1;
  }

  static void Encode(BitVector &out, pos_type *pos, T val) {
    uint64_t nbits = Utils::BitWidth(val) - 1;
    (*pos) += nbits;
    assert((1ULL << nbits) <= val);
    out.SetBit((*pos)++);
    out.SetValPos(*pos, val - (1ULL << nbits), nbits);
    (*pos) += nbits;
  }

  static BitVector EncodeArray(std::vector<T> &in) {
    size_type out_size = 0;
    for (size_t i = 0; i < in.size(); i++) {
      assert(in[i] > 0);
      out_size += EncodingSize(in[i]);
    }
    BitVector out(out_size);
    pos_type pos = 0;
    for (size_t i = 0; i < in.size(); i++) {
      Encode(out, &pos, in[i]);
    }
    return out;
  }

  static T Decode(BitVector &in, pos_type *pos) {
    width_type val_width = 0;
    while (!in.GetBit(*pos)) {
      val_width++;
      (*pos)++;
    }
    (*pos)++;
    T decoded = in.GetValPos(*pos, val_width) + (1ULL << val_width);
    (*pos) += val_width;
    return decoded;
  }

  static std::vector<T> DecodeArray(BitVector &in) {
    std::vector<T> out;
    pos_type pos = 0;
    auto max_pos = in.GetSizeInBits();
    while (pos != max_pos) {
      out.push_back(Decode(in, &pos));
    }
    return out;
  }
};

}

#endif // ELIAS_GAMMA_ENCODED_ARRAY_H_
