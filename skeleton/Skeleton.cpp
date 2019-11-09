#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

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

    // we need to create a function with a single basic block in it
    // making sure to maintain any global variables and stuff
    void initTrace(std::string traceName, Module *theModule) {
      _traceName = traceName;
      _theModule = theModule;

      // create basic block with nothing in it yet
      Function *F = nullptr;
      _body = BasicBlock::Create(_theModule->getContext(), "entry", F);

    }

    // return a trace with signature
    Function *getTraceFunction() {
      // https://gist.github.com/JacquesLucke/1bddc9aa24fe684d1b19d4bf51a5eb47
      // Make the function signature:  double(double,double) etc.
      int numArgs = 4;
      std::vector<Type*> Doubles(numArgs, Type::getDoubleTy(_theModule->getContext()));
      FunctionType *FT = FunctionType::get(Type::getDoubleTy(_theModule->getContext()), Doubles, false);

      Function *F = Function::Create(FT, Function::ExternalLinkage, _traceName, _theModule);

      return F;
    }
  };

  // TODO potentially want a LoopPass b/c want to trace across within a loop nest and not outside
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {

      // cheat in branch vector (TODO from file)
      bool *pathArray = new bool[1];

      // the current trace in llvm instructions (treat as a single basic block)


      errs() << "I saw a function called " << F.getName() << "!\n";
      for (auto &B : F) {
        for (auto &I : B) {
          /*if (auto *op = dyn_cast<BinaryOperator>(&I)) {
            errs() << "I saw an op called " << op->getName() << "!\n";
          }*/
          errs() << "Instruction " << I.getOpcodeName();
        }
      }

      delete[] pathArray;

      // whether code was modified or not
      return false;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
