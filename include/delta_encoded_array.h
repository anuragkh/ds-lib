#ifndef BITMAP_DELTA_ENCODED_ARRAY_H_
#define BITMAP_DELTA_ENCODED_ARRAY_H_

#include <vector>

#include "bit_vector.h"
#include "compact_vector.h"
#include "elias_gamma_prefix_sum.h"
#include "utils.h"

#define USE_PREFIXSUM_TABLE 1

namespace bits {

template<typename T, uint32_t sampling_rate = 128>
class DeltaEncodedVector {
 public:
  typedef size_t size_type;
  typedef size_t pos_type;
  typedef uint8_t width_type;

  DeltaEncodedVector() = default;

  virtual ~DeltaEncodedVector() = default;

  // Serialization and De-serialization
  virtual size_type Serialize(std::ostream &out) {
    size_type out_size = 0;

    out_size += samples_.Serialize(out);
    out_size += delta_offsets_.Serialize(out);
    out_size += deltas_.Serialize(out);

    return out_size;
  }

  virtual size_type Deserialize(std::istream &in) {
    size_type in_size = 0;

    in_size += samples_.Deserialize(in);
    in_size += delta_offsets_.Deserialize(in);
    in_size += deltas_.Deserialize(in);

    return in_size;
  }

 protected:
  // Get the encoding size for an delta value
  virtual width_type EncodingSize(T delta) = 0;

  // Encode the delta values
  virtual void EncodeDeltas(T *deltas, size_type num_deltas) = 0;

  // Encode the delta encoded array
  void Encode(T *elements, size_type num_elements) {
    if (num_elements == 0) {
      return;
    }

#ifdef DEBUG
    assert(std::is_sorted(elements, elements + num_elements));
#endif
    std::vector<T> samples, deltas;
    std::vector<pos_type> delta_offsets;
    T last_val = 0;
    uint64_t tot_delta_count = 0, delta_count = 0;
    uint64_t delta_enc_size;
    size_type cum_delta_size = 0;

    for (size_t i = 0; i < num_elements; i++) {
      if (i % sampling_rate == 0) {
        samples.push_back(elements[i]);
        delta_offsets.push_back(cum_delta_size);
        if (i != 0) {
          assert(delta_count == sampling_rate - 1);
          tot_delta_count += delta_count;
          delta_count = 0;
        }
      } else {
        assert(elements[i] > last_val);
        T delta = elements[i] - last_val;
        deltas.push_back(delta);

        delta_enc_size = EncodingSize(delta);
        cum_delta_size += delta_enc_size;
        delta_count++;
      }
      last_val = elements[i];
    }
    tot_delta_count += delta_count;

    assert(tot_delta_count == deltas.size());
    assert(samples.size() + deltas.size() == num_elements);
    assert(delta_offsets.size() == samples.size());

    if (samples.size() != 0) {
      samples_.Init(&samples[0], samples.size());
    }

    if (cum_delta_size != 0) {
      deltas_.Init(cum_delta_size);
      EncodeDeltas(&deltas[0], deltas.size());
    }

    if (!delta_offsets.empty()) {
      delta_offsets_.Init(&delta_offsets[0], delta_offsets.size());
    }
  }

  CompactVector<T, std::numeric_limits<T>::digits> samples_;
  CompactVector<pos_type, std::numeric_limits<pos_type>::digits> delta_offsets_;
  BitVector deltas_;

 private:
};

template<typename T, uint32_t sampling_rate = 128>
class EliasGammaDeltaEncodedVector : public DeltaEncodedVector<T, sampling_rate> {
 public:
  typedef typename DeltaEncodedVector<T>::size_type size_type;
  typedef typename DeltaEncodedVector<T>::pos_type pos_type;
  typedef typename DeltaEncodedVector<T>::width_type width_type;

  using DeltaEncodedVector<T>::EncodingSize;
  using DeltaEncodedVector<T>::EncodeDeltas;

  EliasGammaDeltaEncodedVector()
      : DeltaEncodedVector<T>() {
  }

  EliasGammaDeltaEncodedVector(T *elements, size_type num_elements)
      : EliasGammaDeltaEncodedVector<T>() {
    this->Encode(elements, num_elements);
  }

  virtual ~EliasGammaDeltaEncodedVector() = default;

  T Get(pos_type i) {
    // Get offsets
    pos_type samples_idx = i / sampling_rate;
    pos_type delta_offsets_idx = i % sampling_rate;
    T val = this->samples_.Get(samples_idx);

    if (delta_offsets_idx == 0)
      return val;

    pos_type delta_offset = this->delta_offsets_.Get(samples_idx);
    val += PrefixSum(delta_offset, delta_offsets_idx);
    return val;
  }

