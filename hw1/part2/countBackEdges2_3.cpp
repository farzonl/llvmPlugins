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

#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <nlohmann/json.hpp>
#include<valarray>
#include <fstream>

using json = nlohmann::json;
using namespace llvm;

#define TEST true

namespace
{
    class HelperFunctions
    {
    public:
        static json createAndWriteJson(const std::vector<int> & vecCount,
                                  const std::vector<std::string> & vecFuncName,
                                  const std::string &countName,
                                  bool turnOnSummation = false,
                                  bool turnOnMin = true,
                                  bool turnOnAvg = true)
        {
            std::valarray<int> seq {vecCount.data(), vecCount.size()};
            double average = seq.sum()/static_cast<double>(vecCount.size());
            int iMax = seq.max();
            int iMin = seq.min();
            std::string sMaxFuncName;
            std::string sMinFuncName;
            for (size_t i = 0; i < vecCount.size(); i++)
            {
                if(iMax == vecCount[i])
                {
                    sMaxFuncName = vecFuncName[i];
                }
                if(iMin == vecCount[i])
                {
                    sMinFuncName = vecFuncName[i];
                }
            }
            json j;
            
            if(turnOnMin)
            {
                j["Minimum"] = { {"functionName", sMinFuncName}, {countName, iMin} };
            }
            
            j["Maximum"] = { {"functionName", sMaxFuncName}, {countName, iMax} };
            
            if(turnOnAvg)
            {
                j["Average"] = average;
            }
            
            if(turnOnSummation)
            {
                j["Summation"] = seq.sum();
            }
                

#if TEST
            json jTest;
            for (size_t i = 0; i < vecCount.size(); i++)
            {
                jTest[i] = { {"functionName", vecFuncName[i]}, {countName, vecCount[i]} };
            }
            j["Test"] = jTest;
#endif

            std::ofstream o("testResults/"+countName+".json");
            o << std::setw(4) << j << std::endl;
#if TEST
            j.erase("Test");
#endif
            return j;
        }
    private:
        HelperFunctions() = delete;
    };
}

namespace
{
    //2.1 Average, maximum and minimum number of basic blocks inside functions.
    struct BasicBlockFuncCounter : public FunctionPass
    {
        static std::vector<int> vecBasicBlockCount;
        static std::vector<std::string> vecBasicBlockFuncName;
        static char ID; // Pass identification, replacement for typeid
        BasicBlockFuncCounter() : FunctionPass(ID) {}
        virtual ~BasicBlockFuncCounter() {}
        bool runOnFunction(Function &F) override
        {
            getBasicBlockInfo(F);
            return false;
        }
        
        //An iterator over a Function gives us a list of basic blocks.
        void getBasicBlockInfo(const Function& func) const
        {
            vecBasicBlockCount.push_back(func.size());
            vecBasicBlockFuncName.push_back(func.getName());
        }

        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecBasicBlockCount, vecBasicBlockFuncName, "BasicBlockCount");
            errs() << j.dump();
            return false;
        }

    };
}

char BasicBlockFuncCounter::ID = 0;
std::vector<int> BasicBlockFuncCounter::vecBasicBlockCount;
std::vector<std::string> BasicBlockFuncCounter::vecBasicBlockFuncName;
static RegisterPass<BasicBlockFuncCounter>
X("basicblock", "basic block function counter pass.");

namespace
{
    // 2.2 Average, maximum and minimum number of CFG edges inside functions.
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
            
