#ifndef SL_HPP
#define SL_HPP
#include "stdlib.h"

#include <urcu-defer.h>
#include <atomic>
#include <random>
#include <utility>
#include <iostream>  // only for debug
#include "markable_ptr.hpp"

namespace nanahan {

namespace detail {
template <typename T>
void nothing_deleter(const T*) {
} // do nothing
}  // namespace detail

template <typename KeyType, typename ValueType>
class LachesisDB {
private:
  class node_t {
   private:
    friend class LachesisDB<KeyType, ValueType>;

    static node_t* allocate(int size) {
      const size_t calc_size =
          sizeof(KeyType) +
          sizeof(ValueType) +
          sizeof(int) +
          sizeof(markable_ptr<node_t>) * size;
      const size_t aligned_size = ((calc_size + 7) / 8) * 8;
      std::cout << "aligned_size:" << aligned_size << std::endl;
      node_t* ret = static_cast<node_t*>(malloc(aligned_size));
      ret->top_layer = size;
      return ret;
    }

    static node_t* new_node(KeyType key, ValueType value, int height) {
      node_t* ret = allocate(height);
      new (&ret->first) KeyType(key);
      new (&ret->second) ValueType(value);
      return ret;
    }

   public:
    const KeyType first;
    ValueType second;
   private:
    int top_layer;  // length
    markable_ptr<node_t> next[1];  // variable length
  };

 public:
  class iterator {
   public:
    node_t* point;
   private:
    iterator(node_t* p)
      : point(p) {}
   public:
    ~iterator() {}

    node_t& operator*() { return *point; }
    node_t* operator->() { return &operator*(); }
    const node_t& operator*() const { return *point; }
    const node_t* operator->() const { return &operator*();}
    bool operator==(const iterator& rhs) const {
      return point == rhs.point;
    }
    bool operator!=(const iterator& rhs) const {
      return !this->operator==(rhs);
    }

    iterator& operator++() {
      for (;;) {
        if (!point->next[0]->is_null()) {
          point = point->next[0];
        }
      }
      return *this;
    }
  };

 private:
  const int max_height_;
  mutable std::mt19937 rand_;
  node_t* head_;
 public:

  LachesisDB(int max_height, int randomseed = std::random_device()())
    : max_height_(max_height),
      rand_(static_cast<unsigned long>(randomseed)) {
    head_ = node_t::allocate(max_height);
    std::cout << "max_height:" <<max_height << std::endl;
    for (int i = 0; i < max_height; ++i) {
      head_->next[i] = nullptr;
    }
  }

  ~LachesisDB() {
    this->clear();
  }

  bool contains(const KeyType& k) {
  }

  iterator get(const KeyType& k) {
  }

  iterator lower_bound(const KeyType& k) {
  }

  bool add(const KeyType& k, const ValueType& v) {
  }

  iterator& begin() { return iterator(head_); }
  const iterator& begin() const { return iterator(nullptr);}
  iterator& end() { return nullptr; }
  const iterator& end() const { return nullptr; }

  bool remove(const KeyType& k) {
  }

  bool is_empty() const {
    return head_ == nullptr;
  }

  void clear() {
    node_t* next = head_;
    while (next == nullptr) {
      node_t* old_next = next;
      next = next->next[0].get_ptr();
      free(old_next);
    }
  }
public:
  uint32_t random_level() const {
    const uint32_t gen = rand_();
    int bit = 1;
    int cnt = 0;
    while (cnt < max_height_ - 1) {
      if (gen & bit) {
        return cnt;
      }
      bit <<= 1;
      ++cnt;
    }
    return cnt;
  }
private:
  LachesisDB();
  LachesisDB(const LachesisDB&);
  LachesisDB& operator=(const LachesisDB&);
};

}  // namespace nanahan
#endif
