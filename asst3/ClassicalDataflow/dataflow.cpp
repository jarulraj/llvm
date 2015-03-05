// 15-745 S15 Assignment 2: dataflow.cpp
// Group: jarulraj, nkshah
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

using namespace std;

namespace llvm {

    /* Applying Meet Operator */
    BitVector DataFlow::applyMeetOp(BitVectorList inputs) {
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
    DataFlowResult DataFlow::run(Function &F, std::vector<void*> domain,
                                 BitVector boundaryCond, BitVector initCond) {
        std::map<BasicBlock*, BlockResult> result;
        bool modified = false;

        //DBG(outs() << "APPLY ANALYSIS :: \n");

        // Map domain values to index in bitvector
        std::map<void*, int> domainToIndex;
        for (int i = 0; i < domain.size(); i++)
            domainToIndex[(void*)domain[i]] = i;

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

        default:
            errs() << "Invalid direction \n";
            break;
        }

        //DBG(outs() << "Boundary Blocks :" << boundaryBlocks.size() << "\n");
        //DBG(outs() << "Initialize Boundary Blocks \n");

        // Initialize Boundary Blocks
        BlockResult boundaryRes = BlockResult();
        // Forward analysis => Initialize IN, Backward analysis => OUT
        BitVector* value = (direction == Direction::FORWARD) ? &boundaryRes.in : &boundaryRes.out;

        *value = boundaryCond;
        boundaryRes.transferOutput.element = boundaryCond;
        for (BasicBlockList::iterator I = boundaryBlocks.begin(), E = boundaryBlocks.end(); I != E; ++I) {
            result[*I] = boundaryRes;	// TODO: If we run into errors, this might be a cause (pointer problems!)
        }

        //DBG(outs() << "Initialize Interior Blocks \n");

        // Initialize Interior Blocks
        BlockResult intRes = BlockResult();
        // Forward analysis => Initialize IN, Backward analysis => OUT
        BitVector* intVal = (direction == Direction::FORWARD) ? &intRes.out : &intRes.in;
        BitVector* intOtherVal = (direction == Direction::FORWARD) ? &intRes.in : &intRes.out;

        *intVal = initCond;
        *intOtherVal = initCond;
        intRes.transferOutput.element = initCond;
        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
            if (std::find(boundaryBlocks.begin(),boundaryBlocks.end(),(BasicBlock*)BB) == boundaryBlocks.end()) {
                // If it is not one of the boundary blocks
                result[(BasicBlock*)BB] = intRes;
            }
        }

        //DBG(outs() << "Generate Neighbour list \n");

        // Generate "neighbor" list: For forward analysis, these are predecessors, for backward analysis these are successors
        // So we won't have to switch on direction every time
        std::map<BasicBlock*, BasicBlockList > blockNeighbors;

        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
            BasicBlockList neighborList;

            switch (direction) {
            case Direction::FORWARD:
                for (pred_iterator neighbor = pred_begin(BB), E = pred_end(BB); neighbor != E; ++neighbor)
                    neighborList.push_back(*neighbor);

                break;

            case Direction::BACKWARD:
                for (succ_iterator neighbor = succ_begin(BB), E = succ_end(BB); neighbor != E; ++neighbor)
                    neighborList.push_back(*neighbor);
                break;

            default:
                errs() << "Invalid direction \n";
                break;
            }

            blockNeighbors[BB] = neighborList;

