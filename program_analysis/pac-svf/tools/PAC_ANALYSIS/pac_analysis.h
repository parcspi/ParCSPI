#pragma once 

// #include <cassert>
// #include <cstdio>
// #include <fstream>
// #include <iostream>
// #include <set>
// #include <sstream>
// #include <string>
// #include <fstream>
// #include <fcntl.h>
// #include <unistd.h>

// #include "SVF-FE/LLVMUtil.h"
// #include "WPA/WPAPass.h"
// #include "Util/Options.h"
// #include "DDA/DDAPass.h"
// #include "SVF-FE/SVFIRBuilder.h"
// #include "CFL/CFLAlias.h"
// #include "CFL/CFLVF.h"
#include "common.h"



// using namespace llvm;
// using namespace std;
// using namespace SVF;

// namespace {
// class DepWalker;





struct Pac_analysisPass : public llvm::ModulePass {
    static char ID;
    
     DepWalker* depWalker;
    Pac_analysisPass() : ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &M) override;

    Set<Instruction*> _find_difference(Set<BasicBlock*>);
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.setPreservesAll();
    }
    
    // void init_cust();

};  // 


// } 