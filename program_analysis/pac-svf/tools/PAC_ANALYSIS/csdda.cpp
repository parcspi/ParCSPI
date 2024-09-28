#include "csdda.h"

std::string CSDDA::getDpmLabel(CSPL_CxtLocDPItem &dpm)
{
    std::string label;
    Instruction *inst = (Instruction *)dpm.getLoc()->getValue();

    label = getInstLabel(inst);

    label += "[";
    CallStrCxt &ctx = dpm.getCond().getContexts();
    for (auto n : ctx)
    {
        Instruction *callinst = getCallsite(n > 0 ? n : -n);
        std::string clabel = getInstLabel(callinst);
        label = n > 0 ? label + "+" : label + "-";
        label += clabel;
        label += ";";
    }
    label += "]";
    return label;
}

std::string CSDDA::getCtxVarStrCallsiteLabel(CxtVar &var, Instruction *root_inst)
{
    std::string label;

    Value *inst = getStoreOrOriginalFromCVar(var, root_inst);
    // errs() << "sfdf\n";
    if (!inst)
    {

        return label;
    }

    PAGNode *p_n = pag->getGNode(var.get_id());
    if (!p_n->hasValue())
    {

        return label;
    }
    else
    {
    }

    errs().flush();

    // SVFUtil::outs().flush();
    PAGNode *pn = pag->getGNode(var.get_id());
    if (!pn->hasValue())
    {

        return label;
    }
    const Value *tmpv = pn->getValue();

    if (!inst || !tmpv->getType()->isPointerTy())
    {

        return label;
    }
    else
    {
        // Instruction *strInst = llvm::dyn_cast<Instruction>((Value *)tmpv);
        label = std::to_string(getInstID(inst)) + ":" + std::to_string(getInstID((Value *)tmpv));
        // errs() << "original: " << *tmpv << "\n";
        for (User *U : ((Value *)tmpv)->users())
        {
            Instruction *strInst = llvm::dyn_cast<Instruction>((Value *)U);
            if (strInst)
                if (CallBase *callInst = llvm::dyn_cast<llvm::CallBase>(strInst))
                {
                    llvm::Function *calledFunc = callInst->getCalledFunction();
                    // errs()<<"calledFunc: "<<calledFunc->getName()<<"\n";
                    if (calledFunc)
                    {
                        if (calledFunc->getName().startswith("pac_"))
                        {
                            // errs() << "filtered:" << calledFunc->getName() << "\n";
                            continue;
                        }
                    }
                }

            if (strInst && instrumentedInstSet.find({strInst, 2}) == instrumentedInstSet.end())
            {
                llvm::Module &M = *m;
                // errs() << "Instrumenting-: " << *strInst << "\n";
                // continue;
                llvm::IRBuilder<> ib(strInst);
                // llvm::IRBuilder<> ib_2(strInst);
                // llvm::IRBuilder<> ib_3(strInst);
                // Type *elementType = tmpv->getType()->getPointerElementType();

                // Step 3: Create the GEP using the element type (automatically determined)
                // Value *gepPtr = ib.CreateGEP(
                //     elementType,                                // The type of the element the pointer points to
                //     (Value*)tmpv,                                       // The pointer value
                //     {ib.getInt32(0), ib.getInt32(0)}, // GEP indices (example for accessing the first field)
                //     "generated_gep_ptr"                                   // Optional name for the GEP instruction
                // );
                // errs() << "gepPtr: " << *gepPtr << "\n";
                // Step 4: Bitcast the GEP result to void* (i8*)
                Value *voidPtr = ib.CreateBitCast((Value*)tmpv, Type::getInt8PtrTy(M.getContext()),"generated_bitcast_ptr");

                llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function_ptr", llvm::Type::getInt8PtrTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), voidPtr->getType());
                llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 2); // Type Store Def
                llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID((Value *)tmpv));
                Value* fret=ib.CreateCall(fnew, {typeLog, instid, voidPtr});
                // errs()<<"fret: "<<*fret<<"\n";
                Value *origin = ib.CreateBitCast(fret, tmpv->getType());
                // errs()<<"origin: "<<*origin<<"\n";
                U->replaceUsesOfWith((Value*)tmpv, origin);
                


                instrumentedInstSet.insert({strInst, 2});
            }
            else if (!strInst)
            {
                errs() << "ErrorInst" << *tmpv << "\n";
            }
        }
    }

    label += "[";
    const CallStrCxt ctx = var.get_cond().getContexts();
    for (auto n : ctx)
    {

        Instruction *callinst = getCallsite(n > 0 ? n : -n);
        if (!callinst)
        {
            continue;
        }

        std::string clabel = std::to_string(getInstID(callinst));

        label = n > 0 ? label + "+" : label + "-";
        label += clabel;
        label += ";";
    }
    label += "]";
    return label;
}
std::string CSDDA::getCtxVarLabel(CxtVar &var)
{
    std::string label;

    PAGNode *pn = pag->getGNode(var.get_id());
    if (!pn->hasValue())
    {

        // label="NoooInst";
        return label;
    }
    const Value *tmpv = pn->getValue();
    if (const Function *f = llvm::dyn_cast<llvm::Function>(tmpv))
    {

        label = f->getName().str();

        // return label;
    }
    else if (const Instruction *inst = llvm::dyn_cast<llvm::Instruction>(tmpv))
    {

        label = getInstLabel((Instruction *)inst);
        // errs() << "Instruction: " << label.c_str() << "\n";
    }
    else
    {
        // errs() << "ErrorCtxVariable\n";
        std::stringstream ss;
        // llvm::raw_ostream &os = ss;
        // ss << "Label: " << label;
        // std::string result = ss.str();
        // tmpv->toString();
        // errs() << *tmpv << "\n";
        // errs() << var.toString() << "\n";
        // errs()<<tmpv->getName()<<"\n";
        // errs() << *(tmpv->getType())<< "\n";
        // label="ErrorCtx";
        label = std::string(tmpv->getName());
        // return label;
    }

    // errs()<<"fdsvd: "<<label;
    label += "[";
    const CallStrCxt ctx = var.get_cond().getContexts();
    for (auto n : ctx)
    {
        Instruction *callinst = getCallsite(n > 0 ? n : -n);
        if (!callinst)
        {
            continue;
        }
        std::string clabel = getInstLabel(callinst);
        label = n > 0 ? label + "+" : label + "-";
        label += clabel;
        label += ";";
    }
    label += "]";
    return label;
}

