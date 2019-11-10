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
    ValueToValueMapTy _vmap;

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

    BasicBlock *getTracedBlock() {
      return _body;
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

    void generate(BasicBlock *curBB, bool *pathArray, int brIdx) {
      bool done;

      do {
        // set done flag
        done = true;

        // get pointers to each instrcution, so when delete in curBB the iteration isn't messed up...
        std::vector<Instruction*> instPtrs = getInstPtrsInBlk(curBB);

        for (int i = 0; i < instPtrs.size(); i++) {
          Instruction *I = instPtrs[i];
          // if branch then go to next basic block and delete this one
          
          if (I == nullptr) continue;
          errs() << *I << "\n";

          BranchInst *branchInst = dyn_cast<BranchInst>(I);
          if (branchInst != nullptr && branchInst->isConditional()) {
            // take a branch direction if conditional
            //if (branchInst->isConditional()) {
              errs() << "found conditional " << *I << "\n";
              BasicBlock* t  = cast<BasicBlock>(branchInst->getOperand(2));
              BasicBlock* nt = cast<BasicBlock>(branchInst->getOperand(1));
              
              // remove the branch from this first block
              // important to do this BEFORE remove the blocks, b/c will try to update refs?
              /*if (traceBB == curBB) {
                errs() << "erase " << *branchInst << "\n";
                branchInst->eraseFromParent();
                //errs() << *curBB << "\n";
              }*/

              // remove the branch from the end of the block
              branchInst->eraseFromParent();

              // go in first basic block
              // delete not taken, pull all instructions into trace block, set dirty
              // if dirty repeat

              bool tracedOutcome = pathArray[brIdx];
              // continue tracing on the path that was taken
              if (tracedOutcome) {
                errs() << "erase not taken\n";
                nt->eraseFromParent();
                // add all instructions from the taken block and repeat
                mergeBlks(curBB, t);
                

                //generate(traceBB, t, pathArray, brIdx + 1);
              }
              else {
                errs() << "erase taken\n";
                t->eraseFromParent();
                
                mergeBlks(curBB, nt);
                //generate(traceBB, nt, pathArray, brIdx + 1);
              }

              // if this was a branch then still more work to do
              done = false;

              /*if (curBB != traceBB) {
                errs() << "erase self\n";
                curBB->eraseFromParent();
              }*/
              /*// remove the branch from this first block
              else {
                errs() << "erase " << *branchInst << "\n";
                branchInst->eraseFromParent();
                //errs() << *curBB << "\n";
              }*/

            //}
          }
          // normal tracing on blocks other than the entry
          /*else {
            
            if (curBB != traceBB) {
              errs() << *I << " 1\n";
              I->removeFromParent(); // shouldn't modify while iterating!!!
              errs() << *I << " 2\n";
              traceBB->getInstList().push_back(I);
              errs() << *I << " 3\n";
            }
            //I.eraseFromParent();
            //traceBB->getInstList().push_back(&I);
            //Instruction *copiedInst = I.clone();

            // need to restich instructions together (otherwise get badref)
            //_vmap[&I] = copiedInst;
            //RemapInstruction(copiedInst, _vmap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);

            //errs() << *copiedInst << "\n";
            // TODO should do vmap stuff here?
            //append(&I);
          }*/
          //errs() << "complete inst" << "\n";
        }
        errs() << "complete block\n";
        //errs() << *curBB << "\n";
      } while (!done);
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
      //Constant *c = _theModule->getOrInsertFunction(_traceName, FT);
      //Function *F = cast<Function>(c);

      Function *F = Function::Create(FT, Function::ExternalLinkage, _traceName, _theModule);

      // insert the block into the function
      _body->insertInto(F);



      return F;   
    }
  };


  // TODO potentially want a LoopPass b/c want to trace across within a loop nest and not outside
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {

      // cheat in branch vector (TODO from file)
      bool *pathArray = new bool[ 10 ] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      // the current trace in llvm instructions (treat as a single basic block)
      Trace trace;
      trace.initTrace("trace", F.getParent());

      // TODO just do on known function
      if (F.getName() != "test") return false;

      // get the first block of the function
      auto &bb = F.getEntryBlock();

      // roll through basic blocks adding all instruction to original basic block, when finish a basic block call eraseFromParent() to delete
      // at the end do a dead code elimination to get rid of basic blocks that weren't taken
      trace.generate(&bb, pathArray, 0);

      errs() << "finish gen\n";     

      //verifyFunction(F, &outs());

      // put the generated block into the function
      //BasicBlock *tracedBlk = trace.getTracedBlock();
      //errs() << "trace\n" << tracedBlk << "\nendtrace\n";
      //tracedBlk->insertInto(&F);


      // collect instructions on the path (need full vmap before creating new instructions)
      

      // This works as long as remove the original use of those registers
      // Remap instruction on the path and place into trace
      // https://stackoverflow.com/questions/43303943/in-llvm-ir-i-want-to-copy-a-set-of-instructions-and-paste-those-instructions-to
      /*ValueToValueMapTy vmap; // looks at prev instruction to know refs of current instruction
      BasicBlock *newBB = BasicBlock::Create(F.getParent()->getContext(), "entry", nullptr); 
      vmap[&bb] = newBB;
      for (auto &I : bb) {
        errs() << "Original: " << I << "\n";
        // add a copy of the instruction to the current trace
        Instruction *newInst = I.clone();
        newBB->getInstList().push_back(newInst);
        vmap[&I] = newInst;
        RemapInstruction(newInst, vmap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);

        errs() << "Copy: " << *newInst << "\n";

        // the 'name' is the SSA dest reg (needs to be unique)
        //if (I.hasName())
        //  newInst->setName(I.getName() + "__t");
        //trace.append(newInst);
      }*/

       errs() << F << "\n";

      // need to delete this block (or really the entire function// ?)

      // get the traced function
      //auto traceF = trace.createTraceFunction();

      /*for (auto &B : *traceF) {
        for (auto &I : B) {
          errs() << "Instruction " << I << "\n";
        }
      }*/

      delete[] pathArray;

      // whether code was modified or not
      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
