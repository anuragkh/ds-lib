#ifndef COUNTED_BTREE_H_
#define COUNTED_BTREE_H_

template<typename T, int N_INTERNAL = 4, int N_LEAF = 7>
class CountedBTree {
 public:
  typedef T key_type;
  typedef int32_t ptr_type;
  typedef uint32_t cnt_type;

  class CountedNode {
   public:
    cnt_type cnt;
  };

  class CountedInternalNode : public CountedNode {
   public:
    key_type key[N_INTERNAL];
    ptr_type ptr[N_INTERNAL + 1];
  };

  class CountedLeafNode : public CountedNode {
   public:
    key_type key[N_LEAF];
  };

 private:
  CountedNode root_;

};

#endif /* COUNTED_BTREE_H_ */