void CSDDA::logReturnProtectToFile(Instruction *inst)
{

    if (!inst)
    {
        return;
    }

    if (instrumentedInstSet.find({inst, 4}) != instrumentedInstSet.end())
    {
        return;
    }

    llvm::Module &M = *m;
    if (const ReturnInst *returnInst = llvm::dyn_cast<llvm::ReturnInst>(inst))
    {
        llvm::IRBuilder<> _ib(inst);
        llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
        llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 4);              // Type  return
        llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID(inst)); //
        _ib.CreateCall(fnew, {typeLog, instid});
        instrumentedInstSet.insert({inst, 4});
    }
    else
    {
        return;
    }

    {
        Function *func = inst->getParent()->getParent();

        const SVFFunction *svffun = LLVMModuleSet::getLLVMModuleSet()->getSVFFunction(func);
        // SVFUtil::outs() << "rLabel: \n";
        if (svffun)
        {
            // errs()<<*(inst->getParent()->getParent())<<"csjn\n";
        }
        else
        {
            // svffun->getName();
            return;
        }
        // return;
        PTACallGraphEdge::CallInstSet csSet;
        cdda->getPTACallGraph()->getAllCallSitesInvokingCallee(svffun, csSet);
        int size = csSet.size();
        // errs()<<size<<"  -mj\n";

        if (1)
        {
            // SVFUtil::outs() << "rLabel: \n";
            // root->dump();
            // Instruction *inst = getLdrStrInst(*root);
            *ofs_str_pt << getInstID(inst) << " { ";
            // const ContextDDA::CPtSet &cpts = cdda->getCachedPointsTo(*root);

            // std::string rootLabel = getDpmLabel(dpm);

            // errs() << "root--Label: " << *inst << "\n";
            // *ofs_str_pt << rootLabel << " { ";
            // dpm.getLoc()->getValue()->print(errs());

            for (auto iter : csSet)
            {
                const CallICFGNode *callnode = iter;
                const Instruction *callsite = callnode->getCallSite();
                {
                    if (instrumentedInstSet.find({(Instruction *)callsite, 3}) == instrumentedInstSet.end())
                    {
                        // continue;
                        llvm::IRBuilder<> ib((Instruction *)callsite);
                        llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
                        llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 3); // Type Callsite
                        llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID((Value *)callsite));
                        ib.CreateCall(fnew, {typeLog, instid});
                        instrumentedInstSet.insert({(Instruction *)callsite, 3});
                    }
                }
                // errs()<<"callsite-- "<<*callsite<<"\n";
                std::string label = std::to_string(getInstID((Value *)callsite)) + ":" + std::to_string(getInstID((Value *)callsite));
                {
                    // errs() << "hdfh\n";
                    // errs()<<"fdsvd: "<<label;
                    label += "[";
                    label += "-" + std::to_string(getInstID((Value *)callsite));
                    label += ";";

                    label += "]";
                }
                *ofs_str_pt << label << " ";
            }

            *ofs_str_pt << "}\n";
        }

        //
        //   if (pag->hasValueNode(inst))
        //     {
        //         PAGNode *pagnode = pag->getGNode(pag->getValueNode(inst));
        //         // getAllCallSitesInvokingCallee
        //    }
    }
}
void CSDDA::logToFile(Instruction *root_inst)
{

    CSPL_CxtLocDPItem *root = ((CSPL_ContextDDA *)cdda)->rootDPM;
    {
        std::vector<CSPL_CxtLocDPItem> WorkList;

        WorkList.push_back(*root);
        while (!WorkList.empty())
        {
            CSPL_CxtLocDPItem dpm = WorkList.back();
            WorkList.pop_back();

            if (logged_def_use_DPTItems.find(dpm) == logged_def_use_DPTItems.end())
            {
                if (flattened_dpmTodpmSet.find(dpm) != flattened_dpmTodpmSet.end())
                {

                    logged_def_use_DPTItems.insert(dpm);
                    DPTItemSet flattened = flattened_dpmTodpmSet[dpm];
                    std::string rootLabel = getDpmLabel(dpm);
                    *ofs_du << rootLabel << " { ";

                    for (auto &d : flattened)
                    {

                        std::string instlabel = getDpmLabel(*((CSPL_CxtLocDPItem *)&d));
                        *ofs_du << instlabel << " ";

                        if (logged_def_use_DPTItems.find(d) == logged_def_use_DPTItems.end())
                        {
                            WorkList.push_back(d);
                        }
                    }
                    *ofs_du << "}\n";
                }
            }
        }
        ofs_du->flush();
    }

    {

        std::vector<CSPL_CxtLocDPItem> WorkList;

        WorkList.push_back(*root);
        while (!WorkList.empty())
        {
            CSPL_CxtLocDPItem dpm = WorkList.back();
            WorkList.pop_back();

            if (logged_pt_DPTItems.find(dpm) == logged_pt_DPTItems.end())
            {

                if (flattened_dpmTodpmSet.find(dpm) != flattened_dpmTodpmSet.end())
                {
                    logged_pt_DPTItems.insert(dpm);
                    // DPTItemSet flattened=flattened_dpmTodpmSet[dpm];
                    const ContextDDA::CPtSet &cpts = cdda->getCachedPointsTo(dpm);
                    std::string rootLabel = getDpmLabel(dpm);

                    // errs() << "rootLabel: " << rootLabel << "\n";
                    *ofs_pt << rootLabel << " { ";
                    // dpm.getLoc()->getValue()->print(errs());
                    for (auto &var : cpts)
                    {
                        std::string varlabel = getCtxVarLabel(*((CxtVar *)&var));
                        *ofs_pt << varlabel << " "; // pac_comment
                    }
                    *ofs_pt << "}\n";

                    DPTItemSet flattened = flattened_dpmTodpmSet[dpm];
                    for (auto &d : flattened)
                    {
                        if (logged_pt_DPTItems.find(d) == logged_pt_DPTItems.end())
                        {
                            WorkList.push_back(d);
                        }
                    }
                }
            }
        }
    }
    if (1)
    {

        Instruction *inst_ptr = getLdrStrInst(*root);
        // errs() << "root:" << *inst_ptr << "\n";
        // for (User *U : inst_ptr->users())
        {

            Instruction *inst = dyn_cast<Instruction>((Value *)inst_ptr);
            if (!inst)
            {

                // continue;
            }
            {
                llvm::Module &M = *m;
                {
                    llvm::IRBuilder<> _ib(inst->getNextNode());
                    Value *voidPtr = _ib.CreateBitCast((Value*)inst, Type::getInt8PtrTy(M.getContext()),"generated_bitcast_ptr");
                    llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function_ptr", llvm::Type::getInt8PtrTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), voidPtr->getType());
                    llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 1);              //              // Type Load Use
                    llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID(inst)); // Type Load Use
                    _ib.CreateCall(fnew, {typeLog, instid, voidPtr});
                    instrumentedInstSet.insert({inst, 1});
                }
            }

            *ofs_str_pt
                << getInstID(inst) << " { ";

            const ContextDDA::CPtSet &cpts = cdda->getCachedPointsTo(*root);

            for (auto &var : cpts)
            {
                std::string varlabel = getCtxVarStrCallsiteLabel(*((CxtVar *)&var), root_inst);
                *ofs_str_pt << varlabel << " ";
            }
            *ofs_str_pt << "}\n";
        }
    }
    ofs_du->flush();
    ofs_pt->flush();
    ofs_str_pt->flush();
}

