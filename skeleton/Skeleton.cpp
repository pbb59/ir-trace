#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Transforms/Utils/ValueMapper.h"

// to load file with branch info give command line arg with name (might need multiple for different loops, so json file?)
// https://stackoverflow.com/questions/13626993/is-it-possible-to-add-arguments-for-user-defined-passes-in-llvm

using namespace llvm;

namespace {

  struct Trace {
    // keep track of the live-ins to this trace
    // used to generate function signature to actually run this thing
    std::vector<Type*> _signature;

    // A trace is a single basic block, store every instruction in this
    BasicBlock *_body;

    // The program/module the trace exists in (global variables are stored here)
    Module *_theModule;

    // Store the name of the trace to generate function name with
    std::string _traceName;

    // looks at prev instruction to know refs of current instruction
    //ValueToValueMapTy _vmap;

    // list of instructions, safe to delete in the current block
    std::vector<Instruction*> getInstPtrsInBlk(BasicBlock *blk) {
      std::vector<Instruction*> instPtrs;
      for (auto& I : *blk) {
          instPtrs.push_back(&I);
      }
      return instPtrs;
    }

    void mergeBlks(BasicBlock *traceBB, BasicBlock *otherBB) {
      auto instPtrs = getInstPtrsInBlk(otherBB);
      for (int i = 0; i < instPtrs.size(); i++) {
        Instruction *movedInst = instPtrs[i];
        movedInst->removeFromParent();
        traceBB->getInstList().push_back(movedInst);
      }
      otherBB->eraseFromParent();
    }

    void generate(BasicBlock *curBB, std::vector<bool> pathArray) {
      bool done;
      int brIdx = 0;

      do {
        // set done flag
        done = true;

        // get pointers to each instrcution, so when delete in curBB the iteration isn't messed up...
        std::vector<Instruction*> instPtrs = getInstPtrsInBlk(curBB);

        // get the last instruction (only one that can be a branch)
        Instruction *I = instPtrs[instPtrs.size() - 1];

        // check if conditional branch
        BranchInst *branchInst = dyn_cast<BranchInst>(I);
        if (branchInst != nullptr && branchInst->isConditional()) {
          // take a branch direction if conditional
          errs() << "found conditional " << *I << "\n";
          BasicBlock* t  = cast<BasicBlock>(branchInst->getOperand(2));
          BasicBlock* nt = cast<BasicBlock>(branchInst->getOperand(1));

          // remove the branch from the end of the block
          branchInst->eraseFromParent();

          bool tracedOutcome = pathArray[brIdx];
          // continue tracing on the path that was taken and delete the other
          if (tracedOutcome) {
            errs() << "erase not taken\n";
            nt->eraseFromParent();
            mergeBlks(curBB, t);
          }
          else {
            errs() << "erase taken\n";
            t->eraseFromParent(); 
            mergeBlks(curBB, nt);
          }

          // if this was a branch then still more work to do
          done = false;
          brIdx++;
        }
      
      } while (!done);
    } 
  };


  // TODO potentially want a LoopPass b/c want to trace across within a loop nest and not outside
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {

      // cheat in branch vector (TODO from file)
      std::vector<bool> branchOutcomes;
      for (int i = 0; i < 10; i++) {
        branchOutcomes.push_back(0);
      }

      // the current trace in llvm instructions (treat as a single basic block)
      Trace trace;

      // TODO just do on known function
      if (F.getName() != "test") return false;

      // get the first block of the function
      auto &bb = F.getEntryBlock();

      // generate a trace starting from a basic block
      trace.generate(&bb, branchOutcomes);

      // whether code was modified or not
      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
