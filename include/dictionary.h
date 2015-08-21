#ifndef BITMAP_DICTIONARY_H_
#define BITMAP_DICTIONARY_H_

#include <cassert>
#include <map>
#include <vector>

#include "bitmap.h"
#include "utils.h"

namespace bitmap {

/* Get rank operations */
#define GETRANKL2(n)    (n >> 32)
#define GETRANKL1(n, i) (((n & 0xffffffff) >> (32 - i * 10)) & 0x3ff)

#define GETPOSL2(n)     (n >> 31)
#define GETPOSL1(n, i)  (((n & 0x7fffffff) >> (31 - i * 10)) & 0x3ff)

/* Rank Data structure constants */

/* Constant tables used for encoding/decoding
 * dictionary and delta encoded vector */
static struct table {
 public:
  table() {
    uint16_t q[17];

    C16[0] = 1;
    offbits[0] = 1;
    C16[1] = 16;
    offbits[1] = 4;
    C16[2] = 120;
    offbits[2] = 7;
    C16[3] = 560;
    offbits[3] = 10;
    C16[4] = 1820;
    offbits[4] = 11;
    C16[5] = 4368;
    offbits[5] = 13;
    C16[6] = 8008;
    offbits[6] = 13;
    C16[7] = 11440;
    offbits[7] = 14;
    C16[8] = 12870;
    offbits[8] = 14;

    for (uint32_t i = 0; i <= 16; i++) {
      if (i > 8) {
        C16[i] = C16[16 - i];
        offbits[i] = offbits[16 - i];
      }
      decode_table[i] = new uint16_t[C16[i]];
      q[i] = 0;
    }
    q[16] = 1;
    for (uint64_t i = 0; i <= 65535; i++) {
      uint64_t p = Utils::Popcount(i);
      decode_table[p % 16][q[p]] = (uint16_t) i;
      encode_table[p % 16][i] = q[p];
      q[p]++;
      for (uint64_t j = 0; j < 16; j++) {
        smallrank[i][j] = (uint8_t) Utils::Popcount(i >> (15 - j));
      }
    }
  }

  uint16_t *decode_table[17];
  std::map<uint16_t, uint16_t> encode_table[17];
  uint16_t C16[17];
  uint8_t offbits[17];
  uint8_t smallrank[65536][16];
} dictionary_coder;

class Dictionary {
 public:
  static const uint64_t kL1BlockSize = 512ULL;
  static const uint64_t kL2BlockSize = 2048ULL;
  static const uint64_t kL3BlockSize = 4294967296ULL;

  Dictionary();
  Dictionary(const Bitmap &B);
  ~Dictionary();

  uint64_t Rank0(uint64_t i);
  uint64_t Rank1(uint64_t i);

  uint64_t Select0(uint64_t i);
  uint64_t Select1(uint64_t i);

  size_t Serialize(std::ostream& out);
  size_t Deserialize(std::istream& in);

