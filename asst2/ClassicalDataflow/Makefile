all: liveness.so available.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

dataflow.o: dataflow.cpp dataflow.h

available-support.o: available-support.cpp available-support.h	

%.so: %.o dataflow.o available-support.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

.PHONY: clean all
