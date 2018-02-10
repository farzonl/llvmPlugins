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

#define TEST true

namespace
{
    class HelperFunctions
    {
    public:
        static json createAndWriteJson(const std::vector<int> & vecCount,
                                  const std::vector<std::string> & vecFuncName,
                                  const std::string &countName)
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
            j["Minimum"] = { {"functionName", sMinFuncName}, {countName, iMin} };
            j["Maximum"] = { {"functionName", sMaxFuncName}, {countName, iMax} };
            j["Average"] = average;

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
X("basicblock", "basic block function counter pass");

namespace
{
    // 2. Average, maximum and minimum number of CFG edges inside functions.
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
Y("cfgedge", "cfg edge function counter pass");

namespace
{
  //3. Average, maximum and minimum number of single entry loops inside functions (count each loop based on a back edge).
  struct BackEdgeDetector : public /*LoopInfoWrapperPass*/ FunctionPass
  {
    static std::vector<int> vecBackEdgeCount;
    static std::vector<std::string> vecBackEdgeFuncName;
    static char ID; // Pass identification, replacement for typeid
    BackEdgeDetector() : /*LoopInfoWrapperPass()*/ FunctionPass(ID) {}
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
Z("backedge", "back Edge detector pass");
