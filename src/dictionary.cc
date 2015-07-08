#include "dictionary.h"

#include <cassert>
#include <math.h>
#include <vector>

#include "utils.h"

dsl::Dictionary::Dictionary() {
  size_ = 0;
  rank_l12_ = NULL;
  rank_l3_ = NULL;
  pos_l12_ = NULL;
  pos_l3_ = NULL;
}

dsl::Dictionary::Dictionary(Bitmap &B) {
  uint64_t l3_size = (B.size_ / L3BLKSIZE) + 1;
  uint64_t l2_size = (B.size_ / L2BLKSIZE) + 1;
  uint64_t l1_size = (B.size_ / L1BLKSIZE) + 1;
  uint64_t count = 0;

  size_ = B.size_;
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

  for (i = 0; i < B.size_; i++) {
    if (i % L3BLKSIZE == 0) {
      rank_l3_[i / L3BLKSIZE] = count;
      pos_l3_[i / L3BLKSIZE] = size;
    }
    if (i % L2BLKSIZE == 0) {
      rank_l2[i / L2BLKSIZE] = count - rank_l3_[i / L3BLKSIZE];
      pos_l2[i / L2BLKSIZE] = size - pos_l3_[i / L3BLKSIZE];
      rank_l12_[i / L2BLKSIZE] = rank_l2[i / L2BLKSIZE] << 32;
      pos_l12_[i / L2BLKSIZE] = pos_l2[i / L2BLKSIZE] << 31;
      flag = 0;
      sum_l1 = 0;
      sum_pos_l1 = 0;
    }
    if (i % L1BLKSIZE == 0) {
      rank_l1[i / L1BLKSIZE] = count - rank_l2[i / L2BLKSIZE] - sum_l1;
      pos_l1[i / L1BLKSIZE] = size - pos_l2[i / L2BLKSIZE] - sum_pos_l1;
      sum_l1 += rank_l1[i / L1BLKSIZE];
      sum_pos_l1 += pos_l1[i / L1BLKSIZE];
      if (flag != 0) {
        rank_l12_[i / L2BLKSIZE] |=
            (rank_l1[i / L1BLKSIZE] << (32 - flag * 10));
        pos_l12_[i / L2BLKSIZE] |= (pos_l1[i / L1BLKSIZE] << (31 - flag * 10));
      }
      flag++;
    }
    if (B.getBit(i)) {
      count++;
    }
    if (i % 64 == 0) {
      p = dsl::Utils::popcount((B.data_[i / 64] >> 48) & 65535);
      p = p % 16;
      size += offbits[p];
      dict.push_back(p);
      dict.push_back(encode_table[p][(B.data_[i / 64] >> 48) & 65535]);

      p = dsl::Utils::popcount((B.data_[i / 64] >> 32) & 65535);
      p = p % 16;
      size += offbits[p];
      dict.push_back(p % 16);
      dict.push_back(encode_table[p][(B.data_[i / 64] >> 32) & 65535]);

      p = dsl::Utils::popcount((B.data_[i / 64] >> 16) & 65535);
      p = p % 16;
      size += offbits[p];
      dict.push_back(p % 16);
      dict.push_back(encode_table[p][(B.data_[i / 64] >> 16) & 65535]);

      p = dsl::Utils::popcount(B.data_[i / 64] & 65535);
      p = p % 16;
      size += offbits[p];
      dict.push_back(p % 16);
      dict.push_back(encode_table[p][B.data_[i / 64] & 65535]);

      size += 16;
    }
  }

  bimap_ = Bitmap(size);
  uint64_t numBits = 0;
  for (size_t i = 0; i < dict.size(); i++) {
    if (i % 2 == 0) {
      bimap_.putValPos(numBits, dict[i], 4);
      numBits += 4;
    } else {
      bimap_.putValPos(numBits, dict[i], offbits[dict[i - 1]]);
      numBits += offbits[dict[i - 1]];
    }
  }

  delete[] rank_l1;
  delete[] rank_l2;
  delete[] pos_l1;
  delete[] pos_l2;
}

