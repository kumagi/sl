#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

template <typename key, typename value>
struct node : public boost::noncopyable{
	typedef node<key,value> node_t;
	typedef boost::shared_ptr<node_t> shared_node;
	typedef std::vector<shared_node> next_array;
	
	typedef boost::mutex::scoped_lock scoped_lock;
	const key k;
	const value v;
	const size_t top_layer;
	boost::mutex guard;
	next_array next;
	volatile bool marked;
	volatile bool fullylinked;
	node(const key& k_, const value& v_, size_t top)
		:k(k_),v(v_),top_layer(top),marked(false),fullylinked(false){
		if(next.size() != top+1){
			next.resize(top+1);
		}
	}
	~node(){
		//std::cerr << "ptr:" << this << " key:" << k << " value:" << v << " removed\n" ;
	}
	void dump()const{
		std::cerr << "key:" << k << " value:" << v << " lv:" << top_layer << " nexts[";
		{
			boost::io::ios_flags_saver dec(std::cerr);
			for(size_t i=0;i<=top_layer;i++){
				if(next[i] != NULL){
					std::cerr << i << ":" << next[i]->k << " ";
				}else{
					std::cerr << i << ":NULL ";
				}
			}
		}
		std::cerr << "] marked:" << marked << " fullylinked:" << fullylinked;
	}
} __attribute__((aligned (64)));


#endif
