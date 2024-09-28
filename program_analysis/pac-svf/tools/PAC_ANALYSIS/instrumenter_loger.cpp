#include "common.h"

 void Instrumenter::instrument(Set<Instruction*> toadd){
            *ofs<<"\n";
            for(auto inst:toadd){
                        if(!inst){
                            continue;
                        }

                        {
                           
                             if(const LoadInst* loadInst= llvm::dyn_cast<llvm::LoadInst>(inst)){
                                  int num_def=1;
                                 if(load2numdef.find(inst)!=load2numdef.end()){                    
                                    num_def=load2numdef[inst];
                                }
                                    *ofs<<inst->getParent()->getParent()->getName()<<",";
                                    inst->getParent()->printAsOperand(*ofs,false);
                            *ofs<<","<<getTrueOffset(inst)<<","<<num_def<<";";
                             }else if(const StoreInst* storeInst= llvm::dyn_cast<llvm::StoreInst>(inst)){
                                    *ofs<<inst->getParent()->getParent()->getName()<<",";
                                      inst->getParent()->printAsOperand(*ofs,false);
                                     *ofs<<","<<getTrueOffset(inst)<<","<<-1<<";";
                             }
                            
                        }
                        if(instrumented_inst.find(inst)!=instrumented_inst.end()){
                            continue;
                        }
                          llvm::IRBuilder<> ib(inst);
           
                   if(const LoadInst* loadInst= llvm::dyn_cast<llvm::LoadInst>(inst)){
                    int num_def=1;
                    if(load2numdef.find(inst)!=load2numdef.end()){
                       
                        num_def=load2numdef[inst];
                         errs()<<"here inst "<<*loadInst<<" dd "<<num_def<<"\n";
                    }
                    // *ofs<<"【"<<getTrueOffset(inst)<<"】"<<";";
        
                    llvm::FunctionCallee fnew=M.getOrInsertFunction("dfi_load_check",llvm::Type::getVoidTy(M.getContext()),llvm::IntegerType::getInt32Ty(M.getContext()));
                    llvm::Value* arg=llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()),num_def);
                    ib.CreateCall(fnew,{arg});
                    instrumented_inst.insert(inst);
                    // inst->getParent()->printAsOperand(*ofs,false);
                    // *ofs<<","<<getTrueOffset(inst)<<","<<num_def<<";";
        
                   }else if(const StoreInst* storeInst= llvm::dyn_cast<llvm::StoreInst>(inst)){
                    // *ofs<<"【"<<getTrueOffset(inst)<<"】"<<";";
                        llvm::FunctionCallee fnew=M.getOrInsertFunction("dfi_store_check",llvm::Type::getVoidTy(M.getContext()),llvm::IntegerType::getInt32Ty(M.getContext()));
                    llvm::Value* arg=llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()),1);
                     ib.CreateCall(fnew,{arg});
                    instrumented_inst.insert(inst);
                    
                    
                    // ofs->flush();
                   }
                
                    
            }
            
    }



    void Instrumenter::pgi_instrument(DPImToDPImSetMap& dPImToDPImSetMap,Instruction* root,CSPL_CxtLocDPItem* rootdpm ){
        CSPL_CxtLocDPItem& dpm=*rootdpm;
    //       NodeID id = var.get_id();
    // PAGNode* node = getPAG()->getGNode(id);
    // CxtLocDPItem dpm = getDPIm(var, getDefSVFGNode(node));
        errs()<<"the root instruction is: "<< *root<<" with pointer: "<<dpm.getLoc()->toString() <<" context: "<<dpm.getCond().toString()<<"\n";

        DPImToDPImSetMap ldrStrOnly;
        

    }