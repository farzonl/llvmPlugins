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

//#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <nlohmann/json.hpp>
#include<valarray>
#include <fstream>

using json = nlohmann::json;
using namespace llvm;


//STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace
{
    //1. Average, maximum and minimum number of basic blocks inside functions.
    struct BasicBlockFuncCounter : public FunctionPass
    {
        static std::vector<int> vecBasicBlockCount;
        static std::vector<std::string> vecBasicBlockFuncName;
        static char ID; // Pass identification, replacement for typeid
        BasicBlockFuncCounter() : FunctionPass(ID) {}
        virtual ~BasicBlockFuncCounter() {}
        bool runOnFunction(Function &F) override
        {
            //errs().write_escaped(F.getName()) << '\n';
            getBasicBlockInfo(F);
            return false;
        }
        
        //An iterator over a Function gives us a list of basic blocks.
        void getBasicBlockInfo(const Function& func) const
        {
            //int basicBlockCount = 0;
            /*const Function::BasicBlockListType &blocks =func.getBasicBlockList();
            for (Function::BasicBlockListType::const_iterator bIter = blocks.begin(); bIter != blocks.end(); ++bIter)
            {
                basicBlockCount++;
            }*/
            /*for (Function::const_iterator fIter = func.begin(); fIter != func.end(); ++fIter)
            {
                basicBlockCount++;
            }*/
            vecBasicBlockCount.push_back(func.size());
            vecBasicBlockFuncName.push_back(func.getName());
            //errs() << "# of basic blocks: " << basicBlockCount << '\n';
            //errs() << "# of basic blocks: " << func.size() << '\n';
        }

        bool doFinalization(Module &M) override {
            std::valarray<int> seq {vecBasicBlockCount.data(), vecBasicBlockCount.size()};
            double average = seq.sum()/static_cast<double>(vecBasicBlockCount.size());
            int iMax = seq.max();
            int iMin = seq.min();
            std::string sMaxFuncName;
            std::string sMinFuncName;
            for (size_t i = 0; i < vecBasicBlockCount.size(); i++)
            {
                if(iMax == vecBasicBlockCount[i])
                {
                    sMaxFuncName = vecBasicBlockFuncName[i];
                }
                if(iMin == vecBasicBlockCount[i])
                {
                    sMinFuncName = vecBasicBlockFuncName[i];
                }
            }
            json j;
            j["Minimum"] = { {"functionName", sMinFuncName}, {"blockCount", iMin} };
            j["Maximum"] = { {"functionName", sMaxFuncName}, {"blockCount", iMax} };
            j["Average"] = average;
            
            errs() << j.dump();
            std::ofstream o("testResults/BasicBlockCount.json");
            o << std::setw(4) << j << std::endl;
            return false;
        }

    };
}

char BasicBlockFuncCounter::ID = 0;
std::vector<int> BasicBlockFuncCounter::vecBasicBlockCount;
std::vector<std::string> BasicBlockFuncCounter::vecBasicBlockFuncName;
static RegisterPass<BasicBlockFuncCounter>
X("basicblock", "basic block function counter pass");

namespace
{
    //1. Average, maximum and minimum number of basic blocks inside functions.
    struct CFGEdgeCounter : public FunctionPass
    {
        static std::vector<int> veccCFGEdgeCount;
        static std::vector<std::string> vecCFGEdgeFuncName;
        static char ID; // Pass identification, replacement for typeid
        CFGEdgeCounter() : FunctionPass(ID) {}
        virtual ~CFGEdgeCounter() {}
        bool runOnFunction(Function &F) override
        {
            getCFGEdgeInfo(F);
            return false;
        }
        
        //An iterator over a Function gives us a list of basic blocks.
        void getCFGEdgeInfo(const Function& func) const
        {
            int numEdges = 0;
            const Function::BasicBlockListType &blocks =func.getBasicBlockList();
            for (Function::BasicBlockListType::const_iterator bIter = blocks.begin(); bIter != blocks.end(); ++bIter)
            {
                const BasicBlock &block = *bIter;
                const TerminatorInst *TInst = block.getTerminator();
                // count the jumps
                numEdges += TInst->getNumSuccessors();
            }
            veccCFGEdgeCount.push_back(numEdges);
            vecCFGEdgeFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            std::valarray<int> seq {veccCFGEdgeCount.data(), veccCFGEdgeCount.size()};
            double average = seq.sum()/static_cast<double>(veccCFGEdgeCount.size());
            int iMax = seq.max();
            int iMin = seq.min();
            std::string sMaxFuncName;
            std::string sMinFuncName;
            for (size_t i = 0; i < veccCFGEdgeCount.size(); i++)
            {
                if(iMax == veccCFGEdgeCount[i])
                {
                    sMaxFuncName = vecCFGEdgeFuncName[i];
                }
                if(iMin == veccCFGEdgeCount[i])
                {
                    sMinFuncName = vecCFGEdgeFuncName[i];
                }
            }
            json j;
            j["Minimum"] = { {"functionName", sMinFuncName}, {"blockCount", iMin} };
            j["Maximum"] = { {"functionName", sMaxFuncName}, {"blockCount", iMax} };
            j["Average"] = average;
            
            errs() << j.dump();
            std::ofstream o("testResults/CFGEdgeCount.json");
            o << std::setw(4) << j << std::endl;
            return false;
        }
    };
}

std::vector<int> CFGEdgeCounter::veccCFGEdgeCount;
std::vector<std::string> CFGEdgeCounter::vecCFGEdgeFuncName;
char CFGEdgeCounter::ID = 0;
static RegisterPass<CFGEdgeCounter>
Y("cfgedge", "cfg edge function counter pass");

namespace
{
  // 2. Average, maximum and minimum number of CFG edges inside functions.
  struct BackEdgeDetector : public /*LoopInfoWrapperPass*/ FunctionPass
  {
    static char ID; // Pass identification, replacement for typeid
    BackEdgeDetector() : /*LoopInfoWrapperPass()*/ FunctionPass(ID) {}
    virtual ~BackEdgeDetector() {}
    bool runOnFunction(Function &F) override
    {
        errs().write_escaped(F.getName()) << '\n';
        getBackEdgeInfo(F);
        return false;
    }
      
    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
        AU.setPreservesCFG();
        AU.addRequired<LoopInfoWrapperPass>();
    }
    
    void getBackEdgeInfo(const Function& func) const
    {
        LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        int backEdgesCount = 0;
        int currTreeDepth = 0;
        int prevTreeDepth = 0;
        int loopCounter = 0;
        for (LoopInfo::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
        {
            
            Loop *loop =*iter;
            loopCounter++;
            int loopBlockCounter = 0;

            for(Loop::block_iterator bIter = loop->block_begin(); bIter != loop->block_end(); ++bIter)
            {
                loopBlockCounter++;
            }
            errs() << "# of blocks of loops: " << loopBlockCounter << '\n';
          currTreeDepth = (*iter)->getLoopDepth();
          errs() << "current tree depth: " << currTreeDepth << '\n';
          //Back edges point from a node to one of its ancestors in the DFS tree.
          // therefore the prevDepth should be larger than the current depth
          if (prevTreeDepth > currTreeDepth)
          {
            backEdgesCount++;
          }

          prevTreeDepth = currTreeDepth;
        }
        errs() << "# of loops: " << loopCounter << '\n';
        errs() << "Total back edges: " << backEdgesCount << '\n';
      }
  };
}

char BackEdgeDetector::ID = 0;
static RegisterPass<BackEdgeDetector>
Z("backedge", "back Edge detector pass");
