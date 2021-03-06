CXXFLAGS += -g -W -Wall -Werror -m64 -march=core2 -O3 -pipe $(ADDFLAGS) 
AS=$(CXX) -c $(CXXFLAGS)

OBJS=opensieve.o arithmetic.o
SRCS=$(wildcard *.cc)
BINS=test_general test_perf test_print

all: $(BINS)

test_general: $(SRCS) $(OBJS) test_general.o 
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $@.o

test_perf: $(SRCS) $(OBJS) test_perf.o 
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $@.o

test_print: $(SRCS) $(OBJS) test_print.o
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $@.o

clean:
	rm -fv $(BINS) .depend *.o *~ */*~

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

include .depend

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

