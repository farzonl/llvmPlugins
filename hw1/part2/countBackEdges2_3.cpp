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
#include "llvm/Analysis/PostDominators.h"

#include <nlohmann/json.hpp>
#include<valarray>
#include <fstream>
#include <sstream>      // std::stringstream, std::stringbuf
#include <stack>
#include <set>
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
            errs() << j.dump() <<"\n";
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
                const TerminatorInst *termInst = block.getTerminator();
                // count the jumps
                numEdges += termInst->getNumSuccessors();
            }
            veccCFGEdgeCount.push_back(numEdges);
            vecCFGEdgeFuncName.push_back(func.getName());
        }
        
        bool doFinalization(Module &M) override {
            
            json j = HelperFunctions::createAndWriteJson(veccCFGEdgeCount, vecCFGEdgeFuncName, "CFGEdgeCount");
            errs() << j.dump() <<"\n";
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
        errs() << j.dump() <<"\n";
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
            errs() << j.dump() <<"\n";
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
            
            errs() << j.dump() <<"\n";
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
            errs() << j.dump() <<"\n";
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
            errs() << j.dump() <<"\n";
            return false;
        }
    };
}


std::vector<int> LoopExitCFGCount::vecExitCFGLoopCount;
std::vector<std::string> LoopExitCFGCount::vecExitCFGLoopFuncNames;
char LoopExitCFGCount::ID = 0;
static RegisterPass<LoopExitCFGCount>
E("exitcfgloops", "counts loop exit CFG edges.");

namespace
{
    typedef std::map<const BasicBlock *, std::map<const BasicBlock *, int> > matBasicBlocks;
    typedef std::map<const BasicBlock *, std::map<const BasicBlock *, const BasicBlock *> > matSucBasicBlocks;
    typedef std::vector<const BasicBlock *> basicBlockPath;
    ///3.2 Using the cycles detected, determine the number of single entry loops as well as multi-entry loop.
    
    //ex. A loop is a cycle characterized by the entry point. For example, if a cycle is entered at two different points
    // (multi-entry), it is to be counted as two separate loops.
    
    // part 2: compare the time taken for your algorithm with that the originally
    // implemented dominator based single entry loop detector
    
    // part 3: Also compare the number of additional loops detected by your algorithm vs the dominator based one
    //  part 4: Is Mr. Compiler justified in changing the loop detector?
    struct Warshall3_2 : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        const int infinity = SHRT_MAX;
        const int minValue = 1;
        static std::vector<int> vecWarshallCounts;
        static std::vector<std::string> vecWarshallFuncName;
        Warshall3_2() :  FunctionPass(ID) {}
        virtual ~Warshall3_2() {}
        
        bool runOnFunction(Function &F) override
        {
            errs() << F.getName();
            matBasicBlocks dist;
            matSucBasicBlocks next;
            warhsalAlgo(F, dist, next);
            pathReconstruction(F, dist, next);
            return false;
        }
        
        void printMap(matBasicBlocks &aMap)
        {
            for (auto& t : aMap)
            {
                for (auto& tt : t.second)
                {
                    if(tt.second == SHRT_MAX)
                        errs() << "INF ";
                    else
                        errs() << " " << tt.second << "  ";
                }
                errs() << "\n";
            }
        }
        
        void printMap(matSucBasicBlocks &aMap)
        {
            
            for (auto& t : aMap)
            {
                (t.first)->printAsOperand(errs(), false);
                errs() << ": ";
                for (auto& tt : t.second)
                {
                    if(tt.second == nullptr)
                    {
                        errs() << "NULL ";
                    }
                    else
                    {
                        errs() << " ";
                        (tt.second)->printAsOperand(errs(), false);
                        errs() << "  ";
                    }
                }
                errs() << "\n";
            }
        }
        void printMapPointer(matSucBasicBlocks &aMap)
        {
            for (auto& t : aMap)
            {
                for (auto& tt : t.second)
                {
                    if(tt.second == nullptr)
                    {
                        errs() << "nullptr        ";
                    }
                    else
                    {
                       errs() << tt.second << "  ";
                    }
                }
                errs() << "\n";
            }
        }
        
