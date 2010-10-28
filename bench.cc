#define NDEBUG
#include <limits.h>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/random.hpp>
#include <boost/timer.hpp>

#include "sl.hpp"

const int nodes = 2000000;
using namespace std;
using namespace boost;

typedef sl<int,int, 21> skip;

skip skiplist(INT_MIN, INT_MAX, 0);

template<typename rand>
void inserter(int i, rand& rnd, int seed){
	while(i>0){
		skiplist.add(rnd(),1);
		--i;
	}
};
template<typename rand>
void deleter(int i, rand& rnd, int seed){
	
	while(i>0){
		skiplist.remove(rnd());
		--i;
	}
};

int main(){
	using namespace boost;
	
	typedef variate_generator<mt19937&, uniform_int<> > random_type;
	mt19937 gen1(1),gen2(2),gen3(3),gen4(4);
	mt19937 gen5(1),gen6(2),gen7(3),gen8(4);
	uniform_int<> dst(0, 10000000);
	
	boost::timer t;
	boost::thread ins1(boost::bind(&::inserter<random_type>, nodes/2, random_type(gen1, dst), 1));
	boost::thread ins2(boost::bind(&::inserter<random_type>, nodes/2, random_type(gen2, dst), 2));
//	boost::thread ins3(boost::bind(&::inserter<random_type>, nodes/4, random_type(gen3, dst), 3));
//	boost::thread ins4(boost::bind(&::inserter<random_type>, nodes/4, random_type(gen4, dst), 4));
//	usleep(100);
	//boost::thread del1(boost::bind(&::deleter<random_type>, nodes/4, random_type(gen5, dst), 5));
//	boost::thread del2(boost::bind(&::deleter<random_type>, nodes/4, random_type(gen6, dst), 6));
//	boost::thread del3(boost::bind(&::deleter<random_type>, nodes/4, random_type(gen7, dst), 7));
//	boost::thread del4(boost::bind(&::deleter<random_type>, nodes/4, random_type(gen8, dst), 8));

//	skiplist.dump();
	ins1.join();ins2.join();//ins3.join();ins4.join();
//	del1.join();del2.join();del3.join();del4.join();
	
	int time = t.elapsed();
	if(time == 0) time++;
	//date_time::subsecond_duration<time_duration, long long int>
	
	std::cout << nodes/time << " ops" << std::endl;
}

