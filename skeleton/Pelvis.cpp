#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

namespace {
  struct PelvisPass : public FunctionPass {
    static char ID;
    PelvisPass() : FunctionPass(ID) {}
    
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      AU.addRequired<LoopInfoWrapperPass>();
    }

    virtual bool runOnFunction(Function &F) {
      errs() << "I saw a function called " << F.getName() << "!\n";
      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
      for (auto &IT: LI) {
          errs() << "Inside iteration \n";
          errs() << *IT << "\n";
      } 
      for (auto &B: F) {
          errs() << "Inside block \n";
          bool isLoop = LI.getLoopFor(&B);
          if(isLoop) {
            errs() << "loop \n";
            errs() << B << "\n";
            for (auto &A: B) {
                errs() << "What is this? \n";
                errs() << A << "\n";
                for (auto &C: A) {
                    errs() << "What is this? \n";
                    errs() << C << "\n";
                }
            }
          }
      } 
      return(false);
    }
  };
}

char PelvisPass::ID = 0;

// Register the pass so `opt -pelvis` runs it.
static RegisterPass<PelvisPass> X("pelvis", "a useless pass");

