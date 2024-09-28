#pragma once 

#include "common.h"
class CSDDA{
    public:
    typedef OrderedSet<CSPL_CxtLocDPItem*> DPTptrSet;
    typedef OrderedMap<CSPL_CxtLocDPItem*, DPTptrSet> DPIptrToDPIptrSetMap;

       typedef OrderedMap<Value*, int> InstToIntMap;

       typedef OrderedSet<CSPL_CxtLocDPItem> DPTItemSet;
    typedef OrderedMap<CSPL_CxtLocDPItem, DPTItemSet> DPImToDPImSetMap;

     typedef OrderedSet<Instruction*> InstSet;

    InstSet instrumentedSet;
    InstSet instrumentedCallsite;
    DPTItemSet instrumentedDpmSet;
    OrderedSet<std::pair<Instruction*,int>> instrumentedInstSet;

    DPImToDPImSetMap flattened_dpmTodpmSet;

    DPImToDPImSetMap root_to_addr_storeSet;

    DPTItemSet printed_DPTItems;
    InstSet instrumentedAddrStoreSet;

    DPTItemSet logged_pt_DPTItems;
    DPTItemSet logged_def_use_DPTItems;


    typedef OrderedMap<Instruction*,OrderedSet<std::pair<Instruction*,CxtVar>>> Inst_to_InstCtxvarSet_Map;
    Inst_to_InstCtxvarSet_Map root_to_pointsTo_Map;
    llvm::raw_fd_ostream* ofs_pt;
    llvm::raw_fd_ostream* ofs_str_pt;
    llvm::raw_fd_ostream* ofs_du;

    ContextDDA* cdda;
    SVFG* svfg;
    SVFIR* pag;
    llvm::Module *m;

    int instruction_ID=1;
    InstToIntMap instToIDMap; //pac: not needs to all inst
    int getInstID(Value* inst){
        if(instToIDMap.find(inst)==instToIDMap.end()){
            instToIDMap.insert(std::make_pair(inst,instruction_ID));
            // [inst]=instruction_ID;
            instruction_ID++;
        }
        return instToIDMap[inst];
    }
    CSDDA(ContextDDA* _cdda):cdda(_cdda){
        pag=_cdda->getPAG();
        svfg=cdda->getSVFG();
        int fd = open("pac_log_pt.txt", O_WRONLY | O_CREAT, 0666);
        ofs_pt= new llvm::raw_fd_ostream(fd,true);

        int _fd = open("pac_log_du.txt", O_WRONLY | O_CREAT, 0666);
        ofs_du= new llvm::raw_fd_ostream(_fd,true);

        int __fd = open("pac_log_str_pt.txt", O_WRONLY | O_CREAT, 0666);
        ofs_str_pt=new llvm::raw_fd_ostream(__fd,true);
    }

    void cs_def_use(Instruction* inst);
    DPTItemSet flatten_DPI_strldr( CSPL_CxtLocDPItem *root);
    void flatten_allDPI_to_strldr( CSPL_CxtLocDPItem *root);
    DPTItemSet pac_root_addrStr( const CSPL_CxtLocDPItem *root);
    void pac_root_All_addrStr( const CSPL_CxtLocDPItem *root);

    
    void instrument(CSPL_CxtLocDPItem &root);
    void pac_addr_instrument(Instruction* root_inst);
    void logReturnProtectToFile(Instruction* inst);
    void logToFile(Instruction* root_inst);
     inline int getTrueOffset(Instruction* _inst){
            BasicBlock* bb=_inst->getParent();
            int position=0;
             for (BasicBlock::iterator it = bb->begin(), eit = bb->end();
                    it != eit; ++it){
                         Instruction* inst = &(*it);
                        if(inst==_inst){
                            return position;
                        }
                         if(const CallInst* callInst= llvm::dyn_cast<llvm::CallInst>(inst)){
                                    llvm::Function *calledFunc=callInst->getCalledFunction();
                                    if(calledFunc){
                                        if(calledFunc->getName()=="pgi_load_check"||calledFunc->getName()=="pgi_store_check"||calledFunc->getName()=="pgi_callsite"){
                                            continue;
                                        }
                                    }
                         }
                         position++;
                    }

                    return position;
    }
    inline Instruction* getInstFromTrueOffset(BasicBlock* bb,int offset){

        // BasicBlock* bb=_inst->getParent();
            int position=0;
             for (BasicBlock::iterator it = bb->begin(), eit = bb->end();
                    it != eit; ++it){
                         Instruction* inst = &(*it);
                        
                         if(const CallInst* callInst= llvm::dyn_cast<llvm::CallInst>(inst)){
                                    llvm::Function *calledFunc=callInst->getCalledFunction();
                                    if(calledFunc){
                                        if(calledFunc->getName()=="pgi_load_check"||calledFunc->getName()=="pgi_store_check"||calledFunc->getName()=="pgi_callsite"){
                                            continue;
                                        }
                                    }
                         }
                         if(offset==position){
                            return inst;
                        }
                         position++;
                    }
                return nullptr;
    }
    
