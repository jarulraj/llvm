// 15-745 S15 Assignment 2: dataflow.cpp
// Group: jarulraj, nkshah
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"
#include <vector>
#include "llvm/ADT/PostOrderIterator.h"

using namespace std;

namespace llvm {

    /* Applying Meet Operator */
    template<class Element>
    BitVector DataFlow<Element>::applyMeetOp(BitVectorList inputs) {
        BitVector result;

        if (!inputs.empty()) {
            for (int i = 0; i < inputs.size(); i++) {
                if (i == 0) {
                    result = inputs[i];
                }
                else {
                    if (meetup_op == MeetOp::UNION) {
                        result |= inputs[i];
                    }
                    else if (meetup_op == MeetOp::INTERSECTION) {
                        result &= inputs[i];
                    }
                    else{
                        outs() << "Unknown Meetop \n";
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        return result;
    }

    /* Apply analysis on Function F */
    template<class Element>
    DataFlowResult<Element> DataFlow<Element>::apply(Function &F) {
        DenseMap<BasicBlock*, BlockResult> result;

        // Map domain values to index in bitvector
        DenseMap<Element, int> domainToIndex;
        for (int i = 0; i < domain.size(); i++)
            domainToIndex[domain[i]] = i;

        // Find Boundary Blocks
        BasicBlockList boundaryBlocks;
        switch (direction) {

        case Direction::FORWARD:
            // Entry = first block
            boundaryBlocks.push_back(&F.front());
            break;

        case Direction::BACKWARD:
            // TODO: Improve this. What if the function has exit(0)? Or use CFGNode from LLVM?
            // Exit = blocks with a return statement (could be multiple)
            for(Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
                if (isa<ReturnInst>(I->getTerminator()))
                    boundaryBlocks.push_back(I);
            break;
        }

        // Initialize Boundary Blocks
        BlockResult boundaryRes = BlockResult();
        // Forward analysis => Initialize IN, Backward analysis => OUT
        BitVector* value = (direction == Direction::FORWARD) ? &boundaryRes.in : &boundaryRes.out;

        *value = boundaryCond;
        boundaryRes.transferOutput.element = boundaryCond;
        for (BasicBlockList::iterator I = boundaryBlocks.begin(), E = boundaryBlocks.end(); I != E; ++I) {
            result[*I] = boundaryRes;	// TODO: If we run into errors, this might be a cause (pointer problems!)
        }

        // Initialize Interior Blocks
        BlockResult intRes = BlockResult();
        // Forward analysis => Initialize IN, Backward analysis => OUT
        BitVector* intVal = (direction == Direction::FORWARD) ? &intRes.out : &intRes.in;

        *intVal = initCond;
        intRes.transferOutput.element = initCond;
        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
            if (std::find(boundaryBlocks.begin(),boundaryBlocks.end(),(BasicBlock*)BB) == boundaryBlocks.end()) {
                // If it is not one of the boundary blocks
                result[(BasicBlock*)BB] = intRes;
            }
        }

        // Generate "neighbor" list: For forward analysis, these are predecessors, for backward analysis these are successors
        // So we won't have to switch on direction every time
        DenseMap<BasicBlock*, BasicBlockList > blockNeighbors;

        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
            BasicBlockList neighborList;

            switch (direction) {

            case Direction::FORWARD: {
                for (pred_iterator neighbor = pred_begin(BB), E = pred_end(BB); neighbor != E; ++neighbor)
                    neighborList.push_back(*neighbor);
            }
                break;

            case Direction::BACKWARD: {
                for (succ_iterator neighbor = succ_begin(BB), E = succ_end(BB); neighbor != E; ++neighbor)
                    neighborList.push_back(*neighbor);
            }
                break;

            }

            blockNeighbors[BB] = neighborList;
        }

        // Prepare an order in which we will traverse BasicBlocks. This is to prevent having to write analysis code twice (for each direction)
        BasicBlockList traversalOrder;

        switch (direction) {

        case Direction::FORWARD: {
            ReversePostOrderTraversal<Function*> TR(&F);
            for (ReversePostOrderTraversal<Function*>::rpo_iterator I = TR.begin(), E = TR.end(); I != E; ++I) {
                traversalOrder.push_back(*I);
            }
        }
            break;

        case Direction::BACKWARD: {
            for (po_iterator<BasicBlock*> I = po_begin(&F.getEntryBlock()), E = po_end(&F.getEntryBlock()); I != E; ++I) {
                traversalOrder.push_back(*I);
            }
        }
            break;

        }

        // Keep processing blocks until convergence
        bool converged = false;
        while (!converged) {
            converged = true;

            for (BasicBlockList::iterator BB = traversalOrder.begin(), BE = traversalOrder.end(); BB != BE; ++BB) {

                // Get the current result for the block
                BlockResult& blockRes = result[*BB];

                // Store it to later check if it changed
                BlockResult oldBlockRes = blockRes;
                BitVector oldVal = (direction == Direction::FORWARD) ? blockRes.out : blockRes.in;

                // Collect Neighbor Results for Meet
                BitVectorList meetInputs;

                for (BasicBlockList::iterator NI = blockNeighbors[*BB].begin(), NE = blockNeighbors[*BB].end(); NI != NE; ++NI) {
                    BlockResult& neighborRes = result[*NI];
                    BitVector neighVal = neighborRes.transferOutput.element;

                    // Union the value if we find a match with neighbor-specific value
                    DenseMap<BasicBlock*, BitVector>::iterator match = neighborRes.transferOutput.neighborVals.find(*BB);
                    if (match != neighborRes.transferOutput.neighborVals.end()) {
                        neighVal |= match->second;
                    }

                    meetInputs.push_back(neighVal);
                }

                // Apply Meet Operator, Store in (IN/OUT) depending on direction
                BitVector* blockInput = (direction == Direction::FORWARD) ? &blockRes.in : &blockRes.out;
                if (!meetInputs.empty())
                    *blockInput = applyMeetOp(meetInputs);

                //Apply transfer function to input set in order to get output set for this iteration
                blockRes.transferOutput = transferFn(*blockInput, domainToIndex, *BB);
                // TODO: Memoize GEN, KILL (and other stuff) to avoid recomputations
                BitVector* blockOutput = (direction == Direction::FORWARD) ? &blockRes.out : &blockRes.in;
                *blockOutput = blockRes.transferOutput.element;

                // If something changed, set converged to false
                if (converged) {
                    // Either the IN/OUT set itself changed, or a neighbor-specific value changed
                    // (actually, for phi nodes, the size will also change, so can just check that)
                    if (*blockOutput != oldVal ||
                        blockRes.transferOutput.neighborVals.size() != oldBlockRes.transferOutput.neighborVals.size())
                        converged = false;
                }
            }
        }

        // Create result
        DataFlowResult<Element> answer;
        answer.domainToIndex = domainToIndex;
        answer.result = result;

        return answer;
    }

}
