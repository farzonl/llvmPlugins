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
using namespace llvm;


//STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace 
{
  struct BackEdgeDetector : public LoopInfoWrapperPass /*FunctionPass*/
  {
    static char ID; // Pass identification, replacement for typeid
    BackEdgeDetector() : LoopInfoWrapperPass() /*FunctionPass(ID)*/ {}
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
    
    void getBackEdgeInfo(const Function& F) const
    {
        LoopInfo &loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        int backEdgesCount = 0;
        int currTreeDepth = 0;
        int prevTreeDepth = 0;
        
        for (Function::const_iterator iter = F.begin(); iter != F.end(); ++iter)
        {
          currTreeDepth = loopInfo.getLoopDepth(&*iter);
          errs() << "current tree depth: " << currTreeDepth << '\n';
          //Back edges point from a node to one of its ancestors in the DFS tree.
          // therefore the prevDepth should be larger than the current depth
          if (prevTreeDepth > currTreeDepth)
          {
            backEdgesCount++;
          }

          prevTreeDepth = currTreeDepth;
        }
        errs() << "Total back edges: " << backEdgesCount << '\n';
      }
  };
}

char BackEdgeDetector::ID = 0;
static RegisterPass<BackEdgeDetector>
X("backedge", "back Edge detector pass");
