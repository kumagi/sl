#ifndef SL_HPP
#define SL_HPP
#include <urcu-defer.h>
#include <atomic>
#include <random>
#include <utility>
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
  class node {
    node(KeyType key, ValueType value)
      : first(key), second(value) {
    }
    friend class LachesisDB<KeyType, ValueType>;

  public:
    const KeyType first;
    ValueType second;
  private:
    const int top_layer;  // length
    markable_ptr<node> next[1];  // variable length
  };

public:
  class iterator {
  public:
    node* point;
  private:
    iterator(node* p)
      : point(p) {}
  public:
    ~node() {}

    node_t& operator*() { return *node; }
    node_t* operator->() { return &operator*(); }
    const node_t& operator*() const { return *node; }
    const node_t* operator->() const { return &operator*();}
    bool operator==(const iterator& rhs) const = default;
    bool operator!=(const iterator& rhs) const = default;
    iterator& operator++() {
      while (true) {
        if (!node->next[0]->is_null()) {
          node = node->next[0];
        }
      }
      return *this;
    }
  }; __attribute__((aligned (64)));

private:
  const int max_height_;
  mutable boost::mt19937 rand_;
  markable_ptr<node> head_[1];  // variable length
public:
  LachesisDB(int max_height, int randomseed = std::random_device()())
    : max_height_(max_height),
      rand(static_cast<unsigned long>(randomseed)) {
    for (markable_ptr<node> h : head_) {
      h = nullptr;
    }
  }

  ~sl() {
  }
  bool contains(const key& k) {
    nodelists lists;
    const int lv = find(k, &lists);
    weak_node_array& succs = lists.second;
    if (lv == -1) return false;
    shared_node succ = succs[lv].lock();
    if (!succ) return false;
    return succ->fullylinked
      && !succ->marked;
  }

  iterator get(const key& k) {
    nodelists lists;
    const int lv = find(k, &lists);
    weak_node_array& succs = lists.second;
    if (lv == -1) return end();
    shared_node succ = succs[lv].lock();
    if (!succ) return end();
    if (succ->fullylinked && !succ->marked) {
      return iterator(succ);
    }else{
      return end();
    }
  }

  iterator lower_bound(const key& k) {
    nodelists lists;
    const int lv = find(k, &lists);
    weak_node_array& currs = lists.first;
    weak_node_array& succs = lists.second;
    shared_node curr = currs[0].lock();
    if (lv == -1) return iterator(curr);
    shared_node succ = succs[lv].lock();
    if (!succ) return end();
    if (succ->fullylinked && !succ->marked) {
      return iterator(succ);
    }else{
      shared_node curr = currs[0].lock();
      return iterator(curr);
    }
  }

  bool add(const key& k, const value& v) {
    const int top_layer = random_level();
    assert(top_layer < height);
    nodelists lists;
    std::auto_ptr<node_t> newnode(new node_t(k, v, top_layer));
    while(true) {
      const int lv = find(k, &lists);
      weak_node_array& preds = lists.first;
      weak_node_array& succs = lists.second;

      if (lv != -1) {
        const node_t* found = succs[lv].lock().get();
        if (!found) continue;
        if (!found->marked) {
          while(!found->fullylinked) {;}
          return false;
        }
        usleep(1);
        continue;
      }
      bool valid = true;


      // get shared_ptr from weak_ptr array
      locked_node_array locked_preds;
      locked_node_array locked_succs;
      for(int i=0;i<=top_layer;++i) {
        locked_preds[i] = preds[i].lock();
        locked_succs[i] = succs[i].lock();
        if (!locked_preds[i] || !locked_succs[i]) {valid = false; break;}
      }
      if (!valid) {
        continue;
      }

      node_t *prev_pred = NULL;
      std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
      for(int layer = 0; valid && (layer <= top_layer); ++layer) {
        node_t* pred = locked_preds[layer].get();
        const node_t* succ = locked_succs[layer].get();

        if (pred != prev_pred) {
          pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
          prev_pred = pred;
        }
        assert(pred);
        assert(succ);
        valid = !pred->marked
          && !succ->marked
          && pred->next[layer].get() == succ;

      }
      if (!valid) {
//        std::cerr << "unlock \n";
        continue; // unlock all
      }

      // start to insert
      for(int i = 0;i<=top_layer; ++i) {
        newnode->next[i] = shared_node(succs[i]);
        //std::cerr << "[" << newnode->next[i] << "]f";
      }

      shared_node newnode_insert(newnode.get());
      newnode.release(); // it's all reason why I use auto_ptr
      for(int layer = 0;layer <= top_layer; ++layer) {
        locked_preds[layer]->next[layer] = newnode_insert;
      }
      newnode_insert->fullylinked = true;
      return true;
    }
    assert(!"never reach");
  }

  iterator& begin() { return shared_head;}
  const iterator& begin() const { return shared_head;}
  iterator& end() { return shared_tail;}
  const iterator& end() const { return shared_tail;}

  bool remove(const key& k) {
    nodelists lists;
    bool is_marked = false;
    int top_layer = -1;
    shared_node victim;
    assert(victim.get() == NULL);
    while(true) {

      int lv = find(k, &lists);
      weak_node_array& preds = lists.first;
      weak_node_array& succs = lists.second;

      if (lv != -1) {
        victim = succs[lv].lock();
        if (!victim) return false;
      }
      if (is_marked || 
         (lv != -1 && (victim->fullylinked
                       && victim->top_layer == lv 
                       && !victim->marked))) {
        if (is_marked == false) { // only one time done
          top_layer = victim->top_layer;
          victim->guard.lock(); // lock
          if (victim->marked) {
            victim->guard.unlock();
            return false;
          }
          victim->marked = true;
          is_marked = true;
        }

        bool valid = true;
        // get shared_ptr from weak_ptr array
        locked_node_array locked_preds;
        locked_node_array locked_succs;
        for(int i=0;i<=top_layer;++i) {
          locked_preds[i] = preds[i].lock();
          locked_succs[i] = succs[i].lock();
          if (!locked_preds[i] || !locked_succs[i]) {valid = false; break;}
        }
        if (!valid) {
          continue;
        }

        node_t *prev_pred = NULL;
        std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
        for(int layer = 0; valid && (layer <= top_layer); ++layer) {
          node_t *pred = locked_preds[layer].get();
          const node_t *succ = locked_succs[layer].get();
          if (pred != prev_pred) {
            pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
            prev_pred = pred;
          }
          valid = !pred->marked && pred->next[layer].get() == succ;
        }
        if (!valid) {
          continue;
        }

        for(int layer = top_layer; layer>=0; --layer) {
          locked_preds[layer]->next[layer] = victim->next[layer];
        }
        victim->guard.unlock();
        return true;
      }else 
        return false; 
    }
  }

  int find(const key& target, nodelists* lists) {
    int found = -1;

    shared_node pred = shared_head.get();
    shared_node curr;
    *lists = nodelists();
    //lists->second.resize(height);
    for(int lv = height-1; lv >= 0; --lv) {
      curr = pred->next[lv];
      while(curr->first < target) {
        pred = curr;
        curr = pred->next[lv];
      }
      if (found == -1 && target == curr->first) { found = lv;}
      lists->first[lv] = weak_node(pred);
      lists->second[lv] = weak_node(curr);
    }

    return found;
  }

  void dump() const {
    const node_t* p = &head;
    while(p != NULL) {
      p->dump();
      p = p->next[0].get();
      std::cerr << std::endl;
    }
  }

  bool is_empty() const {
    return head.next[0].get() == end().get().get();
  }

  bool clear() {
    while(shared_head->next[0] != shared_tail.get()) {
      for(int i=0;i<height;i++) {
        shared_tail->next[i] = shared_tail->next[0]->next[0];
      }
    }
  }
public:
  uint32_t random_level() const {
    const uint32_t gen = rand();
    int bit = 1; int cnt=0;
    while(cnt < height-1) {
      if (gen & bit) {
        return cnt;
      }
      bit <<= 1;++cnt;
    }
    return cnt;
  }
private:
  sl();
  sl(const sl_t&);
  sl_t& operator=(const sl_t&);
};

}  // namespace nanahan
#endif
