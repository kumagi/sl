CXX=g++
OPTS=-O0 -fexceptions -g -coverage
OPTIMIZE=-O4
LD=-L/usr/local/lib -lboost_program_options -lpthread -lboost_thread -lgcov
OPTIMIZE_LD=-L/usr/local/lib -lboost_program_options -lpthread -lboost_thread -lboost_date_time
TEST_LD= -lpthread $(LD)
GTEST_INC= -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_DIR=/opt/google/gtest-1.5.0
GMOCK_DIR=/opt/google/gmock-1.5.0
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

target:test
#target:bench
#target:bench2
#target:pg_bench

test:sl.o gtest_main.a
	$(CXX) -o $@ $^ $(LD) $(OPTS) $(WARNS)
	./test $(NOTIFY)
sl.o:sl.cc sl.hpp node.hpp
	$(CXX) -c -o $@ sl.cc  $(OPTS) $(WARNS) -I$(GTEST_DIR)/include -I$(GTEST_DIR)
bench:bench.o
	$(CXX) -o $@ $^ $(LD) $(OPTS) $(WARNS) 
	./bench $(NOTIFY)
	gcov bench.gcda > /dev/null
bench.o:bench.cc sl.hpp node.hpp
	$(CXX) -c -o $@ bench.cc  $(OPTS) $(WARNS)
bench2:bench2.o
	$(CXX) -o $@ $^ $(OPTIMIZE_LD) $(OPTIMIZE) $(WARNS) -DNDEBUG -g
	./bench2 $(NOTIFY)
bench2.o:bench.cc sl.hpp node.hpp
	$(CXX) -c -o $@ bench.cc  $(OPTIMIZE) $(WARNS) -DNDEBUG -g
pg_bench:pg_bench.o
	$(CXX) -o $@ $^ $(OPTIMIZE_LD) $(WARNS) -O4 -fno-inline -pg -g
	./pg_bench $(NOTIFY)
	gprof pg_bench > profile
pg_bench.o:bench.cc sl.hpp node.hpp 
	$(CXX) -c -o $@ bench.cc $(WARNS) -O4 -fno-inline -pg -g

# gtest
gtest_main.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest_main.cc -o $@
gtest-all.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest-all.cc -o $@
gtest_main.a:gtest-all.o gtest_main.o
	ar -r $@ $^

libgmock.a:
	g++ ${GTEST_INC} -I${GTEST_DIR} -I${GMOCK_DIR}/include -I${GMOCK_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	g++ ${GTEST_INC} -I${GTEST_DIR} -I${GMOCK_DIR}/include -I${GMOCK_DIR} -c ${GMOCK_DIR}/src/gmock-all.cc
	ar -rv libgmock.a gtest-all.o gmock-all.o
# mtrace
mtrace_on.so:mtrace_on.c
	g++ $^ -o $@ -shared
clean:
	rm -f *.o
	rm -f *~
