#include "delta_encoded_array.h"

#include "gtest/gtest.h"

class DeltaEncodedVectorTest : public testing::Test {
 public:
  const uint64_t kArraySize = (1024ULL * 1024ULL);  // 1 KBytes
};

TEST_F(DeltaEncodedVectorTest, EliasGammaEncodedVectorTest) {
  auto *array = new uint64_t[kArraySize];
  for (uint64_t i = 0; i < kArraySize; i++) {
    array[i] = i * 2;
  }

  bits::EliasGammaDeltaEncodedVector<uint64_t> enc_array(array, kArraySize);

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(enc_array[i], i * 2);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_TRUE(enc_array.Find(i * 2));
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_FALSE(enc_array.Find(i * 2 + 1));
  }
}
