#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>

template <typename key, typename value>
struct node : public boost::noncopyable{
	typedef node<key,value> node_t;
	typedef std::vector<node_t*> next_array;
	typedef boost::mutex::scoped_lock scoped_lock;
	const key k;
	const value v;
	const size_t top_layer;
	next_array next;
	volatile bool marked;
	volatile bool fullylinked;
	boost::mutex guard;
	node(const key& k_, const value& v_, size_t top)
		:k(k_),v(v_),top_layer(top),marked(false),fullylinked(false){
		if(next.size() != top+1){
			next.resize(top+1);
		}
	}
	~node(){
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