void CSDDA::cs_def_use(Instruction *_inst)
{
    OrderedSet<std::pair<Instruction *, CxtVar>> pointsToSetPairs;

    if (root_to_pointsTo_Map.find(_inst) == root_to_pointsTo_Map.end())
    {
        // return;
        root_to_pointsTo_Map.insert(std::make_pair(_inst, pointsToSetPairs));
    }
    const LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(_inst);
    const StoreInst *storeInst = llvm::dyn_cast<llvm::StoreInst>(_inst);

    const Value *inst;
    if (loadInst)
    {
        inst = loadInst;
        // inst = loadInst->getPointerOperand();
    }
    else if (storeInst)
    {
        return;
        // inst = storeInst->getPointerOperand();
    }
    else
    {
        return;
    }
    if (pag->hasValueNode(inst))
    {
        PAGNode *pagnode = pag->getGNode(pag->getValueNode(inst));
        if (pag->isValidTopLevelPtr(pagnode))
        {
            errs() << "Analyze - PAG node ID: " << pagnode->getId() << ", Inst: " << *inst << "_\n";
            ContextCond cxt;
            CxtVar var(cxt, pagnode->getId());
            const CxtPtSet &cptset = cdda->computeDDAPts(var);
            for (CxtPtSet::iterator piter = cptset.begin(); piter != cptset.end(); ++piter)
            {
                CxtVar ptd = *piter;
                const ContextCond cond = ptd.get_cond();
                const CallStrCxt *const_context = &cond.getContexts();
                CallStrCxt *context = (CallStrCxt *)const_context;

                NodeID nid = ptd.get_id();
                PAGNode *pn = pag->getGNode(nid);
                if (!pn->hasValue())
                {
                    continue;
                }
                // errs() << "errrr" << nid << ", iJJJJ:" << *(pn->getValue()) << " \n";
                const Instruction *strInst = ((CSPL_ContextDDA *)cdda)->getStrInst(ptd);

                if (!strInst)
                {

                    continue;
                }

                root_to_pointsTo_Map[_inst].insert(std::make_pair((Instruction *)strInst, ptd));

                const Value *v = pn->getValue();
                // errs() << "target :" << nid << ", inst:" << *v << " context: ";
                for (auto _csid : *context)
                {
                    int tmp = _csid;
                    if (_csid < 0)
                    {
                        tmp = -tmp;
                    }
                    const CallICFGNode *cifgnode = cdda->getPTACallGraph()->getCallSite(tmp);
                    const Instruction *_i = cifgnode->getCallSite();
                    errs() << "ContextID:" << _csid << ", callsite inst:" << *_i << "; ";
                }
                errs() << "\n";
            }

            // errs() << "PonitsToSet: " << cptset.toString() << "\n";
        }
    }
    // {
    //     PAGNode* node1 = pag->getGNode(pag->getValueNode(V1));
    //     if(pag->isValidTopLevelPtr(node1))
    //         _pta->computeDDAPts(node1->getId());
}

