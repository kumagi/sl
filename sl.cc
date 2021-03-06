#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include <limits.h>
#include <boost/shared_ptr.hpp>

#include "sl.hpp"

using std::string;
using boost::shared_ptr;

TEST(construct, first){
	sl<string,shared_ptr<int>,8> i("0", "~~~~");
}

TEST(insert, one){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"tmp"));
	std::cerr << "insert ok" << std::endl;
}

TEST(insert, one_contains){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"tmp"));
	EXPECT_TRUE(!dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(!dat.contains(3));
	dat.dump();
}
TEST(insert, two){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"v2"));
	EXPECT_TRUE(dat.add(3,"v3"));
	dat.dump();
}

TEST(insert, two_contains){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"tmp"));
	EXPECT_TRUE(dat.add(3,"v3"));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.contains(3));
	dat.dump();
}
TEST(insert, collide){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"tmp"));
	EXPECT_TRUE(!dat.add(2,"tmp"));
	EXPECT_TRUE(dat.contains(2));
}

TEST(remove, one){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(2,"tmp"));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.remove(2));
	EXPECT_TRUE(!dat.contains(2));
}

TEST(remove, head){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(1,"v1"));
	EXPECT_TRUE(dat.add(2,"v2"));
	EXPECT_TRUE(dat.add(3,"v3"));

	EXPECT_TRUE(dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.contains(3));
	EXPECT_TRUE(dat.remove(1));
	EXPECT_TRUE(!dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.contains(3));

}

TEST(remove, middle){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(1,"v1"));
	EXPECT_TRUE(dat.add(2,"v2"));
	EXPECT_TRUE(dat.add(3,"v3"));
	EXPECT_TRUE(dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.contains(3));
	EXPECT_TRUE(dat.remove(2));
	EXPECT_TRUE(dat.contains(1));
	EXPECT_TRUE(!dat.contains(2));
	EXPECT_TRUE(dat.contains(3));
}

TEST(remove, tail){
	sl<int, string,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.add(1,"v1"));
	EXPECT_TRUE(dat.add(2,"v2"));
	EXPECT_TRUE(dat.add(3,"v3"));
	EXPECT_TRUE(dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(dat.contains(3));
	EXPECT_TRUE(dat.remove(3));
	EXPECT_TRUE(dat.contains(1));
	EXPECT_TRUE(dat.contains(2));
	EXPECT_TRUE(!dat.contains(3));
}

TEST(empty,test){
	sl<int, int,4> dat(INT_MIN,INT_MAX);
	EXPECT_TRUE(dat.is_empty());
	EXPECT_TRUE(dat.add(2,3));
	EXPECT_FALSE(dat.is_empty());
}

TEST(get, failue){
	sl<int, int,4> dat(INT_MIN,INT_MAX);
	for(int i=0;i<10;i++){
		EXPECT_TRUE(dat.get(i) == dat.end());
	}
}

TEST(get, succeed){
	sl<int, int,4> dat(INT_MIN,INT_MAX);
	for(int i=0;i<10;i++){
		dat.add(i,i*2);
	}
	for(int i=0;i<10;i++){
		EXPECT_TRUE(dat.get(i)->second == i*2);
	}
}


TEST(random_add, 500){
	sl<int, int, 6> dat(INT_MIN,INT_MAX);
	for(int i=0;i<500;i++){
		EXPECT_TRUE(dat.add(i,3));
	}
	for(int i=0;i<500;i++){
		EXPECT_TRUE(dat.contains(i));
	}
	for(int i=0;i<500;i++){
		EXPECT_TRUE(dat.remove(i));
	}
	for(int i=0;i<500;i++){
		EXPECT_FALSE(dat.contains(i));
	}
}

TEST(iterator, get){
	typedef sl<int,int,4> sl;
	sl dat(INT_MIN,INT_MAX);
	dat.add(12,1);
	sl::iterator it = dat.get(21);
	EXPECT_TRUE(it == dat.end());
	sl::iterator jt = dat.get(12);
	EXPECT_TRUE(jt->first == 12);
	EXPECT_TRUE(jt->second == 1);
}

TEST(iterator, inc){
	typedef sl<int,int,4> sl;
	sl dat(INT_MIN,INT_MAX);
	dat.add(12,1);
	dat.add(13,1);
	sl::iterator it = dat.lower_bound(12);
	EXPECT_TRUE(it->first == 12);
	++it;
	EXPECT_TRUE(it->first == 13);
	++it;
	EXPECT_TRUE(it == dat.end());
}