  T operator[](pos_type i) {
    return Get(i);
  }

  bool Find(T val, pos_type *found_idx = nullptr) {
    pos_type sample_off = this->samples_.LowerBound(val);
    pos_type current_delta_offset = this->delta_offsets_.Get(sample_off);
    val -= this->samples_.Get(sample_off);

    pos_type delta_idx = 0;
    T delta_sum = 0;
    size_type delta_max = this->deltas_.GetSizeInBits();

    while (delta_sum < val && current_delta_offset < delta_max && delta_idx < sampling_rate) {
      uint16_t block = this->deltas_.GetValPos(current_delta_offset, 16);
      uint16_t block_cnt = elias_gamma_prefix_table.count(block);
      uint16_t block_sum = elias_gamma_prefix_table.sum(block);

      if (block_cnt == 0) {
        // If the prefixsum table for the block returns count == 0
        // this must mean the value spans more than 16 bits
        // read this manually
        uint8_t delta_width = 0;
        while (!this->deltas_.GetBit(current_delta_offset)) {
          delta_width++;
          current_delta_offset++;
        }
        current_delta_offset++;
        auto decoded_value = this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);

        delta_sum += decoded_value;
        current_delta_offset += delta_width;
        delta_idx += 1;

        // Roll back
        if (delta_idx == sampling_rate) {
          delta_idx--;
          delta_sum -= decoded_value;
          break;
        }
      } else if (delta_sum + block_sum < val) {
        // If sum can be computed from the prefixsum table
        delta_sum += block_sum;
        current_delta_offset += elias_gamma_prefix_table.offset(block);
        delta_idx += block_cnt;
      } else {
        // Last few values, decode them without looking up table
        T last_decoded_value = 0;
        while (delta_sum < val && current_delta_offset < delta_max && delta_idx < sampling_rate) {
          int delta_width = 0;
          while (!this->deltas_.GetBit(current_delta_offset)) {
            delta_width++;
            current_delta_offset++;
          }
          current_delta_offset++;
          last_decoded_value = this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);

          delta_sum += last_decoded_value;
          current_delta_offset += delta_width;
          delta_idx += 1;
        }

        // Roll back
        if (delta_idx == sampling_rate) {
          delta_idx--;
          delta_sum -= last_decoded_value;
          break;
        }
      }
    }

    if (found_idx) {
      pos_type res = sample_off * sampling_rate + delta_idx;
      *found_idx = (delta_sum <= val) ? res : res - 1;
    }
    
    return val == delta_sum;
  }

 private:
  width_type EncodingSize(T delta) override {
    return 2 * (Utils::BitWidth(delta) - 1) + 1;
  }

  void EncodeDeltas(T *deltas, size_type num_deltas) override {
    uint64_t pos = 0;
    for (size_t i = 0; i < num_deltas; i++) {
      uint64_t delta_bits = Utils::BitWidth(deltas[i]) - 1;
      pos += delta_bits;
      assert((1ULL << delta_bits) <= deltas[i]);
      this->deltas_.SetBit(pos++);
      this->deltas_.SetValPos(pos, deltas[i] - (1ULL << delta_bits), delta_bits);
      pos += delta_bits;
    }
  }

  T PrefixSum(pos_type delta_offset, pos_type until_idx) {
    T delta_sum = 0;
    pos_type delta_idx = 0;
    pos_type current_delta_offset = delta_offset;
    while (delta_idx != until_idx) {
      uint16_t block = this->deltas_.GetValPos(current_delta_offset, 16);
      uint16_t cnt = elias_gamma_prefix_table.count(block);
      if (cnt == 0) {
        // If the prefixsum table for the block returns count == 0
        // this must mean the value spans more than 16 bits
        // read this manually
        width_type delta_width = 0;
        while (!this->deltas_.GetBit(current_delta_offset)) {
          delta_width++;
          current_delta_offset++;
        }
        current_delta_offset++;
        delta_sum += this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);
        current_delta_offset += delta_width;
        delta_idx += 1;
      } else if (delta_idx + cnt <= until_idx) {
        // If sum can be computed from the prefixsum table
        delta_sum += elias_gamma_prefix_table.sum(block);
        current_delta_offset += elias_gamma_prefix_table.offset(block);
        delta_idx += cnt;
      } else {
        // Last few values, decode them without looking up table
        while (delta_idx != until_idx) {
          width_type delta_width = 0;
          while (!this->deltas_.GetBit(current_delta_offset)) {
            delta_width++;
            current_delta_offset++;
          }
          current_delta_offset++;
          delta_sum += this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);
          current_delta_offset += delta_width;
          delta_idx += 1;
        }
      }
    }
    return delta_sum;
  }
};

}

#endif // BITMAP_DELTA_ENCODED_ARRAY_H_
