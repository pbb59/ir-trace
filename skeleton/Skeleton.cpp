#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Transforms/Utils/Cloning.h"

#include <stdio.h>

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

    // we need to generate a fallback routine to call when fails
    // TODO don't know how to do this without making it a branch instruction
    void initTrace() {

    }

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

        // loop through all instructions (could prob make this more efficient by remembering we're we left off)
        for (int i = 0; i < instPtrs.size(); i++) {

          // get the current instruction that we're looking at
          Instruction *I = instPtrs[i];

          // check if branch (if return then we should finish the algorithm... for now)
          //BranchInst *branchInst = dyn_cast<BranchInst>(I);
          //if (branchInst != nullptr) {
          if (BranchInst *branchInst = dyn_cast<BranchInst>(I)) {
            if (branchInst->isConditional()) {
              // take a branch direction if conditional
              BasicBlock* t  = cast<BasicBlock>(branchInst->getOperand(2));
              BasicBlock* nt = cast<BasicBlock>(branchInst->getOperand(1));

              bool tracedOutcome = pathArray[brIdx];
              // continue tracing on the path that was taken and delete the other
              if (tracedOutcome) {
                //errs() << "erase not taken\n";
                //nt->eraseFromParent();
                mergeBlks(curBB, t);
              }
              else {
                //errs() << "erase taken\n";
                //t->eraseFromParent(); 
                mergeBlks(curBB, nt);
              }

              brIdx++;
            }
            // unconditional, note can't jump outside of function, so not really inlining
            // TODO what if multiple basic blocks go to this label... don't want to fully delete?
            // BUT is this a problem if it's a single forward path?
            else {
              BasicBlock* t  = cast<BasicBlock>(branchInst->getOperand(0));
              mergeBlks(curBB, t);
            }

            // remove the branch from the end of the block
            branchInst->eraseFromParent();

            // if this was a branch then still more work to do
            done = false;
            
          }
          // inline function calls along the path
          else if (CallInst *callInst = dyn_cast<CallInst>(I)) {
            // only inline if it's an llvm intrinsic (inserted when you inline the original function, so ignore)
            // These intrinsics are deleted when compile to machine code (not real function calls)
            // verified with objdump -dC
            Function *fun = callInst->getCalledFunction();
            //if (fun->getName().str().compare(0, 5, "llvm.", 0, 5) == 0) {
            if (fun->isDeclaration()) {
              continue;
            }

            InlineFunctionInfo ifi;
            InlineFunction(callInst, ifi);
            done = false;
            // need to break here b/c potentially more instructions to handle afterwards
            break; 

          }
          // remove phi nodes and update refs
          else if (PHINode *phiInst = dyn_cast<PHINode>(I)) {
            errs() << "phi " << *I << "\n";
            // get refs from phi node, only take the ref that is in the current basic block
            // if multiple in current basic block take the most recently assigned
            unsigned numOps = phiInst->getNumOperands();
            Value *lastDef = nullptr;
            for (int j = 0; j < instPtrs.size(); j++) {
              for (int k = 0; k < numOps; k++) {
                Value* depInst = phiInst->getOperand(k);
                if (depInst == cast<Value>(instPtrs[j])) {
                  lastDef = depInst;
                }
              }
            }

            // update everyone who uses this instruction
            for (auto& U : phiInst->uses()) {
              User* user = U.getUser();
              if (cast<User>(phiInst) == user) continue; // don't try to mod self b/c going to delete
              user->setOperand(U.getOperandNo(), lastDef);
            }
            phiInst->eraseFromParent();
          }

        }
      } while (!done);
    } 

    // we needed to keep every not taken block in case was needed later
    // now that the full path has been generated delete blocks with no predessors
    void elimStranded(Function &F) {
      // get list of bbs because we're going to delete
      // everytime we delete one, we need to check again b/c maybe a successor also free now
      bool moreToElim;
      do {
        moreToElim = false;

        std::vector<BasicBlock*> bbs;
        for (auto &B : F) {
          bbs.push_back(&B);
        }

        for (int i = 0; i < bbs.size(); i++) {
          BasicBlock *bb = bbs[i];
          if (bb->hasNPredecessors(0) && (&F.getEntryBlock() != bb)) {
            //errs() << "elimate blk\n" << "\n"; 
            bb->eraseFromParent();
            moreToElim = true;
          }
        }
      } while(moreToElim);
    }

  };

  // TODO potentially want a LoopPass b/c want to trace across within a loop nest and not outside
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {

      // TODO just do on known function
      if (F.getName() == "main" || F.getName() == "print_path" || F.getName() == "minver_fabs") return false;

      // load profile info
      std::vector<bool> branchOutcomes;

      char buffer[100];
      FILE *fp;
      fp = fopen("trace.txt", "r");
      if (fp) {
        if (fgets(buffer, sizeof(buffer), fp)) {
          char *token = strtok(buffer, ",");
          while (token != NULL) {
            bool branch = (bool)atoi(token);
            branchOutcomes.push_back(branch);
            token = strtok(0, ",");
          }
        }
        fclose(fp);
      }
      
      // the current trace in llvm instructions (treat as a single basic block)
      Trace trace;

      // get the first block of the function
      auto &bb = F.getEntryBlock();

      // generate a trace starting from a basic block
      trace.generate(&bb, branchOutcomes);

      // delete any blocks that aren't being pointed to
      trace.elimStranded(F);

      // whether code was modified or not
      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