        void printVector(basicBlockPath &avector)
        {
            errs() << "[";
            for (auto v = avector.begin(); v != avector.end(); ++v)
            {
                (*v)->printAsOperand(errs(), false);
                errs() << " ";
            }
            errs() << "]\n";
        }
        /*
         https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm#Path_reconstruction
        procedure Path(u, v)
            if next[u][v] = null then
                return []
            path = [u]
            while u ≠ v
                u ← next[u][v]
                path.append(u)
            return path
        */
        basicBlockPath Path(const BasicBlock *u, const BasicBlock *v, matSucBasicBlocks &next)
        {
            if(next[u][v] == nullptr)
            {
                return basicBlockPath();
            }
            basicBlockPath path;
            path.push_back(u);
            const BasicBlock *u_inc = u;
            while(u_inc != v)
            {
                u_inc = next[u_inc][v];
                path.push_back(u_inc);
            }
            return path;
        }
        
        basicBlockPath mergePaths(const basicBlockPath &vuPath,const basicBlockPath &uvPath)
        {
            basicBlockPath concatPath;
            if(vuPath.size() > 0)
            {
                concatPath.insert(concatPath.end(), vuPath.begin(), vuPath.end());
            }
            if(vuPath.size() > 0)
            {
                if(concatPath.back() == uvPath.front())
                {
                    concatPath.pop_back();
                }
                
                concatPath.insert(concatPath.end(), uvPath.begin(), uvPath.end());
            }
            return concatPath;
        }
        
        std::string getPathHash(const basicBlockPath &path)
        {
            std::stringstream ss;
            for (auto v = path.begin(); v != path.end(); ++v)
            {
                 ss << *v << " ";
            }
            return ss.str();
        }
        
