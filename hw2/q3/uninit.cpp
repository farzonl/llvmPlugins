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

        }

        bool doFinalization(Module &M) override {
            return false;
        }

    };
}

char UninitializedVar::ID = 0;
static RegisterPass<UninitializedVar>
X("uninit", "counts number of unitialized variables");