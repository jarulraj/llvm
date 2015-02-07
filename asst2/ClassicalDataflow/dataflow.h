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
        DenseMap<BasicBlock*, BitVector> neighborVals;
    };

    /* Stores the IN and OUT sets for a basic block. Also a variable to store the temporary output of the transfer function */
    struct BlockResult {
        BitVector in;
        BitVector out;

        TransferOutput transferOutput;
    };

    /* Result of Data Flow Analysis */
    template<class Element> class DataFlowResult {

        /* Mapping from basic blocks to their results */
        DenseMap<BasicBlock*, BlockResult> result;

        /* Mapping from domain elements to indices in bitvectors
           (to figure out which bits are which values) */
        DenseMap<Element, int> domainToIndex;

    };

    enum Direction {
        FORWARD,
        BACKWARD
    };

    enum MeetOp {
        UNION,
        INTERSECTION
    };

    /* Basic Class for Data flow analysis. Specific analyses must extend this */
    template<class Element> class DataFlow {
    public:
        /* Simple constructor */
        DataFlow<Element>(std::vector<Element> domain,
                          Direction direction, MeetOp meetup_op,
                          BitVector boundaryCond, BitVector initCond)
            : domain(domain), direction(direction), meetup_op(meetup_op),
            boundaryCond(boundaryCond), initCond(initCond)
        {
        }

        /* Apply meet operator */
        BitVector applyMeetOp(BitVectorList meetInputs);

        /* Apply analysis on Function F */
        DataFlowResult<Element> apply(Function& F);

    protected:
        /*      Transfer Function: To be implmented by the specific analysis.
                Takes one set (IN/OUT), domain to index mapping, basic block, and outputs the other set (OUT/IN) */
        virtual TransferOutput transferFn(BitVector input, DenseMap<Element, int> domainToIndex, BasicBlock* block) = 0;

    private:

        /* Pass-specific parameters */
        std::vector<Element> domain;
        Direction direction;
        MeetOp meetup_op;
        BitVector boundaryCond;
        BitVector initCond;
    };

}

#endif
