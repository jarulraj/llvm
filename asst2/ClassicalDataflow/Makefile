all: liveness.so inputs

CXX = clang
CXXFLAGS = $(shell llvm-config --cxxflags) -fcolor-diagnostics -g -O0
OPT = opt
TEST = test-inputs

dataflow.o: dataflow.cpp dataflow.h

liveness.o: liveness.cpp available-support.h

available-support.o: available-support.cpp available-support.h	

%.so: %.o dataflow.o available-support.o
	$(CXX) -dylib -shared $^ -o $@
 
# TESTING
inputs : $(patsubst %.c,%.bc,$(wildcard $(TEST)/*.c)) 
 
%.tmp: %.c
	$(CXX) -O0 -emit-llvm -c $^ -o $@ 

%.bc: %.tmp
	$(OPT) -mem2reg $^ -o $@
 
# PDF
pdf: $(patsubst %.cpp,%.pdf,$(wildcard ./*.cpp))  $(patsubst %.c,%.pdf,$(wildcard $(TEST)/*.c))   

%.pdf: %.c 
	enscript -q --color=1 -j -G -DDuplex:true -r -E -fCourier7 --tabsize=2 -p listing.ps $^
	ps2pdf listing.ps $@

%.pdf: %.cpp 
	enscript -q --color=1 -j -G -DDuplex:true -r -E -fCourier7 --tabsize=2 -p listing.ps $^
	ps2pdf listing.ps $@
 
# CLEAN
clean:
	rm -f *.o *~ *.so *.pdf $(TEST)/*.bc out $(TEST)/*.pdf                 