CSDDA::DPTItemSet CSDDA::flatten_DPI_strldr(CSPL_CxtLocDPItem *root)
{
    DPImToDPImSetMap &dPImToDPImSetMap = ((CSPL_ContextDDA *)cdda)->dPImToDPImSetMap;
    DPImToDPImSetMap &aliasSetMap = ((CSPL_ContextDDA *)cdda)->aliasSetMap;
    DPTItemSet flattenedset;

    // if(dPImToDPImSetMap.find(*root)==dPImToDPImSetMap.end()){
    //     return flattenedset;
    // }
    std::vector<CSPL_CxtLocDPItem> WorkList;
    DPTItemSet visited;

    DPTItemSet mainNext;
    if (dPImToDPImSetMap.find(*root) != dPImToDPImSetMap.end())
    {
        mainNext = dPImToDPImSetMap[*root];
    }

    DPTItemSet aNext = ((CSPL_ContextDDA *)cdda)->getAliasNext(*root);
    for (auto &t : aNext)
    {
        mainNext.insert(t);
    }

    for (auto &dpm : mainNext)
    {
        if (isLdrStr(*((CSPL_CxtLocDPItem *)&dpm)))
        {
            flattenedset.insert(dpm);
        }
        else
        {
            if (visited.find(dpm) == visited.end())
            {
                WorkList.push_back(dpm);
            }
        }
        visited.insert(dpm);
    }

    while (!WorkList.empty())
    {

        CSPL_CxtLocDPItem subdpm = WorkList.back();
        WorkList.pop_back();

        DPTItemSet subNext;
        if (dPImToDPImSetMap.find(subdpm) != dPImToDPImSetMap.end())
        {
            subNext = dPImToDPImSetMap[subdpm];
        }
        DPTItemSet asubNext = ((CSPL_ContextDDA *)cdda)->getAliasNext(*root);
        for (auto &t : asubNext)
        {
            subNext.insert(t);
        }

        for (auto &tdpm : subNext)
        {
            if (isLdrStr(*((CSPL_CxtLocDPItem *)&tdpm)))
            {
                flattenedset.insert(tdpm);
            }
            else
            {
                if (visited.find(tdpm) == visited.end())
                {
                    WorkList.push_back(tdpm);
                }
            }
            visited.insert(tdpm);
        }
    }

    return flattenedset;
}

