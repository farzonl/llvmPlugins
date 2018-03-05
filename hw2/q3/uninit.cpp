/*
 *  
 *  
 * ______                        
 *|  ____|                       
 *| |__ __ _ _ __ _______  _ __  
 *|  __/ _` | '__|_  / _ \| '_ \
 *| | | (_| | |   / / (_) | | | |
 *|_|  \__,_|_|  /___\___/|_| |_|
 *
 *  Created by Farzon Lotfi.
 *  Copyright 2018 Georgia Tech. All rights reserved.
 *
 */

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"

#include <set>

using namespace llvm;

namespace
{
    //2.1 Average, maximum and minimum number of basic blocks inside functions.
    struct UninitializedVar : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        UninitializedVar() : FunctionPass(ID) {}
        virtual ~UninitializedVar() {}
        bool runOnFunction(Function &F) override
        {
            getUninitializedVarInfo(F);
            return false;
        }
        
        //An iterator over a Function gives us a list of basic blocks.
        void getUninitializedVarInfo(const Function& func) const
        {
            std::set<StringRef> stored;
            errs() << "function: " << func.getName() << "\n";
            for (Function::const_iterator iter = func.begin(); iter != func.end(); ++iter) {
                const BasicBlock &currBlock = *iter;
                const BasicBlock::InstListType* instList =  &currBlock.getInstList();

                for(BasicBlock::InstListType::const_iterator instrIter = instList->begin(); 
                instrIter != instList->end(); ++instrIter) {
                    const Instruction &currInst = *instrIter;
                    
                    if(isa<StoreInst>(currInst)) {
                        //errs() << "store instruction found: [ ";
                        //for (auto op = currInst.op_begin(); op != currInst.op_end(); op++) {
                        //    errs() << (*op)->getName() << " ";
                        //}
                        //errs() << "]\n";
                        auto op = currInst.getOperand(1);
                        stored.insert(op->getName());
                    }
                    
                    if(isa<LoadInst>(currInst)) {
                        auto op = currInst.getOperand(0);
                        bool bNotInit = (stored.find(op->getName()) == stored.end());
                        if(bNotInit) {
                            errs() << "unitialized variable: " << op->getName();
                            const DebugLoc &DL = currInst.getDebugLoc();
                            if(DL) {
                                errs() << "used on line: " << DL.getLine();
                            }
                            else
                            {
                                if(const MDNode *md = currInst.getMetadata("dbg")) {
                                    if(auto *subProgram = dyn_cast<DISubprogram>(md)) {
                                        errs() << "used on line: " << subProgram->getLine();
                                    }
                                }
                            }
                            errs() << " found.\n";
                        }
                        //errs() << "load instruction found " << (bNotInit ? "not" : "") << " initalized : [ ";
                        //for (auto op = currInst.op_begin(); op != currInst.op_end(); op++) {
                        //    errs() << (*op)->getName() << " ";
                        //}
                        //errs() << "]\n";
                    }
                }
            }
        }

        bool doFinalization(Module &M) override {
            return false;
        }

    };
}

char UninitializedVar::ID = 0;
static RegisterPass<UninitializedVar>
X("uninit", "counts number of unitialized variables");
