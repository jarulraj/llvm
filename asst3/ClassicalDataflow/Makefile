all: dominance.so \
	LoopInvariantCodeMotion.so \
	DeadCodeElimination.so \
	inputs 

CXX = clang
CXXFLAGS = $(shell llvm-config --cxxflags) -fcolor-diagnostics -g -O0
OPT = opt
TEST = tests

LoopInvariantCodeMotion.so: LoopInvariantCodeMotion.o dominance.o dataflow.o
	$(CXX) -shared $^ -o $@

%.so: %.o dataflow.o
	$(CXX) -shared $^ -o $@

# TESTING
inputs : $(patsubst %.c,%.bc,$(wildcard $(TEST)/*.c)) $(patsubst %.c,%.ll,$(wildcard $(TEST)/*.c))

%.tmp: %.c
	$(CXX) -O0 -emit-llvm -c $^ -o $@

%.bc: %.tmp
	$(OPT) -mem2reg $^ -o $@

%.ll: %.bc
	llvm-dis $^

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
	rm -f *.o *~ *.so *.pdf out $(TEST)/*.pdf
