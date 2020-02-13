#include "bitmap.h"

#include <cstdio>
#include <vector>
#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

#define BITMAP_SIZE (100*1024*1024)

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
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Bitmap = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += bitmap.GetBit(i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to read Bitmap = %llu; sum=%lld\n",
            (t1 - t0), sum);
  }
  {
    std::vector<bool> bitmap(BITMAP_SIZE, 0);
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      if (i % 2 == 0) {
        bitmap[i] = 1;
      }
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill vector<bool> = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
      sum += bitmap[i];
    }
    t1 = GetTimestamp();
    fprintf(stderr, "Time to read vector<bool> = %llu; sum=%lld\n", (t1 - t0), sum);
  }
}