            json j = HelperFunctions::createAndWriteJson(veccCFGEdgeCount, vecCFGEdgeFuncName, "CFGEdgeCount");
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> CFGEdgeCounter::veccCFGEdgeCount;
std::vector<std::string> CFGEdgeCounter::vecCFGEdgeFuncName;
char CFGEdgeCounter::ID = 0;
static RegisterPass<CFGEdgeCounter>
Y("cfgedge", "cfg edge function counter pass.");

namespace
{
  //2.3 Average, maximum and minimum number of single entry loops inside functions (count each loop based on a back edge).
  struct BackEdgeDetector : public FunctionPass
  {
    static std::vector<int> vecBackEdgeCount;
    static std::vector<std::string> vecBackEdgeFuncName;
    static char ID; // Pass identification, replacement for typeid
    BackEdgeDetector() : FunctionPass(ID) {}
    virtual ~BackEdgeDetector() {}
    bool runOnFunction(Function &F) override
    {
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
        for (Loop::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
        {
            backEdgesCount+=(*iter)->getNumBackEdges();
            
        }
        vecBackEdgeCount.push_back(backEdgesCount);
        vecBackEdgeFuncName.push_back(func.getName());
    }
    
    bool doFinalization(Module &M) override {
        json j = HelperFunctions::createAndWriteJson(vecBackEdgeCount, vecBackEdgeFuncName, "BackEdgeCount");
        errs() << j.dump();
        return false;
    }
  };
}

std::vector<int> BackEdgeDetector::vecBackEdgeCount;
std::vector<std::string> BackEdgeDetector::vecBackEdgeFuncName;
char BackEdgeDetector::ID = 0;
static RegisterPass<BackEdgeDetector>
Z("backedge", "back Edge detector pass.");


namespace
{
    //2.4 Average, maximum and minimum number of loop basic blocks inside functions
    struct LoopBasicBlockDetector : public FunctionPass
    {
        static std::vector<int> vecLoopBasicBlockCount;
        static std::vector<std::string> vecLoopBasicBlockFuncName;
        static char ID; // Pass identification, replacement for typeid
        LoopBasicBlockDetector() :  FunctionPass(ID) {}
        virtual ~LoopBasicBlockDetector() {}
        