void CSDDA::flatten_allDPI_to_strldr(CSPL_CxtLocDPItem *root)
{
    std::vector<CSPL_CxtLocDPItem> WorkList;
    DPTItemSet visited;

    WorkList.push_back(*root);
    while (!WorkList.empty())
    {
        CSPL_CxtLocDPItem dpm = WorkList.back();
        WorkList.pop_back();
        visited.insert(dpm);
        if (flattened_dpmTodpmSet.find(dpm) == flattened_dpmTodpmSet.end())
        {
            DPTItemSet flattened = flatten_DPI_strldr(&dpm);

            flattened_dpmTodpmSet.insert(std::make_pair(dpm, flattened));
            for (auto &d : flattened)
            {
                if (visited.find(d) == visited.end())
                {
                    WorkList.push_back(d);
                }
            }
        }
    }
}

CSDDA::DPTItemSet CSDDA::pac_root_addrStr(const CSPL_CxtLocDPItem *root)
{
    DPImToDPImSetMap &dPImToDPImSetMap = ((CSPL_ContextDDA *)cdda)->dPImToDPImSetMap;
    DPImToDPImSetMap &aliasSetMap = ((CSPL_ContextDDA *)cdda)->aliasSetMap;
    DPTItemSet flattenedset;

    if (root_to_addr_storeSet.find(*root) != root_to_addr_storeSet.end())
    {
        flattenedset = root_to_addr_storeSet[*root];
        return flattenedset;
    }

    // if(dPImToDPImSetMap.find(*root)==dPImToDPImSetMap.end()){
    //     return flattenedset;
    // }
    // std::vector<CSPL_CxtLocDPItem> WorkList;
    DPTItemSet visited;

    DPTItemSet mainNext;

    if (dPImToDPImSetMap.find(*root) != dPImToDPImSetMap.end())
    {
        mainNext = dPImToDPImSetMap[*root];
    }

    if (dPImToDPImSetMap.find(*root) != dPImToDPImSetMap.end())
    {
        mainNext = dPImToDPImSetMap[*root];
    }

    DPTItemSet aNext = ((CSPL_ContextDDA *)cdda)->getAliasNext(*(CSPL_CxtLocDPItem *)root);
    for (auto &t : aNext)
    {
        mainNext.insert(t);
    }

    // SVFUtil::outs() << ">>>----!!start\n";
    // root->dump();
    for (auto &dpm : mainNext)
    {

        // SVFUtil::outs() << "<<RRRR<<\n";
        // dpm.dump();
    }
    // SVFUtil::outs() << "----\n";
    if (SVFUtil::isa<StoreSVFGNode>(root->getLoc()))
    {

        for (auto &dpm : mainNext)
        {
            const SVFGNode *node = dpm.getLoc();
            if (SVFUtil::isa<AddrSVFGNode>(node))
            {
                flattenedset.insert(*root);

                break;
            }
            else
            {
                DPTItemSet subFlat = pac_root_addrStr(&dpm);
                for (auto &d : subFlat)
                {
                    flattenedset.insert(d);
                }
            }
        }
        root_to_addr_storeSet.insert(std::make_pair(*root, flattenedset));
        // return flattenedset;
    }
    else if (mainNext.size() > 0)
    {
        for (auto &dpm : mainNext)
        {
            const SVFGNode *node = dpm.getLoc();
            DPTItemSet subFlat = pac_root_addrStr(&dpm);
            for (auto &d : subFlat)
            {
                flattenedset.insert(d);
            }
        }
        root_to_addr_storeSet.insert(std::make_pair(*root, flattenedset));
    }
    else
    {
        // Someting wrong
        errs() << "Something wrong\n";
        //  return flattenedset;
    }

    return flattenedset;
}

