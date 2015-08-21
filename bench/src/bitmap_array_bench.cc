#include "bitmap_array.h"

#include <cstdio>
#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

#define ARRAY_SIZE (100*1024*1024)

int main(int argc, char** argv) {
  if (argc > 1) {
    fprintf(stderr, "%s does not take any arguments.\n", argv[0]);
  }

  TimeStamp t0, t1;

  {
    bitmap::UnsignedBitmapArray<uint64_t> array(ARRAY_SIZE, 30);

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.Set(i, i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Unsigned Bitmap Array = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      // assert(array[i] == i);
      sum += array.Get(i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to read Unsigned Bitmap Array = %llu; sum=%lld\n",
            (t1 - t0), sum);
  }
  {
    bitmap::SignedBitmapArray<int64_t> array(ARRAY_SIZE, 30);

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.Set(i, i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Signed Bitmap Array = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      // assert(array[i] == i);
      sum += array.Get(i);
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to read Signed Bitmap Array = %llu; sum=%lld\n",
            (t1 - t0), sum);
  }

  {
    int64_t *array = new int64_t[ARRAY_SIZE];
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array[i] = i;
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Array = %llu\n", (t1 - t0));

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array[i];
    }
    t1 = GetTimestamp();
    fprintf(stderr, "Time to read Array = %llu; sum=%lld\n", (t1 - t0), sum);
  }
}