        bool runOnFunction(Function &F) override
        {
            getLoopBasicBlocInfo(F);
            return false;
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        
        void getLoopBasicBlocInfo(const Function& func) const
        {
            LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            int loopBlockCounter = 0;
            for (Loop::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
            {
                Loop *loop =*iter;
                
                for(Loop::block_iterator bIter = loop->block_begin(); bIter != loop->block_end(); ++bIter)
                {
                    loopBlockCounter++;
                }
            }
            vecLoopBasicBlockCount.push_back(loopBlockCounter);
            vecLoopBasicBlockFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecLoopBasicBlockCount, vecLoopBasicBlockFuncName, "LoopBasicBlockCount");
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> LoopBasicBlockDetector::vecLoopBasicBlockCount;
std::vector<std::string> LoopBasicBlockDetector::vecLoopBasicBlockFuncName;
char LoopBasicBlockDetector::ID = 0;
static RegisterPass<LoopBasicBlockDetector>
A("loopbasicblock", "loop basic block counter pass.");

namespace
{
    // 2.5 Average number of dominators for a basic block across all functions.
    struct DominatorsPass : public FunctionPass
    {
        static std::vector<int> vecLoopDominatorsCount;
        static std::vector<std::string> vecDominatorsFuncName;
        static char ID; // Pass identification, replacement for typeid
        DominatorsPass() :  FunctionPass(ID) {}
        virtual ~DominatorsPass() {}
        
        bool runOnFunction(Function &F) override
        {
            getDominatorsInfo(F);
            return false;
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.setPreservesAll();
        }
        
        void getDominatorsInfo(const Function& func) const
        {
            DominatorTree &DomTree = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
            int domCounter = 0;
            for (Function::const_iterator iter = func.begin(); iter != func.end(); ++iter)
            {
                const BasicBlock &currBlock = *iter;
                for (Function::const_iterator nextIter = func.begin(); nextIter != func.end(); ++nextIter)
                {
                    // does a block dominate itself? It does
                    // what about strict dominance?
                    /*if(iter == nextIter)
                    {
                        continue;
                    }*/
                    const BasicBlock &nextBlock = *nextIter;
                    if (DomTree.properlyDominates(&nextBlock, &currBlock))
                    {
                        domCounter++;
                    }
                }
            }
            vecLoopDominatorsCount.push_back(domCounter);
            vecDominatorsFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecLoopDominatorsCount, vecDominatorsFuncName, "DominatorsCount");
            
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> DominatorsPass::vecLoopDominatorsCount;
std::vector<std::string> DominatorsPass::vecDominatorsFuncName;
char DominatorsPass::ID = 0;
static RegisterPass<DominatorsPass>
B("dominatorspass", "loop basic block counter pass.");

namespace
{
    //3.1.1 The number of loops in all functions in the input C file
    struct AllLoopCount : public FunctionPass
    {
        static std::vector<int> vecFuncLoopCounts;
        static std::vector<std::string> vecFuncLoopCountsFuncName;
        static char ID; // Pass identification, replacement for typeid
        AllLoopCount() :  FunctionPass(ID) {}
        virtual ~AllLoopCount() {}
        
        bool runOnFunction(Function &F) override
        {
            getLoopCount(F);
            return false;
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        
        int getLoopCountNested(const Loop &Toploop) const
        {
            int nNestedLoopCount = 1;
            std::vector<Loop *> nestedloops = Toploop.getSubLoops();
            
            for (Loop::iterator iter = nestedloops.begin(); iter != nestedloops.end(); ++iter)
            {
                Loop *loop =*iter;
                nNestedLoopCount += getLoopCountNested(*loop);
            }
            
            return nNestedLoopCount;
        }
        
        void getLoopCount(const Function& func) const
        {
            LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            int loopCounter = 0;
            for (Loop::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
            {
                Loop *loop =*iter;
                loopCounter += getLoopCountNested(*loop);
                
            }
            vecFuncLoopCounts.push_back(loopCounter);
            vecFuncLoopCountsFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecFuncLoopCounts, vecFuncLoopCountsFuncName, "AllLoopsCount", true, false, false);
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> AllLoopCount::vecFuncLoopCounts;
std::vector<std::string> AllLoopCount::vecFuncLoopCountsFuncName;
char AllLoopCount::ID = 0;
static RegisterPass<AllLoopCount>
C("allloops", "counts all the loops including nested loops.");

namespace
{
    //3.1.2 The number of loops that are outermost loops (not nested in any other loops)
    struct TopLevelLoopCount : public FunctionPass
    {
        static std::vector<int> vecTopLoopCounts;
        static std::vector<std::string> vecTopLoopCountsFuncName;
        static char ID; // Pass identification, replacement for typeid
        TopLevelLoopCount() :  FunctionPass(ID) {}
        virtual ~TopLevelLoopCount() {}
        
        bool runOnFunction(Function &F) override
        {
            getLoopCount(F);
            return false;
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        
        void getLoopCount(const Function& func) const
        {
            LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            int loopCounter = 0;
            for (Loop::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
            {
                loopCounter++;
            }
            vecTopLoopCounts.push_back(loopCounter);
            vecTopLoopCountsFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecTopLoopCounts, vecTopLoopCountsFuncName, "TopLoopCount", true, false);
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> TopLevelLoopCount::vecTopLoopCounts;
std::vector<std::string> TopLevelLoopCount::vecTopLoopCountsFuncName;
char TopLevelLoopCount::ID = 0;
static RegisterPass<TopLevelLoopCount>
D("toploops", "counts all the top loops ie not including nested loops.");

namespace
{
    //3.1.3 The total number of loop exit CFG edges for all loops (the source node of the edge is in the loop
    // body, but the destination node is not)
    struct LoopExitCFGCount : public FunctionPass
    {
        static std::vector<int> vecExitCFGLoopCount;
        static std::vector<std::string> vecExitCFGLoopFuncNames;
        static char ID; // Pass identification, replacement for typeid
        
        LoopExitCFGCount() :  FunctionPass(ID) {}
        virtual ~LoopExitCFGCount() {}
        
        bool runOnFunction(Function &F) override
        {
            getLoopCount(F);
            return false;
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        
        void getLoopCount(const Function& func) const
        {
            LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            int loopExitCounter = 0;
            for (Loop::iterator iter = loopInfo.begin(); iter != loopInfo.end(); ++iter)
            {
                Loop *loop = *iter;
                for(Loop::block_iterator bIter = loop->block_begin(); bIter != loop->block_end(); ++bIter)
                {
                    const BasicBlock *loopBlock = *bIter;
                    if(loop->isLoopExiting(loopBlock))
                    {
                        loopExitCounter++;
                    }
                }
            }
            vecExitCFGLoopCount.push_back(loopExitCounter);
            vecExitCFGLoopFuncNames.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecExitCFGLoopCount, vecExitCFGLoopFuncNames, "LoopExitCFGCount", true, false);
            errs() << j.dump();
            return false;
        }
    };
}

std::vector<int> LoopExitCFGCount::vecExitCFGLoopCount;
std::vector<std::string> LoopExitCFGCount::vecExitCFGLoopFuncNames;
char LoopExitCFGCount::ID = 0;
static RegisterPass<LoopExitCFGCount>
E("exitcfgloops", "counts loop exit CFG edges.");