 private:
  Bitmap *bitmap_;
  uint64_t size_;
  uint64_t *rank_l12_;
  uint64_t *rank_l3_;
  uint64_t *pos_l12_;
  uint64_t *pos_l3_;
};

Dictionary::Dictionary() {
  size_ = 0;
  rank_l12_ = NULL;
  rank_l3_ = NULL;
  pos_l12_ = NULL;
  pos_l3_ = NULL;
  bitmap_ = NULL;
}

Dictionary::Dictionary(const Bitmap &bitmap) {
  uint64_t l3_size = (bitmap.size_ / kL3BlockSize) + 1;
  uint64_t l2_size = (bitmap.size_ / kL2BlockSize) + 1;
  uint64_t l1_size = (bitmap.size_ / kL1BlockSize) + 1;
  uint64_t count = 0;

  size_ = bitmap.size_;
  rank_l3_ = new uint64_t[l3_size];
  pos_l3_ = new uint64_t[l3_size];
  uint64_t *rank_l2 = new uint64_t[l2_size];
  uint64_t *rank_l1 = new uint64_t[l1_size];
  uint64_t *pos_l2 = new uint64_t[l2_size];
  uint64_t *pos_l1 = new uint64_t[l1_size];
  rank_l12_ = new uint64_t[l2_size];
  pos_l12_ = new uint64_t[l2_size];

  std::vector<uint16_t> dict;
  uint64_t sum_l1 = 0, sum_pos_l1 = 0, i, p, size = 0;
  uint32_t flag = 0;

  rank_l3_[0] = 0;
  pos_l3_[0] = 0;
  rank_l2[0] = 0;
  rank_l1[0] = 0;

  for (i = 0; i < bitmap.size_; i++) {
    if (i % kL3BlockSize == 0) {
      rank_l3_[i / kL3BlockSize] = count;
      pos_l3_[i / kL3BlockSize] = size;
    }
    if (i % kL2BlockSize == 0) {
      rank_l2[i / kL2BlockSize] = count - rank_l3_[i / kL3BlockSize];
      pos_l2[i / kL2BlockSize] = size - pos_l3_[i / kL3BlockSize];
      rank_l12_[i / kL2BlockSize] = rank_l2[i / kL2BlockSize] << 32;
      pos_l12_[i / kL2BlockSize] = pos_l2[i / kL2BlockSize] << 31;
      flag = 0;
      sum_l1 = 0;
      sum_pos_l1 = 0;
    }
    if (i % kL1BlockSize == 0) {
      rank_l1[i / kL1BlockSize] = count - rank_l2[i / kL2BlockSize] - sum_l1;
      pos_l1[i / kL1BlockSize] = size - pos_l2[i / kL2BlockSize] - sum_pos_l1;
      sum_l1 += rank_l1[i / kL1BlockSize];
      sum_pos_l1 += pos_l1[i / kL1BlockSize];
      if (flag != 0) {
        rank_l12_[i / kL2BlockSize] |= (rank_l1[i / kL1BlockSize]
            << (32 - flag * 10));
        pos_l12_[i / kL2BlockSize] |= (pos_l1[i / kL1BlockSize]
            << (31 - flag * 10));
      }
      flag++;
    }
    if (bitmap.GetBit(i)) {
      count++;
    }
    if (i % 64 == 0) {
      p = Utils::Popcount((bitmap.data_[i / 64] >> 48) & 65535);
      p = p % 16;
      size += dictionary_coder.offbits[p];
      dict.push_back(p);
      dict.push_back(
          dictionary_coder.encode_table[p][(bitmap.data_[i / 64] >> 48)
              & 65535]);

      p = Utils::Popcount((bitmap.data_[i / 64] >> 32) & 65535);
      p = p % 16;
      size += dictionary_coder.offbits[p];
      dict.push_back(p % 16);
      dict.push_back(
          dictionary_coder.encode_table[p][(bitmap.data_[i / 64] >> 32)
              & 65535]);

      p = Utils::Popcount((bitmap.data_[i / 64] >> 16) & 65535);
      p = p % 16;
      size += dictionary_coder.offbits[p];
      dict.push_back(p % 16);
      dict.push_back(
          dictionary_coder.encode_table[p][(bitmap.data_[i / 64] >> 16)
              & 65535]);

      p = Utils::Popcount(bitmap.data_[i / 64] & 65535);
      p = p % 16;
      size += dictionary_coder.offbits[p];
      dict.push_back(p % 16);
      dict.push_back(
          dictionary_coder.encode_table[p][bitmap.data_[i / 64] & 65535]);

      size += 16;
    }
  }

  bitmap_ = new Bitmap(size);
  uint64_t numBits = 0;
  for (size_t i = 0; i < dict.size(); i++) {
    if (i % 2 == 0) {
      bitmap_->SetValPos(numBits, dict[i], 4);
      numBits += 4;
    } else {
      bitmap_->SetValPos(numBits, dict[i],
                         dictionary_coder.offbits[dict[i - 1]]);
      numBits += dictionary_coder.offbits[dict[i - 1]];
    }
  }

  delete[] rank_l1;
  delete[] rank_l2;
  delete[] pos_l1;
  delete[] pos_l2;
}

Dictionary::~Dictionary() {
  delete bitmap_;
}

uint64_t Dictionary::Rank1(uint64_t i) {
  assert(i < size_);

  uint64_t l3_idx = i / kL3BlockSize;
  uint64_t l2_idx = i / kL2BlockSize;
  uint16_t l1_idx = i % kL1BlockSize;
  uint16_t rem = (i % kL2BlockSize) / kL1BlockSize;
  uint16_t block_class, block_offset;

  uint64_t res = rank_l3_[l3_idx] + GETRANKL2(rank_l12_[l2_idx]);
  uint64_t pos = pos_l3_[l3_idx] + GETPOSL2(pos_l12_[l2_idx]);

  switch (rem) {
    case 1:
      res += GETRANKL1(rank_l12_[l2_idx], 1);
      pos += GETPOSL1(pos_l12_[l2_idx], 1);
      break;

    case 2:
      res += GETRANKL1(rank_l12_[l2_idx], 1) + GETRANKL1(rank_l12_[l2_idx], 2);
      pos += GETPOSL1(pos_l12_[l2_idx], 1) + GETPOSL1(pos_l12_[l2_idx], 2);
      break;

    case 3:
      res += GETRANKL1(rank_l12_[l2_idx], 1) + GETRANKL1(rank_l12_[l2_idx], 2)
      + GETRANKL1(rank_l12_[l2_idx], 3);
      pos += GETPOSL1(pos_l12_[l2_idx], 1) + GETPOSL1(pos_l12_[l2_idx], 2)
      + GETPOSL1(pos_l12_[l2_idx], 3);
      break;

    default:
      break;
  }

  // Pop-count
  while (l1_idx >= 16) {
    block_class = bitmap_->GetValPos(pos, 4);
    pos += 4;
    block_offset = (block_class == 0) ? bitmap_->GetBit(pos) * 16 : 0;
    pos += dictionary_coder.offbits[block_class];
    res += block_class + block_offset;
    l1_idx -= 16;
  }

  block_class = bitmap_->GetValPos(pos, 4);
  pos += 4;
  block_offset = bitmap_->GetValPos(
      pos, dictionary_coder.offbits[block_class]);
  res += dictionary_coder.smallrank[dictionary_coder
      .decode_table[block_class][block_offset]][l1_idx];

  return res;
}

uint64_t Dictionary::Rank0(uint64_t i) {
  return i - Rank1(i) - 1;
}

uint64_t Dictionary::Select1(uint64_t i) {
  uint64_t val = i + 1;
  int64_t sp = 0;
  int64_t ep = size_ / kL3BlockSize;
  uint64_t m, r;
  uint64_t pos = 0;
  uint64_t block_class, block_offset;
  uint64_t sel = 0;
  uint16_t lastblock;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = rank_l3_[m];
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = std::max(ep, 0LL);
  sel += ep * kL3BlockSize;
  val -= rank_l3_[ep];
  pos += pos_l3_[ep];
  sp = ep * kL3BlockSize / kL2BlockSize;
  ep = std::min(((ep + 1) * kL3BlockSize / kL2BlockSize),
                Utils::NumBlocks(size_, kL2BlockSize)) - 1;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = GETRANKL2(rank_l12_[(sp + ep) / 2]);
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = std::max(ep, 0LL);
  sel += ep * kL2BlockSize;
  val -= GETRANKL2(rank_l12_[ep]);
  pos += GETPOSL2(pos_l12_[ep]);

  assert(val <= kL2BlockSize);

  r = GETRANKL1(rank_l12_[ep], 1);
  if (sel + kL1BlockSize < size_ && val > r) {
    pos += GETPOSL1(pos_l12_[ep], 1);
    val -= r;
    sel += kL1BlockSize;
    r = GETRANKL1(rank_l12_[ep], 2);
    if (sel + kL1BlockSize < size_ && val > r) {
      pos += GETPOSL1(pos_l12_[ep], 2);
      val -= r;
      sel += kL1BlockSize;
      r = GETRANKL1(rank_l12_[ep], 3);
      if (sel + kL1BlockSize < size_ && val > r) {
        pos += GETPOSL1(pos_l12_[ep], 3);
        val -= r;
        sel += kL1BlockSize;
      }
    }
  }

  assert(val <= kL1BlockSize);

  while (true) {
    block_class = bitmap_->GetValPos(pos, 4);
    unsigned short tempint = dictionary_coder.offbits[block_class];
    pos += 4;
    block_offset = (block_class == 0) ? bitmap_->GetBit(pos) * 16 : 0;
    pos += tempint;

    if (val <= (block_class + block_offset)) {
      pos -= (4 + tempint);
      break;
    }
    val -= (block_class + block_offset);
    sel += 16;
  }

  block_class = bitmap_->GetValPos(pos, 4);
  pos += 4;
  block_offset = bitmap_->GetValPos(
      pos, dictionary_coder.offbits[block_class]);
  lastblock = dictionary_coder.decode_table[block_class][block_offset];

  uint64_t count = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if ((lastblock >> (15 - i)) & 1) {
      count++;
    }
    if (count == val) {
      return sel + i;
    }
  }

