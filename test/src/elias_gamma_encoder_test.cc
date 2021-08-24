#include "elias_gamma_encoder.h"

#include "gtest/gtest.h"

class EliasGammaEncoderTest : public testing::Test {
 public:
  const uint64_t kArraySize = (1024ULL * 1024ULL);  // 1 KBytes
};

TEST_F(EliasGammaEncoderTest, EliasGammaEncodedArrayTest) {

  std::vector<uint64_t> input;
  for (uint64_t i = 0; i < kArraySize; i++) {
    input.push_back(i + 1);
  }

  auto encoded = bits::EliasGammaEncoder<uint64_t>::EncodeArray(input);
  auto decoded = bits::EliasGammaEncoder<uint64_t>::DecodeArray(encoded);

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(decoded[i], i + 1);
  }
}
