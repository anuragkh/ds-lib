#include "bitmap_array.h"
#include "utils.h"

#include "gtest/gtest.h"

class BitmapArrayTest : public testing::Test {
 public:
  const uint64_t kArraySize = (1024ULL * 1024ULL);  // 1 KBytes
  const uint8_t kBitWidth = 20;  // 20 bits
};

TEST_F(BitmapArrayTest, UnsizedBitmapArrayTest) {
  bitmap::UnsizedBitmapArray<uint64_t> array(kArraySize, kBitWidth);
  for (uint64_t i = 0; i < kArraySize; i++) {
    array[i] = i;
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(array[i], i);
  }
}

TEST_F(BitmapArrayTest, UnsignedBitmapArrayTest) {
  bitmap::UnsignedBitmapArray<uint64_t> array(kArraySize, kBitWidth);
  for (uint64_t i = 0; i < kArraySize; i++) {
    array[i] = i;
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_EQ(array[i], i);
  }
}

TEST_F(BitmapArrayTest, SignedBitmapArrayTest) {
  bitmap::SignedBitmapArray<int64_t> array(kArraySize, kBitWidth);
  for (int64_t i = 0; i < kArraySize; i++) {
    if (i % 2 == 0) {
      array[i] = i;
    } else {
      array[i] = -i;
    }
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (i % 2 == 0) {
      ASSERT_EQ(array[i], i);
    } else {
      ASSERT_EQ(array[i], -i);
    }
  }
}
