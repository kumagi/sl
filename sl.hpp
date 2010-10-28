#ifndef SL_HPP
#define SL_HPP
#include "node.hpp"
#include <boost/foreach.hpp>
#include <boost/random.hpp>
#define BOOST_SP_DISABLE_THREADS
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <utility>

template <typename key,typename value, int height = 8>
class sl{
	typedef node<key,value> node_t;
	typedef boost::array<node_t*, height> node_array;
	typedef typename node_t::next_array node_vector;

	typedef std::pair<node_array,node_vector> nodelists;
	typedef typename node_t::scoped_lock scoped_lock;
	
	typedef boost::shared_ptr<scoped_lock> scoped_lock_ptr;
	typedef boost::optional<typename node_t::scoped_lock> optional_lock;

	node_t head;
	node_t tail;
	boost::mt19937 engine;
	boost::uniform_smallint<> range;
	mutable boost::variate_generator<boost::mt19937&, boost::uniform_smallint<> > rand;
public:
	sl(const key& min, const key& max, int randomseed = 0)
		:head(min ,value(), height), tail(max, value(), height)
		,engine(static_cast<unsigned long>(randomseed)),range(0,(1<<height) -1)
		,rand(engine,range){
		BOOST_FOREACH(node_t*& p, head.next){p = &tail;}
		head.fullylinked = true;
	}
	~sl(){
		node_t* ptr = head.next[0];
		while(ptr != &tail){
			node_t* next = ptr->next[0];
			delete ptr;
			ptr = next;
		}
	}
	bool contains(const key& k){
		nodelists lists;
		const int lv = find(k, &lists);
		node_vector& succs = lists.second;
		return (lv != -1 
						&& succs[lv]->fullylinked
						&& !succs[lv]->marked);
	}
	bool add(const key& k, const value& v){
		const int top_layer = random_level();
		assert(top_layer < height);
		nodelists lists;
		std::auto_ptr<node_t> newnode(new node_t(k, v, top_layer));
		while(true){
			const int lv = find(k, &lists);
			node_array& preds = lists.first;
			node_vector& succs = lists.second;
			
			if(lv != -1){
				const node_t* found = succs[lv];
				if(!found->marked){
					while(!found->fullylinked){;}
					return false;
				}
				usleep(1);
				continue;
			}
			
			node_t *prev_pred = NULL;
			bool valid = true;
			std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
			for(int layer = 0; valid&& (layer <= top_layer); ++layer){
				node_t *pred = preds[layer];
				const node_t *succ = succs[layer];
				if(pred != prev_pred){
					pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
					prev_pred = pred;
				}
				valid = !pred->marked 
					&& !succ->marked
					&& pred->next[layer] == succ;
			}
			if(!valid){
				continue; // unlock all
			}

			// start to insert
			newnode->next.swap(succs);
			node_t* newnode_insert = newnode.get();
			newnode.release();
			for(int layer = 0;layer <= top_layer; ++layer){
				preds[layer]->next[layer] = newnode_insert;
			}
			newnode_insert->fullylinked = true;
			return true;
		}
		assert(!"never reach");
	}
	
	bool remove(const key& k){
		nodelists lists;
		bool is_marked = false;
		int top_layer = -1;
		while(true){
			node_t* node_to_delete = NULL;
			
			int lv = find(k, &lists);
			node_array& preds = lists.first;
			node_vector& succs = lists.second;
			if(is_marked || (lv != -1 && ok_to_delete(succs[lv],lv))){
				if(node_to_delete == NULL){ // only one time done
					node_to_delete = succs[lv];
					top_layer = node_to_delete->top_layer;
					node_to_delete->guard.lock(); // lock
					if(node_to_delete->marked){
						node_to_delete->guard.unlock();
						return false;
					}
					node_to_delete->marked = true;
					is_marked = true;
				}

				node_t *prev_pred = NULL;
				bool valid = true;
				std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
				for(int layer = 0; valid && (layer <= top_layer); ++layer){
					node_t *pred = preds[layer];
					const node_t *succ = succs[layer];
					if(pred != prev_pred){
						pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
						prev_pred = pred;
					}
					valid = !pred->marked && pred->next[layer] == succ;
				}
				if(!valid){
					continue;
				}
				
				for(int layer = top_layer; layer>=0; --layer){
					preds[layer]->next[layer] = node_to_delete->next[layer];
				}
				node_to_delete->guard.unlock();
				delete node_to_delete;
				return true;
			}else 
				return false; 
		}
	}
	
	int find(const key& target, nodelists* lists){
		int found = -1;
		node_t* pred = &head;
		*lists = nodelists();
		lists->second.resize(height);
		for(int lv = height-1; lv >= 0; --lv){
			node_t* curr = pred->next[lv];
			while(curr->k < target){
				pred = curr; curr = pred->next[lv];
			}
			if(found == -1 && target == curr->k){ found = lv; }
			lists->first[lv] = pred;
			lists->second[lv] = curr;
		}
		return found;
	}
	
	void dump()const{
		const node_t* p = &head;
		while(p != NULL){
			p->dump();
			p = p->next[0];
			std::cerr << std::endl;
		}
	}
	
	bool empty()const{
		return head.next[0] == &tail;
	}
public:
	uint32_t random_level()const {
		const uint32_t gen = rand();
		int bit = 1; int cnt=0;
		while(cnt < height-1){
			if(gen & bit) {return cnt;}
			bit <<= 1;++cnt;
		}
		return cnt;
	}
private:
	static bool ok_to_delete(const node_t* const candidate, size_t lv){
		return (candidate->fullylinked 
						&& candidate->top_layer == lv
						&& !candidate->marked);
	}
	sl();
};



#endif
