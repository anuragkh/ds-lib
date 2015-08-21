#include "dictionary.h"

#include <cstdio>
#include <vector>
#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

inline static uint64_t rank1(bitmap::Bitmap& b, uint64_t i) {
  uint64_t sum = 0;
  for (uint64_t pos = 0; pos <= i; pos++) {
    sum += b.GetBit(pos);
  }
  return sum;
}

inline static uint64_t rank0(bitmap::Bitmap& b, uint64_t i) {
  return i - rank1(b, i) - 1;
}

inline static uint64_t select1(bitmap::Bitmap& b, uint64_t pos) {
  uint64_t sum = 0;
  uint64_t i = 0;
  while (sum < pos) {
    sum += b.GetBit(i++);
  }
  return sum;
}

inline static uint64_t select0(bitmap::Bitmap& b, uint64_t pos) {
  uint64_t sum = 0;
  uint64_t i = 0;
  while ((i - sum) < pos) {
    sum += b.GetBit(i++);
  }
  return sum;
}

#define BITMAP_SIZE (1000)

int main(int argc, char** argv) {
  if (argc > 1) {
    fprintf(stderr, "%s does not take any arguments.\n", argv[0]);
  }

  TimeStamp t0, t1;

  {

    bitmap::Bitmap bitmap(BITMAP_SIZE);

    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      if (i % 2) {
        bitmap.SetBit(i);
      }
    }
    bitmap::Dictionary dictionary(bitmap);
    t1 = GetTimestamp();

    fprintf(stderr, "Time to create Dictionary = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += dictionary.Rank1(i);
      fprintf(stderr, "Rank %zu = %llu\n", i, dictionary.Rank1(i));
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to compute 1-ranks = %llu; sum=%lld\n", (t1 - t0),
            sum);

    sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += dictionary.Rank0(0);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to compute 0-ranks = %llu; sum=%lld\n", (t1 - t0),
            sum);
  }
  {
    bitmap::Bitmap bitmap(BITMAP_SIZE);

    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      if (i % 2) {
        bitmap.SetBit(i);
      }
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Bitmap = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += rank1(bitmap, i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to compute 1-ranks = %llu; sum=%lld\n", (t1 - t0),
            sum);

    sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += rank0(bitmap, i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to compute 0-ranks = %llu; sum=%lld\n", (t1 - t0),
            sum);
  }
}