  return sel;
}

uint64_t Dictionary::Select0(uint64_t i) {
  uint64_t val = i + 1;
  int64_t sp = 0;
  int64_t ep = size_ / kL3BlockSize;
  uint64_t m, r = 0;
  uint64_t pos = 0;
  uint64_t block_class, block_offset;
  uint64_t sel = 0;
  uint16_t lastblock;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = m * kL3BlockSize - rank_l3_[m];
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = std::max(ep, 0LL);
  sel += ep * kL3BlockSize;
  val -= (ep * kL3BlockSize - rank_l3_[ep]);
  pos += pos_l3_[ep];
  sp = ep * kL3BlockSize / kL2BlockSize;
  ep = std::min(((ep + 1) * kL3BlockSize / kL2BlockSize),
                Utils::NumBlocks(size_, kL2BlockSize)) - 1;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = m * kL2BlockSize - GETRANKL2(rank_l12_[m]);
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = std::max(ep, 0LL);
  sel += ep * kL2BlockSize;
  val -= (ep * kL2BlockSize - GETRANKL2(rank_l12_[ep]));
  pos += GETPOSL2(pos_l12_[ep]);

  assert(val <= kL2BlockSize);
  r = (kL1BlockSize - GETRANKL1(rank_l12_[ep], 1));
  if (sel + kL1BlockSize < size_ && val > r) {
    pos += GETPOSL1(pos_l12_[ep], 1);
    val -= r;
    sel += kL1BlockSize;
    r = (kL1BlockSize - GETRANKL1(rank_l12_[ep], 2));
    if (sel + kL1BlockSize < size_ && val > r) {
      pos += GETPOSL1(pos_l12_[ep], 2);
      val -= r;
      sel += kL1BlockSize;
      r = (kL1BlockSize - GETRANKL1(rank_l12_[ep], 3));
      if (sel + kL1BlockSize < size_ && val > r) {
        pos += GETPOSL1(pos_l12_[ep], 3);
        val -= r;
        sel += kL1BlockSize;
      }
    }
  }

  assert(val <= kL1BlockSize);

  while (true) {
    block_class = bitmap_->GetValPos(pos, 4);
    unsigned short tempint = dictionary_coder.offbits[block_class];
    pos += 4;
    block_offset = (block_class == 0) ? bitmap_->GetBit(pos) * 16 : 0;
    pos += tempint;

    if (val <= (16 - (block_class + block_offset))) {
      pos -= (4 + tempint);
      break;
    }

    val -= (16 - (block_class + block_offset));
    sel += 16;
  }

  block_class = bitmap_->GetValPos(pos, 4);
  pos += 4;
  block_offset = bitmap_->GetValPos(
      pos, dictionary_coder.offbits[block_class]);
  lastblock = dictionary_coder.decode_table[block_class][block_offset];

  uint64_t count = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if (!((lastblock >> (15 - i)) & 1)) {
      count++;
    }
    if (count == val) {
      return sel + i;
    }
  }

  return sel;
}

