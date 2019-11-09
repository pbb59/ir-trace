#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/DerivedTypes.h"
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
      Function *F = nullptr; // put into a function later
      _body = BasicBlock::Create(_theModule->getContext(), "entry", F);

    }

    void append(Instruction *tracedInst) {
      // TODO if the instruction is a branch, need to replace with something else?

      _body->getInstList().push_back(tracedInst);
    }

    // return a trace with signature
    Function *createTraceFunction() {
      auto& context = _theModule->getContext();

      // TODO this should happen somewhere else, or should happen at the beginning so not repeated
      // add a ret statement at the end of the basic block to signal the end of the trace
      IRBuilder<> builder(_body);
      builder.CreateRetVoid();

      for (auto& I : *_body) {
        errs() << "Instruction " << I << "\n";
      }

      // https://gist.github.com/JacquesLucke/1bddc9aa24fe684d1b19d4bf51a5eb47
      // Make the function signature:  double(double,double) etc.
      std::vector<Type*> sigArgs;
      sigArgs.push_back(Type::getInt32Ty(context));
      sigArgs.push_back(Type::getInt32Ty(context));
      // <ret types>, <arg types> 
      FunctionType *FT = FunctionType::get(Type::getVoidTy(context), sigArgs, false);

      // insert function into module with no callback (nullptr at the end)
      FunctionCallee *c = _theModule->getOrInsertFunction(_traceName, FT);
      Function *F = cast<Function>(c);

      // insert the block into the function
      _body->insertInto(F);

      // this doesn't work
      //Function *F = Function::Create(FT, Function::ExternalLinkage, _traceName, _theModule);

      return F;   
    }
  };

  // TODO potentially want a LoopPass b/c want to trace across within a loop nest and not outside
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {

      // cheat in branch vector (TODO from file)
      bool *pathArray = new bool[ 1 ] { 0 };

      // the current trace in llvm instructions (treat as a single basic block)
      Trace trace;
      trace.initTrace("trace", F.getParent());

      // TODO just do on known function
      if (F.getName() != "test") return false;

      // get the first block of the function
      auto &bb = F.getEntryBlock();

      // run through instructions along the path
      for (auto &I : bb) {
        errs() << "Instruction " << I.getOpcodeName() << "\n";

        // add a copy of the instruction to the current trace
        trace.append(I.clone());
      }

      // get the traced function
      auto traceF = trace.createTraceFunction();

      /*for (auto &B : *traceF) {
        for (auto &I : B) {
          errs() << "Instruction " << I << "\n";
        }
      }*/

      delete[] pathArray;

      // whether code was modified or not
      return false;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