void CSDDA::pac_root_All_addrStr(const CSPL_CxtLocDPItem *root)
{
    std::vector<CSPL_CxtLocDPItem> WorkList;
    DPTItemSet visited;

    if (root_to_addr_storeSet.find(*root) != root_to_addr_storeSet.end())
    {
        if (root_to_addr_storeSet.find(*root) != root_to_addr_storeSet.end())
        {
            // SVFUtil::outs() << "GOT-ROOT>>>>>>:\n";
            // root->dump();

            DPTItemSet flattened = root_to_addr_storeSet[*root];
            for (auto &d : flattened)
            {
                // SVFUtil::outs() << "-----\n";
                // d.dump();
            }
            // SVFUtil::outs() << "TTTT<<<<\n";
        }

        return;
    }
    // DPTItemSet flattened =
    pac_root_addrStr(root);
    if (root_to_addr_storeSet.find(*root) != root_to_addr_storeSet.end())
    {
        // SVFUtil::outs() << "ROOT>>>>>>:\n";
        // root->dump();

        DPTItemSet flattened = root_to_addr_storeSet[*root];
        for (auto &d : flattened)
        {
            // SVFUtil::outs() << "-----\n";
            // d.dump();
        }
        // SVFUtil::outs() << "TTTT<<<<\n";
    }
}

void CSDDA::printdpm(CSPL_CxtLocDPItem &dpm)
{

    if (printed_DPTItems.find(dpm) != printed_DPTItems.end())
    {
        return;
    }
    else
    {
        printed_DPTItems.insert(dpm);
        // CSPL_CxtLocDPItem tdpm=dpm;
        // tdpm.cspl_id=76;
        //  SVFUtil::outs() <<"etsting: "<<dpm.cspl_id<<"   "<<(printed_DPTItems.find(dpm)!=printed_DPTItems.end())<<"  :"<<tdpm.cspl_id<<" "<<(printed_DPTItems.find(tdpm)!=printed_DPTItems.end())<<"\n";
    }
    // SVFUtil::outs() <<"whattttt\n";

    if (flattened_dpmTodpmSet.find(dpm) != flattened_dpmTodpmSet.end())
    {
        DPTItemSet dPTItemSet = flattened_dpmTodpmSet[dpm];
        // SVFUtil::outs() << "For node: " << dpm.getLoc()->getId() << "," << dpm.getCondVar().get_id() << dpm.getCond().toString() << " fullinst  :" << dpm.getLoc()->toString() << " :\n";
        for (auto newdpm : dPTItemSet)
        {
            // SVFUtil::outs() << "sub: " << newdpm.getLoc()->getId() << "," << newdpm.getCondVar().get_id() << newdpm.getCond().toString() << " ;";
        }
        // SVFUtil::outs() << "\n";
        for (auto newdpm : dPTItemSet)
        {

            printdpm(newdpm);
        }
    }
}

