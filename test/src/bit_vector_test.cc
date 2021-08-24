#include "bit_vector.h"
#include "utils.h"

#include "gtest/gtest.h"

class BitVectorTest : public testing::Test {
 public:
  const uint64_t kBitmapSize = (1024ULL * 1024ULL);  // 1 KBits

 protected:
  void SetUp() override {
    bitvec = new bits::BitVector(kBitmapSize);
  }

  void TearDown() override {
    delete bitvec;
  }

  bits::BitVector *bitvec{};
};

TEST_F(BitVectorTest, GetSetBitTest) {
  for (uint64_t i = 0; i < kBitmapSize; i++) {
    if (i % 2 == 0) {
      bitvec->SetBit(i);
    }
  }

  for (uint64_t i = 0; i < kBitmapSize; i++) {
    bool val = bitvec->GetBit(i);
    ASSERT_EQ(val, i % 2 == 0);
  }
}

TEST_F(BitVectorTest, GetSetValPosTest) {
  uint64_t pos = 0;
  for (uint64_t i = 0; i < 10000; i++) {
    bitvec->SetValPos(pos, i, bits::Utils::BitWidth(i));
    pos += bits::Utils::BitWidth(i);
  }

  pos = 0;
  for (uint64_t i = 0; i < 10000; i++) {
    uint64_t val = bitvec->GetValPos(pos, bits::Utils::BitWidth(i));
    ASSERT_EQ(val, i);
    pos += bits::Utils::BitWidth(i);
  }
}

TEST_F(BitVectorTest, AppendGetValTest) {
  uint64_t pos = 0;
  bits::BitVector v;
  for (uint64_t i = 0; i < 10000; i++) {
    v.AppendVal(i, bits::Utils::BitWidth(i));
    pos += bits::Utils::BitWidth(i);
  }

  pos = 0;
  for (uint64_t i = 0; i < 10000; i++) {
    uint64_t val = v.GetValPos(pos, bits::Utils::BitWidth(i));
    ASSERT_EQ(val, i);
    pos += bits::Utils::BitWidth(i);
  }
}
