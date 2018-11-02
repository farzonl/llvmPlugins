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
#include <stack>
#include  <utility> //std::pair
#include <algorithm>    // std::set_difference

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
                                errs() << " used on line: " << DL.getLine();
                            }
                            else
                            {
                                if(const MDNode *md = currInst.getMetadata("dbg")) {
                                    if(auto *subProgram = dyn_cast<DISubprogram>(md)) {
                                        errs() << " used on line: " << subProgram->getLine();
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
X("nuninit", "naive counts number of unitialized variables");

namespace
{
    //3.4 this function returns true if there exists a directed path from basic block A to B, false otherwise.
    struct ReachablePass  : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        
        ReachablePass() :  FunctionPass(ID) {}
        virtual ~ReachablePass() {}
        
        bool runOnFunction(Function &F) override
        {
            reachable(F);
            return false;
        }
        
        void printList(std::vector<const BasicBlock *> &list)
        {
            if(list.empty())
            {
                return;
            }
            
            errs() << "[";
            for (auto v = list.begin(); v != list.end(); ++v)
            {
                //errs() << "(" << (*v) <<": ";
                (*v)->printAsOperand(errs(), false);
                //errs() << "), ";
                errs() << " ";
            }
            errs() << "]\n";
        }
        
        void reachable(Function &func)
        {
            errs() << "Start reachable analysis on "<< func.getName() << ":\n";
            int nReachable = 0;
            std::vector<const BasicBlock*> longestPath;
            
            const BasicBlock *lastBlock = nullptr;
            for (Function::const_iterator i_iter = func.begin(); i_iter != func.end(); ++i_iter)
            {
                if(std::next(i_iter) == func.end()) {
                    lastBlock = &*i_iter;
                }
            }

            /*std::set<StringRef> InSet;
            std::set<StringRef> GenSet;
            std::set<StringRef> KillSet;
            std::set<StringRef> OutSet;*/
            std::set<StringRef> stored;
            for (Function::const_iterator i_iter = func.begin(); i_iter != func.end(); ++i_iter)
            {
                const BasicBlock *A_Block = &*i_iter;
                std::set<StringRef> loadedWOStore;
                
                //need to perform a graph search from i to j
                std::vector<const BasicBlock*> path = dfs(A_Block, lastBlock, nReachable);
                
                if(nReachable)
                {
                    for(size_t i = 0; i < path.size(); i++) {
                        
                        const BasicBlock &currBlock = *path[i];
                        const BasicBlock::InstListType* instList =  &currBlock.getInstList();
                        for(BasicBlock::InstListType::const_iterator instrIter = instList->begin();
                            instrIter != instList->end(); ++instrIter) {
                            
                            const Instruction &currInst = *instrIter;
                            
                            if(isa<StoreInst>(currInst)) {
                                auto op = currInst.getOperand(1);
                                stored.insert(op->getName());
                            }
                            if(isa<LoadInst>(currInst)) {
                                auto op = currInst.getOperand(0);
                                bool bNotInit = (stored.find(op->getName()) == stored.end());
                                if(bNotInit) {
                                    loadedWOStore.insert(op->getName());
                                }
                            }
                        }
                            
                        }
                        if(loadedWOStore.size() > 0) {
                            for (auto i : loadedWOStore) {
                                errs() << "unitialized variable: " << i << "\n";
                            }
                        }
                    }
            }
        }
    
        /*
         https://en.wikipedia.org/wiki/Depth-first_search#Pseudocode
         1  procedure DFS-iterative(G,v):
         2      let S be a stack
         3      S.push(v)
         4      while S is not empty
         5          v = S.pop()
         6          if v is not labeled as discovered:
         7              label v as discovered
         8              for all edges from v to w in G.adjacentEdges(v) do
         9                  S.push(w)
         */
        
        std::vector<const BasicBlock*> dfs(const BasicBlock* A, const BasicBlock* B, int &reachable)
        {
            std::stack<std::pair<const BasicBlock*,std::vector<const BasicBlock*>>> s;
            std::set<const BasicBlock*> visited;
            s.push(std::make_pair(A,std::vector<const BasicBlock*>({A})));
            
            while(!s.empty())
            {
                const BasicBlock* v = s.top().first;
                std::vector<const BasicBlock*> path = s.top().second;
                s.pop();
                if(visited.find(v) == visited.end())
                {
                    visited.insert(v);
                    const TerminatorInst *termInst = v->getTerminator();
                    int numEdges = termInst->getNumSuccessors();
                    for(int i = 0; i < numEdges; i++)
                    {
                        BasicBlock *w = termInst->getSuccessor(i);
                        path.push_back(w);
                        
                        if(w == B)
                        {
                            reachable++;
                            return path;
                        }
                        std::vector<const BasicBlock*> cpyPath(path);
                        cpyPath.push_back(w);
                        s.push(std::make_pair(w,cpyPath));
                    }
                }
            }
            return std::vector<const BasicBlock*>();
        }
        
        bool doFinalization(Module &M) override {
            return false;
        }
    };
}

char ReachablePass::ID = 0;
static RegisterPass<ReachablePass>
Y("runinit", "reachable uninit");

