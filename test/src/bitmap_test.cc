#include "bitmap.h"
#include "utils.h"

#include "gtest/gtest.h"

class BitmapTest : public testing::Test {
 public:
  const uint64_t kBitmapSize = (1024ULL * 1024ULL);  // 1 KBits

 protected:
  virtual void SetUp() override {
    bitmap = new bitmap::Bitmap(kBitmapSize);
  }

  virtual void TearDown() override {
    delete bitmap;
  }

  bitmap::Bitmap *bitmap;
};

TEST_F(BitmapTest, GetSetBitTest) {
  for (uint64_t i = 0; i < kBitmapSize; i++) {
    if (i % 2 == 0) {
      bitmap->SetBit(i);
    }
  }

  for (uint64_t i = 0; i < kBitmapSize; i++) {
    bool val = bitmap->GetBit(i);
    ASSERT_EQ(val, i % 2 == 0);
  }
}

TEST_F(BitmapTest, GetSetValPosTest) {
  uint64_t pos = 0;
  for (uint64_t i = 0; i < 10000; i++) {
    bitmap->SetValPos(pos, i, bitmap::Utils::BitWidth(i));
    pos += bitmap::Utils::BitWidth(i);
  }

  pos = 0;
  for (uint64_t i = 0; i < 10000; i++) {
    uint64_t val = bitmap->GetValPos(pos, bitmap::Utils::BitWidth(i));
    ASSERT_EQ(val, i);
    pos += bitmap::Utils::BitWidth(i);
  }
}
