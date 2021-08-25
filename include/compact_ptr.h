#ifndef COMPACT_PTR_H
#define COMPACT_PTR_H

#include <cstdint>
#include <cstddef>

namespace bits {

template<typename T>
class compact_ptr {
 public:
  compact_ptr(T *ptr, size_t size) : ptr_(ptr >> 4ULL), size_(size) {}

  explicit operator T *() {
    return (T *) (ptr_ << 4ULL);
  }

  T *operator->() {
    return (T *) (ptr_ << 4ULL);
  }

  size_t size() const {
    return size_;
  }

 private:
  uintptr_t ptr_: 44;
  size_t size_ : 20;
};

}

#endif // COMPACT_PTR_H
