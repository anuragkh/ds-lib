#include "delta_encoded_array.h"
#include "utils.h"

#include <cstdio>
#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

#define ARRAY_SIZE (10*1024*1024)

int main(int argc, char** argv) {
  if (argc > 1) {
    fprintf(stderr, "%s does not take any arguments.\n", argv[0]);
  }

  TimeStamp t0, t1;

  {
    uint64_t *array = new uint64_t[ARRAY_SIZE];

    t0 = GetTimestamp();
    for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
      array[i] = i;
    }
    bitmap::EliasGammaEncodedArray<uint64_t> enc_array(array, ARRAY_SIZE);
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Delta Encoded Array = %llu\n", (t1 - t0));

    uint64_t sum = 0;
    t0 = GetTimestamp();
    for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
      sum += enc_array[i];
    }
    t1 = GetTimestamp();
    fprintf(stderr, "Time to read Delta Encoded Array = %llu; sum=%lld\n", (t1 - t0), sum);
  }
  {
    uint64_t *array = new uint64_t[ARRAY_SIZE];

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array[i] = i;
    }
    t1 = GetTimestamp();

    fprintf(stderr, "Time to fill Array = %llu\n", (t1 - t0));

    uint64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array[i];
    }
    t1 = GetTimestamp();
    fprintf(stderr, "Time to read Array = %llu; sum=%lld\n", (t1 - t0), sum);
  }
}