void CSDDA::pac_addr_instrument(Instruction *root_inst)
{
    return;
    Instruction *inst = root_inst;

    if (!inst)
    {
        return;
    }

    if (instrumentedInstSet.find({inst, 1}) != instrumentedInstSet.end())
    {
        return;
    }

    llvm::Module &M = *m;
    if (const LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(inst))
    {
        llvm::IRBuilder<> _ib(inst);
        llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
        llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 1);              //       // Type Load Use
        llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID(inst)); // Type Load Use
        _ib.CreateCall(fnew, {typeLog, instid});
        instrumentedInstSet.insert({inst, 1});
    }
    else
    {
        errs() << "NotLoad\n";
        return;
    }

    {
        for (auto &item : root_to_pointsTo_Map[root_inst])
        {

            Instruction *strInst = item.first;
            errs() << "--AAA-" << *strInst << "\n";
            if (!strInst)
            {
                continue;
            }

            if (instrumentedInstSet.find({strInst, 2}) == instrumentedInstSet.end())
            {
                // continue;
                llvm::IRBuilder<> ib(strInst);
                llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
                llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 2); // Type Store Def
                llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID(strInst));
                ib.CreateCall(fnew, {typeLog, instid});
                instrumentedInstSet.insert({strInst, 2});
            }

            const CxtVar &var = item.second;
            const CallStrCxt *const_context = &(var.get_cond().getContexts());
            CallStrCxt *context = (CallStrCxt *)const_context;
            for (auto _csid : *context)
            {
                int tmp = _csid;
                if (_csid < 0)
                {
                    tmp = -tmp;
                }
                const CallICFGNode *cifgnode = cdda->getPTACallGraph()->getCallSite(tmp);
                if (!cifgnode)
                {
                    continue;
                }
                Instruction *_i = (Instruction *)cifgnode->getCallSite();
                if (!_i)
                {
                    continue;
                }
                if (instrumentedInstSet.find({_i, 3}) == instrumentedInstSet.end())
                {
                    // continue;
                    llvm::IRBuilder<> ib(_i);
                    llvm::FunctionCallee fnew = M.getOrInsertFunction("pac_log_function", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
                    llvm::Value *typeLog = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 3); // Type Callsite
                    llvm::Value *instid = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), getInstID(_i));
                    ib.CreateCall(fnew, {typeLog, instid});
                    instrumentedInstSet.insert({_i, 3});
                }
            }
        }
    }
}

void CSDDA::instrument(CSPL_CxtLocDPItem &root)
{
    Instruction *inst = getLdrStrInst(root);

    if (!inst)
    {
        return;
    }

    if (instrumentedDpmSet.find(root) != instrumentedDpmSet.end())
    {
        return;
    }
    else
    {
        instrumentedDpmSet.insert(root);
    }

    llvm::IRBuilder<> ib(inst);
    llvm::Module &M = *m;
    if (const LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(inst))
    {
        if (instrumentedSet.find(inst) == instrumentedSet.end())
        {
            llvm::FunctionCallee fnew = M.getOrInsertFunction("pgi_load_check", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
            llvm::Value *arg = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 1);
            ib.CreateCall(fnew, {arg});
            instrumentedSet.insert(inst);
        }
    }
    else if (const StoreInst *storeInst = llvm::dyn_cast<llvm::StoreInst>(inst))
    {
        if (instrumentedSet.find(inst) == instrumentedSet.end())
        {
            llvm::FunctionCallee fnew = M.getOrInsertFunction("pgi_store_check", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
            llvm::Value *arg = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 1);
            ib.CreateCall(fnew, {arg});
            instrumentedSet.insert(inst);
        }
    }

    CallStrCxt &ctx = root.getCond().getContexts();
    for (auto csId : ctx)
    {
        int abscsid = csId > 0 ? csId : -csId;
        Instruction *csInst = getCallsite(abscsid);
        if (instrumentedSet.find(csInst) == instrumentedSet.end())
        {
            llvm::IRBuilder<> ib(csInst);
            llvm::FunctionCallee fnew = M.getOrInsertFunction("pgi_callsite", llvm::Type::getVoidTy(M.getContext()), llvm::IntegerType::getInt32Ty(M.getContext()));
            llvm::Value *arg = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(M.getContext()), 1);
            ib.CreateCall(fnew, {arg});
            instrumentedSet.insert(csInst);
        }
    }

    if (flattened_dpmTodpmSet.find(root) != flattened_dpmTodpmSet.end())
    {
        DPTItemSet &upper = flattened_dpmTodpmSet[root];
        for (auto dpm : upper)
        {
            instrument(dpm);
        }
    }
}
