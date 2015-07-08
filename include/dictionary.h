#ifndef DSL_DICTIONARY_H_
#define DSL_DICTIONARY_H_

#include <map>

#include "bitmap.h"

namespace dsl {

/* Get rank operations */
#define GETRANKL2(n)    (n >> 32)
#define GETRANKL1(n, i) (((n & 0xffffffff) >> (32 - i * 10)) & 0x3ff)

#define GETPOSL2(n)     (n >> 31)
#define GETPOSL1(n, i)  (((n & 0x7fffffff) >> (31 - i * 10)) & 0x3ff)

/* Rank Data structure constants */
#define L1BLKSIZE   512L
#define L2BLKSIZE   2048L
#define L3BLKSIZE   4294967296L

/* Constant tables used for encoding/decoding
     * dictionary and delta encoded vector */
extern uint16_t *decode_table[17];
extern std::map<uint16_t, uint16_t> encode_table[17];
extern uint16_t C16[17];
extern uint8_t offbits[17];
extern uint8_t smallrank[65536][16];

class Dictionary {
 public:
  Dictionary();
  Dictionary(Bitmap &B);

  uint64_t rank0(uint64_t i);
  uint64_t rank1(uint64_t i);

  uint64_t select0(uint64_t i);
  uint64_t select1(uint64_t i);

  size_t serialize(std::ostream& out);
  size_t deserialize(std::istream& in);

 private:
  Bitmap bimap_;
  uint64_t size_;
  uint64_t *rank_l12_;
  uint64_t *rank_l3_;
  uint64_t *pos_l12_;
  uint64_t *pos_l3_;
};

}

#endif // DSL_DICTIONARY_H_
