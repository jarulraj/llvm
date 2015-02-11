// 15-745 S15 Assignment 2: dataflow.h
// Group: jarulraj, nkshah
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

// DEBUG mode
#define DEBUG 1

#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

namespace llvm {

    typedef std::vector<BitVector> BitVectorList;
    typedef std::vector<BasicBlock*> BasicBlockList;

    /*	For storing the output of a transfer function.
        We also store a list of BitVectors corresponding to predecessors/successors used to handle phi nodes) */
    struct TransferOutput {
        BitVector element;
        std::map<BasicBlock*, BitVector> neighborVals;
    };

    /* Stores the IN and OUT sets for a basic block. Also a variable to store the temporary output of the transfer function */
    struct BlockResult {
        BitVector in;
        BitVector out;

        TransferOutput transferOutput;
    };

    enum Direction {
        INVALID_DIRECTION,
        FORWARD,
        BACKWARD
    };

    enum MeetOp {
        INVALID_MEETOP,
        UNION,
        INTERSECTION
    };

    /* Result of pass on a function */
    struct DataFlowResult {
        /* Mapping from basic blocks to their results */
        std::map<BasicBlock*, BlockResult> result;

        /* Mapping from domain elements to indices in bitvectors
           (to figure out which bits are which values) */
        std::map<void*, int> domainToIndex;

        bool modified;
    };

    /* Basic Class for Data flow analysis. Specific analyses must extend this */
    class DataFlow {
    public:

    DataFlow(Direction direction, MeetOp meetup_op)
        : direction(direction), meetup_op(meetup_op)
        {
        }

        /* Applying Meet Operator */
        BitVector applyMeetOp(BitVectorList inputs);

        /* Apply analysis on Function F */
        DataFlowResult run(Function &F, std::vector<void*> domain,
                           BitVector boundaryCond, BitVector initCond);

    protected:
        /*      Transfer Function: To be implmented by the specific analysis.
                Takes one set (IN/OUT), domain to index mapping, basic block, and outputs the other set (OUT/IN) */
        virtual TransferOutput transferFn(BitVector input,
                                          std::vector<void*> domain,
                                          std::map<void*, int> domainToIndex,
                                          BasicBlock* block) = 0;

    private:

        /* Pass-specific parameters */
        Direction direction;
        MeetOp meetup_op;
    };

    // Pretty printing utility functions

    void printBitVector(BitVector b);

    void printResult(DataFlowResult output);

    std::string printValue(Value* v);

    std::string printSet(std::vector<void*> domain, BitVector on, int mode);

    // EXPRESSION-RELATED UTILS

    class Expression {
    public:
        Value * v1;
        Value * v2;
        Instruction::BinaryOps op;
        Expression (Instruction * I);
        bool operator== (const Expression &e2) const;
        bool operator< (const Expression &e2) const;
        std::string toString() const;
    };

    std::string getShortValueName(Value * v);

    // Silly code to print out a set of expressions in a nice format
    void printSet(std::vector<Expression> * x);

}

#endif
