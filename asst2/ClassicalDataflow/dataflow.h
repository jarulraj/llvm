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

namespace llvm {

    typedef std::vector<BitVector> BitVectorList;
    typedef std::vector<BasicBlock*> BasicBlockList;

/*	For storing the output of a transfer function.
        We also store a list of BitVectors corresponding to predecessors/successors used to handle phi nodes) */
    struct TransferOutput {
        BitVector retValue;
        DenseMap<BasicBlock*, BitVector> neighborVals;
    };

/* Stores the IN and OUT sets for a basic block. Also a variable to store the temporary output of the transfer function */
    struct BlockResult {
        BitVector in;
        BitVector out;

        TransferOutput tempTransferOutput;
    };

/* Result of Data Flow Analysis */
    struct DataFlowResult {

        /* Mapping from basic blocks to their results */
        DenseMap<BasicBlock*, BlockResult> result;

        /* Mapping from domain values to indices in bitvectors (to figure out which bits are which values) */
        DenseMap<Value*, int> domainToIndex;

    };

/* Basic Class for Data flow analysis. Specific analyses must extend this */
    class DataFlow {
    public:
        enum Direction {
            FORWARD,
            BACKWARD
        };

        enum MeetOp {
            UNION,
            INTERSECTION
        };

        /* Storing The Meet Operator */
        MeetOp _op;

        /** Apply meet operator */
        BitVector applyMeetOp(BitVectorList meetInputs);

        /** Run analysis on Function F */
        DataFlowResult run(Function& F, std::vector<Value*> domain, Direction direction, MeetOp op, BitVector boundaryCond, BitVector initCond);

    protected:
        /*      Transfer Function: To be implmented by the specific analysis.
                Takes one set (IN/OUT), domain to index mapping, basic block, and outputs the other set (OUT/IN) */
        virtual TransferOutput transferFn(BitVector input, DenseMap<Value*, int> domainToIndex, BasicBlock* block) = 0;
    };

}

#endif