uint64_t dsl::Dictionary::rank1(uint64_t i) {
  assert(i < size_);

  uint64_t l3_idx = i / L3BLKSIZE;
  uint64_t l2_idx = i / L2BLKSIZE;
  uint16_t l1_idx = i % L1BLKSIZE;
  uint16_t rem = (i % L2BLKSIZE) / L1BLKSIZE;
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
    block_class = bimap_.getValPos(pos, 4);
    pos += 4;
    block_offset = (block_class == 0) ? bimap_.getBit(pos) * 16 : 0;
    pos += offbits[block_class];
    res += block_class + block_offset;
    l1_idx -= 16;
  }

  block_class = bimap_.getValPos(pos, 4);
  pos += 4;
  block_offset = bimap_.getValPos(pos, offbits[block_class]);
  res += smallrank[decode_table[block_class][block_offset]][l1_idx];

  return res;
}

uint64_t dsl::Dictionary::rank0(uint64_t i) {
  return i - rank1(i) - 1;
}

uint64_t dsl::Dictionary::select1(uint64_t i) {
  uint64_t val = i + 1;
  int64_t sp = 0;
  int64_t ep = size_ / L3BLKSIZE;
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

  ep = MAX(ep, 0);
  sel += ep * L3BLKSIZE;
  val -= rank_l3_[ep];
  pos += pos_l3_[ep];
  sp = ep * L3BLKSIZE / L2BLKSIZE;
  ep = MIN(((ep + 1) * L3BLKSIZE / L2BLKSIZE), ceil((double)size_ / L2BLKSIZE))
      - 1;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = GETRANKL2(rank_l12_[(sp + ep) / 2]);
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = MAX(ep, 0);
  sel += ep * L2BLKSIZE;
  val -= GETRANKL2(rank_l12_[ep]);
  pos += GETPOSL2(pos_l12_[ep]);

  assert(val <= L2BLKSIZE);

  r = GETRANKL1(rank_l12_[ep], 1);
  if (sel + L1BLKSIZE < size_ && val > r) {
    pos += GETPOSL1(pos_l12_[ep], 1);
    val -= r;
    sel += L1BLKSIZE;
    r = GETRANKL1(rank_l12_[ep], 2);
    if (sel + L1BLKSIZE < size_ && val > r) {
      pos += GETPOSL1(pos_l12_[ep], 2);
      val -= r;
      sel += L1BLKSIZE;
      r = GETRANKL1(rank_l12_[ep], 3);
      if (sel + L1BLKSIZE < size_ && val > r) {
        pos += GETPOSL1(pos_l12_[ep], 3);
        val -= r;
        sel += L1BLKSIZE;
      }
    }
  }

  assert(val <= L1BLKSIZE);

  while (true) {
    block_class = bimap_.getValPos(pos, 4);
    unsigned short tempint = offbits[block_class];
    pos += 4;
    block_offset = (block_class == 0) ? bimap_.getBit(pos) * 16 : 0;
    pos += tempint;

    if (val <= (block_class + block_offset)) {
      pos -= (4 + tempint);
      break;
    }
    val -= (block_class + block_offset);
    sel += 16;
  }

  block_class = bimap_.getValPos(pos, 4);
  pos += 4;
  block_offset = bimap_.getValPos(pos, offbits[block_class]);
  lastblock = decode_table[block_class][block_offset];

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

uint64_t dsl::Dictionary::select0(uint64_t i) {
  uint64_t val = i + 1;
  long sp = 0;
  long ep = size_ / L3BLKSIZE;
  uint64_t m, r = 0;
  uint64_t pos = 0;
  uint64_t block_class, block_offset;
  uint64_t sel = 0;
  uint16_t lastblock;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = m * L3BLKSIZE - rank_l3_[m];
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = MAX(ep, 0);
  sel += ep * L3BLKSIZE;
  val -= (ep * L3BLKSIZE - rank_l3_[ep]);
  pos += pos_l3_[ep];
  sp = ep * L3BLKSIZE / L2BLKSIZE;
  ep = MIN(((ep + 1) * L3BLKSIZE / L2BLKSIZE), ceil((double)size_ / L2BLKSIZE))
      - 1;

  while (sp <= ep) {
    m = (sp + ep) / 2;
    r = m * L2BLKSIZE - GETRANKL2(rank_l12_[m]);
    if (val > r)
      sp = m + 1;
    else
      ep = m - 1;
  }

  ep = MAX(ep, 0);
  sel += ep * L2BLKSIZE;
  val -= (ep * L2BLKSIZE - GETRANKL2(rank_l12_[ep]));
  pos += GETPOSL2(pos_l12_[ep]);

  assert(val <= L2BLKSIZE);
  r = (L1BLKSIZE - GETRANKL1(rank_l12_[ep], 1));
  if (sel + L1BLKSIZE < size_ && val > r) {
    pos += GETPOSL1(pos_l12_[ep], 1);
    val -= r;
    sel += L1BLKSIZE;
    r = (L1BLKSIZE - GETRANKL1(rank_l12_[ep], 2));
    if (sel + L1BLKSIZE < size_ && val > r) {
      pos += GETPOSL1(pos_l12_[ep], 2);
      val -= r;
      sel += L1BLKSIZE;
      r = (L1BLKSIZE - GETRANKL1(rank_l12_[ep], 3));
      if (sel + L1BLKSIZE < size_ && val > r) {
        pos += GETPOSL1(pos_l12_[ep], 3);
        val -= r;
        sel += L1BLKSIZE;
      }
    }
  }

  assert(val <= L1BLKSIZE);

  while (true) {
    block_class = bimap_.getValPos(pos, 4);
    unsigned short tempint = offbits[block_class];
    pos += 4;
    block_offset = (block_class == 0) ? bimap_.getBit(pos) * 16 : 0;
    pos += tempint;

    if (val <= (16 - (block_class + block_offset))) {
      pos -= (4 + tempint);
      break;
    }

    val -= (16 - (block_class + block_offset));
    sel += 16;
  }

  block_class = bimap_.getValPos(pos, 4);
  pos += 4;
  block_offset = bimap_.getValPos(pos, offbits[block_class]);
  lastblock = decode_table[block_class][block_offset];

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

size_t dsl::Dictionary::serialize(std::ostream& out) {
  size_t out_size = 0;

  out.write(reinterpret_cast<const char *>(&size_), sizeof(uint64_t));
  out_size += sizeof(uint64_t);

  for (uint64_t i = 0; i < (size_ / L3BLKSIZE) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&rank_l3_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L2BLKSIZE) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&rank_l12_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L3BLKSIZE) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&pos_l3_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L2BLKSIZE) + 1; i++) {
    out.write(reinterpret_cast<const char *>(&pos_l12_[i]), sizeof(uint64_t));
    out_size += sizeof(uint64_t);
  }

  out_size += bimap_.serialize(out);

  return out_size;
}