            /*
              DBG(outs() << "NeighborList for block : " << BB->getName() << "\n");
              for(BasicBlock *neighbor : neighborList)
              {
              DBG(outs() << neighbor->getName() << " ");
              }
              DBG(outs() << "\n");
            */
        }

        //DBG(outs() << "Prepare traversal order \n");

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

        default:
            errs() << "Invalid direction \n";
            break;
        }

        //DBG(outs() << "Traversal length : " << traversalOrder.size() << "\n");
        //DBG(outs() << "Start analysis \n");

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
                    std::map<BasicBlock*, BitVector>::iterator match = neighborRes.transferOutput.neighborVals.find(*BB);
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
                blockRes.transferOutput = transferFn(*blockInput, domain, domainToIndex, *BB);
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

        //DBG(outs() << "Setup output \n");

        // Setup output
        DataFlowResult output;
        output.domainToIndex = domainToIndex;
        output.result = result;
        output.modified = modified;

        return output;
    }

    // EXPRESSION-RELATED UTILS

    // The Expression class is provided here to help
    // you work with the expressions we'll be concerned
    // about for the Available Expression analysis
    Expression::Expression (Instruction * I) {
        if (BinaryOperator * BO = dyn_cast<BinaryOperator>(I)) {
            this->v1 = BO->getOperand(0);
            this->v2 = BO->getOperand(1);
            this->op = BO->getOpcode();
        }
        else {
            errs() << "We're only considering BinaryOperators\n";
        }
    }

    // For two expressions to be equal, they must
    // have the same operation and operands.
    bool Expression::operator== (const Expression &e2) const {
        return this->v1 == e2.v1 && this->v2 == e2.v2 && this->op == e2.op;
    }

    // Less than is provided here in case you want
    // to use STL maps, which use less than for
    // equality checking by default
    bool Expression::operator< (const Expression &e2) const {
        if (this->v1 == e2.v1)
            if (this->v2 == e2.v2)
                if (this->op == e2.op)
                    return false;
                else
                    return this->op < e2.op;
            else
                return this->v2 < e2.v2;
        else
            return this->v1 < e2.v1;
    }

    // A pretty printer for Expression objects
    // Feel free to alter in any way you like
    std::string Expression::toString() const {
        std::string op = "?";
        switch (this->op) {
        case Instruction::Add:
        case Instruction::FAdd: op = "+"; break;
        case Instruction::Sub:
        case Instruction::FSub: op = "-"; break;
        case Instruction::Mul:
        case Instruction::FMul: op = "*"; break;
        case Instruction::UDiv:
        case Instruction::FDiv:
        case Instruction::SDiv: op = "/"; break;
        case Instruction::URem:
        case Instruction::FRem:
        case Instruction::SRem: op = "%"; break;
        case Instruction::Shl: op = "<<"; break;
        case Instruction::AShr:
        case Instruction::LShr: op = ">>"; break;
        case Instruction::And: op = "&"; break;
        case Instruction::Or: op = "|"; break;
        case Instruction::Xor: op = "xor"; break;
        default: op = "op"; break;
        }
        return getShortValueName(v1) + " " + op + " " + getShortValueName(v2);
    }

    // Silly code to print out a set of expressions in a nice
    // format
    void printSet(std::vector<Expression> * x) {
        bool first = true;
        outs() << "{";
        for (std::vector<Expression>::iterator it=x->begin(), iend=x->end(); it!=iend; ++it) {
            if (!first) {
                outs() << ", ";
            }
            else { first = false; }
            outs() << (it->toString());
        }
        outs() << "}\n";
    }

    // The following code may be useful for both of your passes:
    // If you recall, there is no "get the variable on the left
    // hand side" function in LLVM. Normally this is fine: we
    // just call getName(). This does not work, however, for
    // instructions that are publically 'unnamed', but internally
    // assigned a name like '%0' or '%1'. To get these names, the
    // following code does some really nasty stuff. It also grabs
    // raw integer values so our expressions can look a little
    // cleaner.

    // Feel free to improve this code if you want nicer looking
    // results. For example, ConstantInts are the only pretty
    // printed Constants.

    // Sadly, this code is a great example of not doing things
    // 'the llvm way', especially since we're using std::string.
    // I encourage you to think of a way to make this code nicer
    // and let me know :)
    // -Jonathan

    std::string getShortValueName(Value * v) {
        if (v->getName().str().length() > 0) {
            return "%" + v->getName().str();
        }
        else if (isa<Instruction>(v)) {
            std::string s = "";
            raw_string_ostream * strm = new raw_string_ostream(s);
            v->print(*strm);
            std::string inst = strm->str();
            size_t idx1 = inst.find("%");
            size_t idx2 = inst.find(" ",idx1);
            if (idx1 != std::string::npos && idx2 != std::string::npos) {
                return inst.substr(idx1,idx2-idx1);
            }
            else {
                return "\"" + inst + "\"";
            }
        }
        else if (ConstantInt * cint = dyn_cast<ConstantInt>(v)) {
            std::string s = "";
            raw_string_ostream * strm = new raw_string_ostream(s);
            cint->getValue().print(*strm,true);
            return strm->str();
        }
        else {
            std::string s = "";
            raw_string_ostream * strm = new raw_string_ostream(s);
            v->print(*strm);
            std::string inst = strm->str();
            return "\"" + inst + "\"";
        }
    }


    // Pretty printing utility functions

    void printBitVector(BitVector b)
    {
        unsigned int i;
        unsigned int b_size = b.size();

        if(b_size == 0)
            DBG(outs() << "-");
        else
        {
            for(i = 0; i < b.size() ; i++)
            {
                if(b[i] == true)
                    DBG(outs() << "1");
                else
                    DBG(outs() << "0");
            }
        }
        DBG(outs() << "\n");
    }

    void printResult(DataFlowResult output)
    {
        for(auto entry : output.result)
        {
            DBG(outs() << "BB " << entry.first->getName() << "\n");

            printBitVector(entry.second.in);
            printBitVector(entry.second.out);
        }
    }

    std::string printValue(Value* v)
    {
        std::string res; llvm::raw_string_ostream raw_st(res);
        v->print(raw_st);
        return res;
    }

    std::string getShortValueName(Value * v);

    std::string printSet(std::vector<void*> domain, BitVector on, int  mode) {
        std::stringstream ss;
        ss << "{";
        int ind = 0;

        for (int i = 0; i < domain.size(); i++) {
            // If element i is on
            if (on[i]) {
                if (ind > 0)
                    ss << ",";

                switch(mode)
                {
                case 0:
                    // Value*
                    ss << " " << getShortValueName((Value*)domain[i]);
                    break;

                case 1:
                    // Expression*
                    ss << " " << ((Expression*)domain[i])->toString();
                    break;


                default:
                    errs() << "Invalid mode :: " << mode << "\n";
                    break;
                }


                ind++;
            }
        }

        ss << " }";

        return ss.str();
    }


}