    std::string getInstLabel(Instruction* inst){
        std::string s;
        if(!inst){
            return s;
        }
        llvm::raw_string_ostream ss(s);
        // errs()<<*inst<< "-hhhh\n";
        if(!inst->getParent()||!inst->getParent()->getParent()){
            return s;
        }
        ss<<inst->getParent()->getParent()->getName()<<",";
        // errs()<<"dfffg\n";
        inst->getParent()->printAsOperand(ss,false);
         ss<<","<<getTrueOffset(inst);
         return s;
    }
    std::string getDpmLabel(CSPL_CxtLocDPItem &dpm);
    std::string getCtxVarLabel(CxtVar& var);
    std::string  getCtxVarStrCallsiteLabel(CxtVar& var,Instruction* rootInst);
    Value* getStoreOrOriginalFromCVar(CxtVar& var,Instruction* rootInst){

        for(auto& item:root_to_pointsTo_Map[rootInst]){
            // break;
            // errs()<<"--->"<<item.second.toString()<<" : "<<item.first<<"\n";
            
            errs()<<*(item.first)<<"\n";
            if(item.second==var){
                return item.first;
            }
        }
        //if not store
        {
            PAGNode *pn = pag->getGNode(var.get_id());
            if (!pn->hasValue())
            {
                errs() << "NoValue\n";
                // label="NoooInst";
                return nullptr;
            }
            const Value *tmpv = pn->getValue();
            return (Value *)tmpv;
        }
        return nullptr;
    }
    Instruction* getInstFromLabel(std::string label){
          std::stringstream ss(label);
         std::string funName,bbLabel;
        int offset;
        // errs()<<"here dived- "<<label<<" : ";


        std::getline(ss, funName, ',');
            errs() << funName << " )) ";

             std::getline(ss, bbLabel, ',');
            errs() << bbLabel << " )) ";
            std::string tmp;
             std::getline(ss, tmp, ',');
             offset=std::stoi(tmp);
            errs() << offset << " )) ";
            errs() << " \n";

            Function* fun = m->getFunction(funName);
            if(!fun){
                return nullptr;
            }
             BasicBlock * basicblock=nullptr;
            for (Function::iterator bb = fun->begin(), e = fun->end(); bb != e; ++bb) {
                BasicBlock * tmp=&(*bb);
                std::string s;
                llvm::raw_string_ostream ss(s);
                tmp->printAsOperand(ss,false);
                if(s==bbLabel){
                    basicblock=tmp;
                    break;
                }
            }
            return getInstFromTrueOffset(basicblock,offset);


       
    } 

    bool isLdrStr(CSPL_CxtLocDPItem& dpm){
        // return true;
        // (SVFUtil::isa<StoreStmt>())
        if(StoreVFGNode::classof(dpm.getLoc())||LoadVFGNode::classof(dpm.getLoc())){
            return true;
        }else{
            return false;
        }
   
    }
    bool isLoad(CSPL_CxtLocDPItem& dpm){
        if(LoadVFGNode::classof(dpm.getLoc())){
            return true;
        }else{
            return false;
        }
    }
    LoadInst* getLoad(CSPL_CxtLocDPItem& dpm){
        if(isLoad(dpm)){
            return (LoadInst*)dpm.getLoc()->getValue();
        }else{
            return nullptr;
        }
    }
    Instruction* getCallsite(CallSiteID csId){
        return (Instruction*)cdda->getPTACallGraph()->getCallSite(csId)->getCallSite();
    }

    

    bool isStore(CSPL_CxtLocDPItem& dpm){
        if(StoreVFGNode::classof(dpm.getLoc())){
            return true;
        }else{
            return false;
        }
    }
    StoreInst* getStore(CSPL_CxtLocDPItem& dpm){
        if(isStore(dpm)){
            return (StoreInst*)dpm.getLoc()->getValue();
        }else{
            return nullptr;
        }
    }

    Instruction* getLdrStrInst(CSPL_CxtLocDPItem& dpm){

        if(isStore(dpm)){
            return (Instruction*)getStore(dpm);
        }else if(isLoad(dpm)){
            return (Instruction*)getLoad(dpm);
        }
        return nullptr;
    }
    Instruction* getLdrStrInst(const CSPL_CxtLocDPItem& dpm){

      
        return getLdrStrInst(const_cast<CSPL_CxtLocDPItem&>(dpm));
    }

     void printdpm(CSPL_CxtLocDPItem& dpm);
};