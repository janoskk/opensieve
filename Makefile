CXXFLAGS += -g -W -Wall -Werror -m64 -march=core2 -O3 -pipe 

OBJS=tests.o opensieve.o arithmetic.o
SRCS=$(wildcard *.cc)
BINS=tests

all: $(BINS)
	./tests

tests: $(SRCS) $(OBJS) 
	$(CXX) -o $@ $(LDFLAGS) $(OBJS)

clean:
	rm -fv $(BINS) .depend *.o *~ */*~

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

include .depend

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

