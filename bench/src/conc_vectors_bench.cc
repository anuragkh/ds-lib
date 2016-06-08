#include "conc_vectors.h"

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
    ConcurrentVector<uint32_t> array;

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.push_back(i);
    }
    t1 = GetTimestamp();
    double avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(stderr, "Time to fill Concurrent vector = %llu, avg = %lf\n",
            (t1 - t0), avg);

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array.at(i);
    }
    t1 = GetTimestamp();
    avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(stderr,
            "Time to read Concurrent vector = %llu, avg = %lf, sum=%lld\n",
            (t1 - t0), avg, sum);
  }
  {
    LockFreeGrowingList<uint32_t> array;

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.push_back(i);
    }
    t1 = GetTimestamp();
    double avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(stderr,
            "Time to fill Lock-free growing list (default) = %llu, avg = %lf\n",
            (t1 - t0), avg);

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array.at(i);
    }
    t1 = GetTimestamp();
    avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(
        stderr,
        "Time to read Lock-free growing list (default) = %llu, avg = %lf, sum=%lld\n",
        (t1 - t0), avg, sum);
  }

  {
    LockFreeGrowingList<uint32_t, 16> array;

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.push_back(i);
    }
    t1 = GetTimestamp();
    double avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(stderr,
            "Time to fill Lock-free growing list (64) = %llu, avg = %lf\n",
            (t1 - t0), avg);

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array.at(i);
    }
    t1 = GetTimestamp();
    avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(
        stderr,
        "Time to read Lock-free growing list (64) = %llu, avg = %lf, sum=%lld\n",
        (t1 - t0), avg, sum);
  }

  {
    std::vector<uint32_t> array;

    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      array.push_back(i);
    }
    t1 = GetTimestamp();
    double avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(stderr,
            "Time to fill std::vector = %llu, avg = %lf\n",
            (t1 - t0), avg);

    int64_t sum = 0;
    t0 = GetTimestamp();
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
      sum += array.at(i);
    }
    t1 = GetTimestamp();
    avg = (double) (t1 - t0) / ARRAY_SIZE;
    fprintf(
        stderr,
        "Time to read std::vector = %llu, avg = %lf, sum=%lld\n",
        (t1 - t0), avg, sum);
  }
}