        int LoopCounter(Function &func, basicBlockPath &path, std::map<std::string,bool> &seenPathsPred)
        {
            DominatorTree *DomTree = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
            int iLoopCounter = 0;
            for (auto node = path.begin(); node != path.end(); ++node)
            {
                const BasicBlock* searchNode = *node;
                for (Function::const_iterator iter = func.begin(); iter != func.end(); ++iter)
                {
                    bool done = false;
                    const BasicBlock &currBlock = *iter;
                    if(std::find(path.begin(), path.end(), &currBlock) == path.end())
                    {
                        const TerminatorInst *termInst = currBlock.getTerminator();
                        for (unsigned int v_succIndex = 0; v_succIndex < termInst->getNumSuccessors(); v_succIndex++)
                        {
                             const BasicBlock *v_succ = termInst->getSuccessor(v_succIndex);
                            if(v_succ == searchNode)
                            {
                                if (!DomTree->dominates(searchNode, &currBlock))
                                {
                                    basicBlockPath predList;
                                    predList.push_back(&currBlock);
                                    predList.push_back(searchNode);
                                    std::string predListHash = getPathHash(predList);
                                    //errs() << "\npath: " << predListHash << "\n";
                                    /*errs() << "[ (";
                                    searchNode->printAsOperand(errs(), false);
                                    errs() << " ," << searchNode << "), ";
                                    v_succ->printAsOperand(errs(), false);
                                    errs() << " ," << v_succ << ") ]\n";*/
                                    if(seenPathsPred.find(predListHash) == seenPathsPred.end())
                                    {
                                        seenPathsPred[predListHash] = true;
                                        iLoopCounter++;
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if(done) break;
                }
            }
            return iLoopCounter;
        }
        
        bool areVectorsPermutations(basicBlockPath & path1, basicBlockPath & path2)
        {
            if(path1.size() != path2.size())
            {
                return false;
            }
            for(size_t i = 0; i < path1.size(); i++)
            {
                bool bExists = false;
                for(size_t j = 0; j < path2.size(); j++)
                {
                    if(path1[i] == path2[j])
                    {
                        bExists =  true;
                        break;
                    }
                }
                if (bExists == false)
                {
                    return false;
                }
            }
            
            return true;
        }
        
        void pathReconstruction(Function &func, matBasicBlocks &dist, matSucBasicBlocks &next)
        {
            int iLoopCounter = 0;
            std::map<std::string,bool> seenPathCombos;
            std::map<std::string,bool> seenPathsPred;
            std::vector<basicBlockPath> seenPaths;
            for (Function::const_iterator v_iter = func.begin(); v_iter != func.end(); ++v_iter)
            {
                const BasicBlock *v_Block = &*v_iter;
                for (Function::const_iterator u_iter = func.begin(); u_iter != func.end(); ++u_iter)
                {
                    const BasicBlock *u_Block = &*u_iter;
                    
                    if (dist[v_Block][u_Block] == infinity || //skip non-weighted path
                        dist[u_Block][v_Block] == infinity ||
                        dist[v_Block][u_Block] == 0        || // skip [v][v]
                        dist[u_Block][v_Block] == 0)
                    {
                        continue;
                    }
                    basicBlockPath vuPath = Path(v_Block, u_Block, next);
                    basicBlockPath uvPath = Path(u_Block, v_Block, next);
                    basicBlockPath path = mergePaths(vuPath, uvPath);
                    std::string pathHash = getPathHash(path);
                    
                    if(seenPathCombos.find(pathHash) == seenPathCombos.end())
                    {
                        seenPathCombos[pathHash] = true;
                        
                        if(path.front() == path.back())
                        {
                            
                            path.pop_back(); // we found a cycle make it a-cyclic
                            bool addToSeenPaths = true;
                            for (auto seenPathIter = seenPaths.begin(); seenPathIter != seenPaths.end(); ++seenPathIter)
                            {
                                basicBlockPath seenPath =*seenPathIter;
                                if(areVectorsPermutations(seenPath,path))
                                {
                                    addToSeenPaths = false;
                                }
                                
                            }
                            if(addToSeenPaths)
                            {
                                //errs() << "\nnew path found:\n";
                                //printVector(path);
                                seenPaths.push_back(path);
                                iLoopCounter += LoopCounter(func, path, seenPathsPred);
                            }
                        }
                        else
                        {
                            errs() << "path does not have a cycle exiting loop";
                        }
                        
                    }
                }
            }
            vecWarshallCounts.push_back(iLoopCounter);
            vecWarshallFuncName.push_back(func.getName());
            //errs() << "Loop Count: " << iLoopCounter << "\n";
        }
        
        void warhsalAlgo(Function &func, matBasicBlocks &dist, matSucBasicBlocks &next)
        {
            /*
             https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
             1 let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)
             2 for each vertex v
             3    dist[v][v] ← 0
             4 for each edge (u,v)
             5    dist[u][v] ← w(u,v)  // the weight of the edge (u,v)
             6 for k from 1 to |V|
             7    for i from 1 to |V|
             8       for j from 1 to |V|
             9          if dist[i][j] > dist[i][k] + dist[k][j]
             10             dist[i][j] ← dist[i][k] + dist[k][j]
             11         end if
             */
            
            
            // initialized dist to ∞ (infinity)
            for (Function::const_iterator v_iter = func.begin(); v_iter != func.end(); ++v_iter)
            {
                const BasicBlock *v_Block = &*v_iter;
                for (Function::const_iterator u_iter = func.begin(); u_iter != func.end(); ++u_iter)
                {
                    const BasicBlock *u_Block = &*u_iter;
                    dist[v_Block][u_Block] = infinity;
                    
                    // note: for path recon
                    //let next be a |V| × |V| array of vertex indices initialized to null
                    next[v_Block][u_Block] = nullptr;
                }
            }
            //errs() << "init to int max: \n";
            //printMap(dist);
            
            //4-5
            for (Function::const_iterator v_iter = func.begin(); v_iter != func.end(); ++v_iter)
            {
                const BasicBlock *v_Block = &*v_iter;
                const TerminatorInst *termInst = v_Block->getTerminator();
                for (unsigned int v_succIndex = 0; v_succIndex < termInst->getNumSuccessors(); v_succIndex++)
                {
                    const BasicBlock *v_succ = termInst->getSuccessor(v_succIndex);
                    dist[v_Block][v_succ] = minValue;
                    
                    //  note: path Recon next[u][v] ← v
                    next[v_Block][v_succ] = v_succ;
                }
            }
            
            //errs() << "add min path weights:\n";
            //printMap(dist);
            
            //line 2-3
            for (Function::const_iterator v_iter = func.begin(); v_iter != func.end(); ++v_iter)
            {
                const BasicBlock *v_Block = &*v_iter;
                dist[v_Block][v_Block] = 0;
            }
            
            //errs() << "0 out [v][v]:\n";
            //printMap(dist);
            
            //line 6-11
            for (Function::const_iterator k_iter = func.begin(); k_iter != func.end(); ++k_iter)
            {
                const BasicBlock *k_Block = &*k_iter;
                for (Function::const_iterator i_iter = func.begin(); i_iter != func.end(); ++i_iter)
                {
                    const BasicBlock *i_Block = &*i_iter;
                    for (Function::const_iterator j_iter = func.begin(); j_iter != func.end(); ++j_iter)
                    {
                        const BasicBlock *j_Block = &*j_iter;
                        if (dist[i_Block][j_Block] > dist[i_Block][k_Block] + dist[k_Block][j_Block])
                        {
                            dist[i_Block][j_Block] = dist[i_Block][k_Block] + dist[k_Block][j_Block];
                            
                            // note: path Recon next[i][j] ← next[i][k]
                            next[i_Block][j_Block] = next[i_Block][k_Block];
                            
                        }
                    }
                }
            }
            errs() << "Warshall graph:\n";
            printMap(dist);
            
            errs() << "Warshall next graph:\n";
            printMap(next);
            errs() << "Warshall next graph pointers:\n";
            printMapPointer(next);
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.setPreservesAll();
        }
        
        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecWarshallCounts, vecWarshallFuncName, "WarshLoopCount", true, false);
            errs() << j.dump() <<"\n";
            return false;
        }
    };
}

char Warshall3_2::ID = 0;
std::vector<int> Warshall3_2::vecWarshallCounts;
std::vector<std::string> Warshall3_2::vecWarshallFuncName;
static RegisterPass<Warshall3_2>
F("warshloopdetector", "counts loop using warshall.");


namespace
{
    //3.3  find a basic block whose predicate’s outcome decides the direction of the branch
    //     which in turn decides execution condition of a basic block further down in the control path.
    //     Def: In a CFG, a basic block j is control dependent on basic block i if
    //     I. There exists a non-null path p from i to j such that j post-dominates every node on path p after i
    //     II. j does not strictly post-dominate i
    //     clarify:  j post-dominates one of the successors of i but not the other one
    struct ControlDependence  : public FunctionPass
    {
        static std::vector<int> vecCount;
        static std::vector<std::string> vecFuncNames;
        static char ID; // Pass identification, replacement for typeid
        
        ControlDependence() :  FunctionPass(ID) {}
        virtual ~ControlDependence() {}
        
        bool runOnFunction(Function &F) override
        {
            postDomAnalysis(F);
            return false;
        }
        
        void printMap(std::map<const BasicBlock *, std::vector<BasicBlock *>> &aMap)
        {
            if(aMap.size() == 0)
            {
                errs() << "found no dependencies\n";
            }
            
            for (auto& t : aMap)
            {
                errs() << "<" << t.first << ": ";
                t.first->printAsOperand(errs(), false);
                errs() << " >= ";
                printVector(t.second);
            }
        }
        
        void printVector(std::vector<BasicBlock *> &avector)
        {
            errs() << "[";
            for (auto v = avector.begin(); v != avector.end(); ++v)
            {
                errs() << "(" << (*v) <<": ";
                (*v)->printAsOperand(errs(), false);
                errs() << "), ";
            }
            errs() << "]\n";
        }
        
        void postDomAnalysis(Function &func)
        {
            int controlDependenceCount = 0;
            errs() << "Start postDomAnalysis on "<< func.getName() << ":\n";
            std::map<const BasicBlock *, std::vector<BasicBlock *>> postDominateMap;
            PostDominatorTree *postDomTree = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
            for (Function::const_iterator i_iter = func.begin(); i_iter != func.end(); ++i_iter)
            {
                const BasicBlock *i_Block = &*i_iter;
                for (Function::const_iterator j_iter = func.begin(); j_iter != func.end(); ++j_iter)
                {
                     const BasicBlock *j_Block = &*j_iter;
                    if (!postDomTree->dominates(j_Block, i_Block)) // j does not strictly post-dominate i
                    {
                        //errs() << "( J: "<<j_Block << "does not Dominate I: " << i_Block << ")";
                        //such that j post-dominates every node on path p after i
                        // so now we need the successors of i
                        const TerminatorInst *termInst = i_Block->getTerminator();
                        int iSuccCount = termInst->getNumSuccessors();
                        for(int i = 0; i < iSuccCount; i++)
                        {
                            BasicBlock *i_Succ = termInst->getSuccessor(i);
                            //here j needs to post dominate every node
                            if (postDomTree->dominates(j_Block, i_Succ))
                            {
                                controlDependenceCount++;
                                postDominateMap[j_Block].push_back(i_Succ);
                                //errs() << "( J: "<<j_Block << "Dominates: " << i_Succ << ")\n";
                            }
                        }
                    }
                }
            }
            vecCount.push_back(controlDependenceCount);
            vecFuncNames.push_back(func.getName());
            printMap(postDominateMap);
            errs() << "\n";
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<PostDominatorTreeWrapperPass>();
            AU.setPreservesAll();
        }

        bool doFinalization(Module &M) override {
            json j = HelperFunctions::createAndWriteJson(vecCount, vecFuncNames, "ControlDependence", true, false);
            errs() << j.dump() <<"\n";
            return false;
        }
    };
}


std::vector<int> ControlDependence::vecCount;
std::vector<std::string> ControlDependence::vecFuncNames;
char ControlDependence::ID = 0;
static RegisterPass<ControlDependence>
G("controldep", "find a basicblock predicate's that decide the direction of the branch ");

namespace
{
    //3.4 this function returns true if there exists a directed path from basic block A to B, false otherwise.
    struct ReachablePass  : public FunctionPass
    {
        static std::vector<int> vecCount;
        static std::vector<std::string> vecFuncNames;
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
                errs() << "(" << (*v) <<": ";
                (*v)->printAsOperand(errs(), false);
                errs() << "), ";
            }
            errs() << "]\n";
        }

        
        void reachable(Function &func)
        {
            errs() << "Start reachable analysis on "<< func.getName() << ":\n";
            int nReachable = 0;
            int nNotReachable = 0;
            int longestReachablePath = 0;
            std::vector<const BasicBlock*> longestPath;
            for (Function::const_iterator i_iter = func.begin(); i_iter != func.end(); ++i_iter)
            {
                const BasicBlock *A_Block = &*i_iter;
                for (Function::const_iterator j_iter = func.begin(); j_iter != func.end(); ++j_iter)
                {
                    const BasicBlock *B_Block = &*j_iter;
                    //need to perform a graph search from i to j
                    std::vector<const BasicBlock*> path = dfs(A_Block, B_Block, nReachable);
                    if((int) path.size() >longestReachablePath)
                    {
                        longestReachablePath = path.size();
                        longestPath = path;
                    }
                    if(path.empty())
                    {
                        nNotReachable++;
                    //    errs() << "{B: "<<B_Block << "} is Not reachable from {A:"<< A_Block << "}" ;
                    }
                    //printList(path);
                }
            }
            int totalPaths = nReachable+nNotReachable;
            errs() << "reachablility score:"<<nReachable << "/" << totalPaths << " = " << nReachable/static_cast<double>(totalPaths)<< "\n";
            errs() << "longest reachable path: " << longestReachablePath << "\n";
            errs() << "longest path: ";
            printList(longestPath);
            errs() << "End reachable analysis on "<< func.getName() <<"\n\n";
            vecCount.push_back(nReachable);
            vecFuncNames.push_back(func.getName());
        }
        
        /*
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
            json j = HelperFunctions::createAndWriteJson(vecCount, vecFuncNames, "NodesReachable", true, false);
            errs() << j.dump() <<"\n";
            return false;
        }
    };
}
std::vector<int> ReachablePass::vecCount;
std::vector<std::string> ReachablePass::vecFuncNames;
char ReachablePass::ID = 0;
static RegisterPass<ReachablePass>
H("reachable", "find reachability from A to B");