size_t dsl::Dictionary::deserialize(std::istream& in) {
  size_t in_size = 0;
  uint64_t dictionary_size;

  in.read(reinterpret_cast<char *>(&size_), sizeof(uint64_t));
  in_size += sizeof(uint64_t);

  rank_l3_ = new uint64_t[(size_ / L3BLKSIZE) + 1];
  rank_l12_ = new uint64_t[(size_ / L2BLKSIZE) + 1];
  pos_l3_ = new uint64_t[(size_ / L3BLKSIZE) + 1];
  pos_l12_ = new uint64_t[(size_ / L2BLKSIZE) + 1];

  for (uint64_t i = 0; i < (size_ / L3BLKSIZE) + 1; i++) {
    in.read(reinterpret_cast<char *>(&rank_l3_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L2BLKSIZE) + 1; i++) {
    in.read(reinterpret_cast<char *>(&rank_l12_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L3BLKSIZE) + 1; i++) {
    in.read(reinterpret_cast<char *>(&pos_l3_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  for (uint64_t i = 0; i < (size_ / L2BLKSIZE) + 1; i++) {
    in.read(reinterpret_cast<char *>(&pos_l12_[i]), sizeof(uint64_t));
    in_size += sizeof(uint64_t);
  }

  in_size += bimap_.deserialize(in);

  return in_size;
}
