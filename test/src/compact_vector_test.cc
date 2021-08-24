#include "compact_vector.h"

#include "gtest/gtest.h"

class CompactVectorTest : public testing::Test {
 public:
  const uint64_t kArraySize = (1024ULL * 1024ULL);  // 1 KBytes
  const uint8_t kBitWidth = 20;  // 20 bits
};

TEST_F(CompactVectorTest, CompactVectorTest1) {
  bits::CompactVector<uint64_t, 20> v(kArraySize);
  for (uint64_t i = 0; i < kArraySize; i++) {
    v[i] = i;
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(v[i], i);
  }
}

TEST_F(CompactVectorTest, CompactVectorTest2) {
  bits::CompactVector<uint64_t, 20> v;
  for (uint64_t i = 0; i < kArraySize; i++) {
    v.Append(i);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(v[i], i);
  }
}