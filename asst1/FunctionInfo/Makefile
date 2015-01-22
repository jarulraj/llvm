all: FunctionInfo.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -fcolor-diagnostics -g -O0

%.so: %.o
	$(CXX) -dylib -shared $^ -o $@
clean:
	rm -f *.o *~ *.so