size_t Dictionary::Serialize(std::ostream& out) {
  size_t out_size = 0;

  out.write(reinterpret_cast<const char *>(&size_), sizeof(uint64_t));
  out_size += sizeof(uint64_t);

  for (uint64_t i = 0; i < (size_ / kL3BlockSize) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&rank_l3_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL2BlockSize) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&rank_l12_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL3BlockSize) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&pos_l3_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL2BlockSize) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&pos_l12_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  out_size += bitmap_->Serialize(out);

  return out_size;
}

size_t Dictionary::Deserialize(std::istream& in) {
  size_t in_size = 0;
  uint64_t dictionary_size;

  in.read(reinterpret_cast<char *>(&size_), sizeof(uint64_t));
  in_size += sizeof(uint64_t);

  rank_l3_ = new uint64_t[(size_ / kL3BlockSize) + 1];
  rank_l12_ = new uint64_t[(size_ / kL2BlockSize) + 1];
  pos_l3_ = new uint64_t[(size_ / kL3BlockSize) + 1];
  pos_l12_ = new uint64_t[(size_ / kL2BlockSize) + 1];

  for (uint64_t i = 0; i < (size_ / kL3BlockSize) + 1; i++) {
    in.read(reinterpret_cast<char *>(&rank_l3_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL2BlockSize) + 1; i++) {
    in.read(reinterpret_cast<char *>(&rank_l12_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL3BlockSize) + 1; i++) {
    in.read(reinterpret_cast<char *>(&pos_l3_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / kL2BlockSize) + 1; i++) {
    in.read(reinterpret_cast<char *>(&pos_l12_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  in_size += bitmap_->Deserialize(in);

  return in_size;
}

}

#endif // BITMAP_DICTIONARY_H_
