

#include "pac_analysis.h"
// #include "dependency_walker.cpp"
// #include "instrumenter_loger.cpp"
using namespace llvm;
using namespace std;
using namespace SVF;




static cl::opt<bool>
StandardCompileOpts("std-compile-opts",
                    cl::desc("Include the standard compile time optimizations"));

// static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
//         llvm::cl::desc("<input bitcode>"));
        // , llvm::cl::init("-"));

//static cl::list<const PassInfo*, bool, PassNameParser>
//PassList(cl::desc("Optimizations available:"));

static cl::opt<bool> DAA("daa", cl::init(false),
                         cl::desc("Demand-Driven Alias Analysis Pass"));

static cl::opt<bool> REGPT("dreg", cl::init(false),
                           cl::desc("Demand-driven regular points-to analysis"));

static cl::opt<bool> RFINEPT("dref", cl::init(false),
                             cl::desc("Demand-driven refinement points-to analysis"));

static cl::opt<bool> ENABLEFIELD("fdaa", cl::init(false),
                                 cl::desc("enable field-sensitivity for demand-driven analysis"));

static cl::opt<bool> ENABLECONTEXT("cdaa", cl::init(false),
                                   cl::desc("enable context-sensitivity for demand-driven analysis"));

static cl::opt<bool> ENABLEFLOW("ldaa", cl::init(false),
                                cl::desc("enable flow-sensitivity for demand-driven analysis"));


int arg_num = 9;
char *arg_value[10];
char * arg0="/home/pac/toolchain/SVF/Release-build/bin/dvf";
char * arg1="-cxt";
char* arg2="--cpts";
char* arg3="-max-cxt=2";
char* arg4="-flow-bg=10000";
char* arg5="-cxt-bg=10000";
char* arg6="--fdaa";
// char* arg7="--cdaa";
// char* arg7="-query=funptr";
// char* arg7="-query=7 8";
char* arg7="-query=0";


