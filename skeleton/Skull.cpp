#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
//#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace {
  struct SkullPass : public FunctionPass {
    static char ID;
    SkullPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << "I saw a function called " << F.getName() << "!\n";
      
      // Get the function to call from our runtime library.
      LLVMContext &Ctx = F.getContext();
      std::vector<Type*> paramTypes = {Type::getInt1Ty(Ctx)};
      Type *retType = Type::getVoidTy(Ctx);
      FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
      auto funccallee = F.getParent()->getOrInsertFunction("print_path", logFuncType);
      Value* logFunc = funccallee.getCallee();

      for (auto &B : F) {
          errs() << "Inside a Block! \n";
          for (auto &I : B) {
              errs() << "Instruction! " << I << " \n";
              BranchInst *bin = dyn_cast<BranchInst>(&I);
              if (bin) {
                  errs() << "and it's a branch instruction! \n";
                  bool branch = bin->isConditional();
                  errs() << branch << "\n";
                  if (branch) {
                      Value *condition = bin->getCondition();
                      errs() << *condition << "\n";
                      Type *cond_type = condition->getType();
                      errs() << *cond_type << "\n";

                      // Cast to instruction
                      if (isa<Instruction>(condition)){
                          Instruction *op = cast<Instruction>(condition);
                  
                          // Insert *after* `op`.
                          IRBuilder<> builder(op);
                          builder.SetInsertPoint(&B, ++builder.GetInsertPoint());

                          // Insert a call to our function.
                          Value* args[] = {op};
                          builder.CreateCall(logFunc, args);
                      }
                  } 
              }
          }
      }

      return false;
    }
  };
}

char SkullPass::ID = 0;

// Register the pass so `opt -skull` runs it.
static RegisterPass<SkullPass> X("skull", "a useless pass");

