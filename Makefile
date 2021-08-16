#Please specify OPENSSL_HOME to your openssl@1.1.1
OPENSSL_HOME = /usr/local
CPPFLAGS = -I./include -I$(OPENSSL_HOME)/include -std=c++20 # -D_DEBUG
CFLAGS =
CXXFLAGS = -O3 -fexceptions
STATIC = lib/libvdf.a

OPENSSL_LDFLAGS = -L$(OPENSSL_HOME)/lib -lssl -lcrypto
PKG_LDFLAGS = -Llib -lvdf
GMP_LDFLAGS = -lgmpxx -lgmp
MPFR_LDFLAGS = -lmpfr

LDFLAGS = $(OPENSSL_LDFLAGS) $(GMP_LDFLAGS) $(MPFR_LDFLAGS)
TEST_LDFLAGS = $(PKG_LDFLAGS) $(LDFLAGS)

src = $(wildcard src/*.cpp)
objtmp = $(subst src/,obj/,$(src))
obj = $(objtmp:.cpp=.o)
testsrc = $(wildcard test/*.cpp)
testbin = $(testsrc:.cpp=.out)
testdep = $(testsrc:.cpp=.d)
timingsrc = $(wildcard timing_*.cpp)
timingbin = $(timingsrc:.cpp=.out)
timingdep = $(timingsrc:.cpp=.d)

.PHONY: all
all: $(STATIC) test timing 

$(STATIC): $(obj)
	mkdir -p lib
	ar rcs  $@ $^ 

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $< 

.PHONY: test
test: $(testbin) $(testdep)

test/%.d: test/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXLAGS) $< -MM -MT $(@:.d=.o) >$@

-include $(testdep)

test/%.out: test/%.cpp  test/%.d $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< $(TEST_LDFLAGS)  

.PHONY: timing
timing: $(timingbin) $(timingdep)

./%.d: ./%.cpp
	$(CXX) $(CPPFLAGS) $(CXXLAGS) $< -MM -MT $(@:.d=.o) >$@

-include $(timingdep)

./%.out: ./%.cpp ./%.d $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< $(TEST_LDFLAGS)  

.PHONY: run
run: $(testbin)
	for testfile in $(testbin); do \
		echo $$testfile; \
	  ./$$testfile; \
	done

.PHONY: clean
clean:
	rm -f $(obj) $(TARGET) $(testbin) $(testdep) $(timingbin) $(timingdep) 

