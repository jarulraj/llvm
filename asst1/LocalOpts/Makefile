all: LocalOpts.so inputs
	
CXX = clang
CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0
OPT = opt

%.so: %.o
	$(CXX) -dylib -shared $^ -o $@

TEST = test-inputs
inputs : $(TEST)/algebraic.bc $(TEST)/constfold.bc $(TEST)/strength.bc
 
%.tmp: %.c
	$(CXX) -O0 -emit-llvm -c $^ -o $@ 

%.bc: %.tmp
	$(OPT) -mem2reg $^ -o $@

clean:
	rm -f *.o *~ *.so $(TEST)/*.bc out