bool Pac_analysisPass::runOnModule(llvm::Module &M) {

    

     arg_value[0]=arg0;
    arg_value[1]=arg1;
    arg_value[2]=arg2;
   arg_value[3]=arg3; 
   arg_value[4]=arg4; 
   arg_value[5]=arg5; 
   arg_value[6]=arg6; 
   arg_value[7]=arg7; 
//    arg_value[8]=arg8; 
char * filename = (char*) M.getModuleIdentifier().c_str();
errs()<<"Bitcode Filename: "<<filename<<"\n";
    arg_value[8]=filename;
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Demand-Driven Points-to Analysis\n");

    std::vector<std::string> moduleNameVec;
    std::string tmp(filename);
    moduleNameVec.push_back(tmp);

    // SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(M);
    svfModule->buildSymbolTableInfo();

    SVFIRBuilder builder;
    SVFIR* svfir = builder.build(svfModule);

    CSPL_DDAPass *dda = new CSPL_DDAPass();
    // DDAPass *dda = new DDAPass();
    
    dda->runOnModule(svfModule);
    

    /*
    CFLVF* cfl = new CFLVF(svfir);
    // cfl->analyze();
    cfl->buildSVFGraph();
    */

   
    // WPAPass *wpa = new WPAPass();
    /*DDAPass *dda = new DDAPass();
    dda->runOnModule(svfModule);*/
    //  wpa->runOnModule(M);
    //below~~~
    SymbolTableInfo *symInfo = SymbolTableInfo::SymbolInfo();
    SymbolTableInfo::ValueToIDMapTy& valSymMap=symInfo->valSyms();

    // PointerAnalysis* pta = cfl->getCurrentPta();
    // SVFIR* pag= pta->getPAG();
    PointerAnalysis* pta=dda->getCurrentPta();
    ContextDDA* cdda=SVFUtil::cast<ContextDDA>(pta);
    SVFG* svfg=cdda->getSVFG();


    CSDDA csdda(cdda);
    csdda.m=&M;
    // errs() << "ss ello done \n";
    
    SVFIR* pag=svfir;
    // if(svfir==svfg->getPAG()){
    //     errs()<<" EQQQ\n";
    // }
    DepWalker depwalker(M,svfModule,svfir,valSymMap,svfg,pag);
    depwalker.mainpass=this;
    this->depWalker=&depwalker;
    Instrumenter instrumenter(M);
    depwalker.instrumenter=&instrumenter;
    //  errs() << "aaHHaaaHHHsdss ello done \n";
    
        /// collect return node of function fun
   
     for (SVFModule::iterator fit = svfModule->begin(), efit = svfModule->end();
            fit != efit; ++fit)
    {
        const SVFFunction& fun = **fit;
        /// collect return node of function fun
     
        
        for (Function::iterator bit = fun.getLLVMFun()->begin(), ebit = fun.getLLVMFun()->end();
                bit != ebit; ++bit)
        {
            BasicBlock& bb = *bit;
            for (BasicBlock::iterator it = bb.begin(), eit = bb.end();
                    it != eit; ++it)
            {
                    Instruction& inst = *it;
                    // CallBase* callInst = llvm::dyn_cast<llvm::CallBase>(&inst);
                    if(ReturnInst* returnInst = llvm::dyn_cast<llvm::ReturnInst>(&inst)){
                        csdda.logReturnProtectToFile(returnInst);
                    }
                    
                    
                    const LoadInst* loadInst= llvm::dyn_cast<llvm::LoadInst>(&inst);
                    const StoreInst* storeInst= llvm::dyn_cast<llvm::StoreInst>(&inst);
                    if (!loadInst) {
                        continue;
                    }
                    if (loadInst) {
                        llvm::Type* loadType = loadInst->getType();
                        if (!loadType->isPointerTy()) {
                            continue;
                        }
                    }
                    //   errs()<<" Now processing: "<<inst<<"\n";
                    
                        csdda.cs_def_use(&inst);
                        // csdda.pac_root_All_addrStr(((CSPL_ContextDDA*)csdda.cdda)->rootDPM); //Error
                        // csdda.flatten_allDPI_to_strldr(((CSPL_ContextDDA*)csdda.cdda)->rootDPM);
                        csdda.flatten_allDPI_to_strldr(((CSPL_ContextDDA*)csdda.cdda)->rootDPM);
                        // errs()<<"now print\n";
                        csdda.printdpm(*((CSPL_ContextDDA*)csdda.cdda)->rootDPM);
                        
                        // errs()<<"nowinstrument\n";
                        // csdda.instrument(*((CSPL_ContextDDA*)csdda.cdda)->rootDPM);
                        // csdda.pac_addr_instrument(&inst);this is commnets
                        //CALLSITE 0
                        //LOAD_POINTER_USE 1
                        //STORE_POINTER_USE 2
                        //POINTER_TARGET 3
                        csdda.logToFile(&inst); //
                        csdda.flattened_dpmTodpmSet.clear();
                        csdda.printed_DPTItems.clear();
                       


                    //   DepWalker::VisitedSVFGNodeSetTy deps;// =depwalker.getAllUpperDeps_single(&inst,true);
            
                    //   Set<Instruction*> loads=depwalker.getLoad(deps);
                    //   Set<Instruction*> stores=depwalker.getStore(deps);
                    //   errs()<<"Num Load: "<<loads.size()<<"  Num store: "<<stores.size()<<"\n";
                    //   instrumenter.instrument(loads);
                    //   instrumenter.instrument(stores);

                    Set<SVFGNode*> uppernodes=depwalker.getUpperDefs(&inst);
                    for(auto i : uppernodes){
                        // errs()<<" upper: "<<i->toString()<<"\n";
                    }

             
            }
        }
    }   

    
    // errs() << "  data-dependency, Maxsize: "<<depwalker.getMaxfromMap(depwalker.visitedSVFGNode2DepNum)<<" total num: "<< depwalker.visitedSVFGNodeSet.size()<< "  \n";
    // errs()<< " load total num: "<<depwalker.visitedSVFGNode2LoadNum.size()<<"  store total num: "<<depwalker.visitedSVFGNode2StoreNum.size()<<" \n";
    // errs()<< " load max num: "<<depwalker.getMaxfromMap(depwalker.visitedSVFGNode2LoadNum)<<"  store max num: "<<depwalker.getMaxfromMap(depwalker.visitedSVFGNode2StoreNum)<<" \n";
    
    return true;
}


char Pac_analysisPass::ID = 0;
static llvm::RegisterPass<Pac_analysisPass> X("pac_analysisPass", "Rain Pass",
                                     false /* Only looks at CFG */,
                                     false /* Analysis Pass */);

static llvm::RegisterStandardPasses
    Y(llvm::PassManagerBuilder::EP_EarlyAsPossible,
      [](const llvm::PassManagerBuilder &Builder,
         llvm::legacy::PassManagerBase &PM) { PM.add(new Pac_analysisPass()); });





           /*  using original Module
            for (auto &F : M.getFunctionList()) 
    {
       
        for (Function::iterator bit = F.begin(), ebit = F.end();
                bit != ebit; ++bit)
        {
            BasicBlock& bb = *bit;
            for (BasicBlock::iterator it = bb.begin(), eit = bb.end();
                    it != eit; ++it)
            {
                Instruction& inst = *it;
                SymbolTableInfo::ValueToIDMapTy::const_iterator iter =  valSymMap.find(&inst);
        
                
                if(iter!=valSymMap.end())
                {
                    errs()<<"found :" << iter->second<< " inst:"<<inst<<"\n";
                }else{
                    errs()<<"NOTfound : "<<inst<<"\n";
                }
            }
        }
    }
    */

 
