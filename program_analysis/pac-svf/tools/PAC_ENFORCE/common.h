
#pragma once 


#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>



#include "SVF-FE/LLVMUtil.h"
#include "WPA/WPAPass.h"
#include "Util/Options.h"
#include "DDA/DDAPass.h"
#include "SVF-FE/SVFIRBuilder.h"
#include "CFL/CFLAlias.h"
#include "CFL/CFLVF.h"
#include "DDA/ContextDDA.h"

using namespace llvm;
using namespace std;
using namespace SVF;






