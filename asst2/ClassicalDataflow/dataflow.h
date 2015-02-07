// 15-745 S15 Assignment 2: dataflow.h
// Group: jarulraj, nkshah
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_DATAFLOW_H__

// Some useful libraries. Change as you see fit
#include <stdio.h>
#include <iostream>
#include <vector>
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <map>
#include "llvm/ADT/PostOrderIterator.h"

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
                                          std::map<void*, int> domainToIndex, BasicBlock* block) = 0;

    private:

        /* Pass-specific parameters */
        Direction direction;
        MeetOp meetup_op;
    };

}

#endif
