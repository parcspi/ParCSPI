//===- ContextDDA.cpp -- Context-sensitive demand-driven analysis-------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * ContextDDA.cpp
 *
 *  Created on: Aug 17, 2014
 *      Author: Yulei Sui
 */

#include "Util/Options.h"
#include "DDA/ContextDDA.h"
#include "DDA/FlowDDA.h"
#include "DDA/DDAClient.h"
#include "MemoryModel/PointsTo.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Constructor
 */
ContextDDA::ContextDDA(SVFIR* _pag,  DDAClient* client)
    : CondPTAImpl<ContextCond>(_pag, PointerAnalysis::Cxt_DDA),DDAVFSolver<CxtVar,CxtPtSet,CxtLocDPItem>(),
      _client(client)
{
    flowDDA = new FlowDDA(_pag, client);
}

/*!
 * Destructor
 */
ContextDDA::~ContextDDA()
{
    if(flowDDA)
        delete flowDDA;
    flowDDA = nullptr;
}

/*!
 * Analysis initialization
 */
void ContextDDA::initialize()
{
    CondPTAImpl<ContextCond>::initialize();
    buildSVFG(pag);
    setCallGraph(getPTACallGraph());
    setCallGraphSCC(getCallGraphSCC());
    stat = setDDAStat(new DDAStat(this));
    flowDDA->initialize();
}

/*!
 * Compute points-to set for a context-sensitive pointer
 */
const CxtPtSet& ContextDDA::computeDDAPts(const CxtVar& var)
{

    resetQuery();
    LocDPItem::setMaxBudget(Options::CxtBudget);

    NodeID id = var.get_id();
    PAGNode* node = getPAG()->getGNode(id);
    CxtLocDPItem dpm = getDPIm(var, getDefSVFGNode(node));

    // start DDA analysis
    DOTIMESTAT(double start = DDAStat::getClk(true));
    const CxtPtSet& cpts = findPT(dpm);
    DOTIMESTAT(ddaStat->_AnaTimePerQuery = DDAStat::getClk(true) - start);
    DOTIMESTAT(ddaStat->_TotalTimeOfQueries += ddaStat->_AnaTimePerQuery);

    if(isOutOfBudgetQuery() == false)
        unionPts(var,cpts);
    else
        handleOutOfBudgetDpm(dpm);

    if (this->printStat())
        DOSTAT(stat->performStatPerQuery(id));
    DBOUT(DGENERAL, stat->printStatPerQuery(id,getBVPointsTo(getPts(var))));
    return this->getPts(var);
}

/*!
 *  Compute points-to set for an unconditional pointer
 */
void ContextDDA::computeDDAPts(NodeID id)
{
    ContextCond cxt;
    CxtVar var(cxt, id);
    computeDDAPts(var);
}

/*!
 * Handle out-of-budget dpm
 */
void ContextDDA::handleOutOfBudgetDpm(const CxtLocDPItem& dpm)
{

    DBOUT(DGENERAL,outs() << "~~~Out of budget query, downgrade to flow sensitive analysis \n");
    flowDDA->computeDDAPts(dpm.getCurNodeID());
    const PointsTo& flowPts = flowDDA->getPts(dpm.getCurNodeID());
    CxtPtSet cxtPts;
    for(PointsTo::iterator it = flowPts.begin(), eit = flowPts.end(); it!=eit; ++it)
    {
        ContextCond cxt;
        CxtVar var(cxt, *it);
        cxtPts.set(var);
    }
    updateCachedPointsTo(dpm,cxtPts);
    unionPts(dpm.getCondVar(),cxtPts);
    addOutOfBudgetDpm(dpm);
}

/*!
 * context conditions of local(not in recursion)  and global variables are compatible
 */
bool ContextDDA::isCondCompatible(const ContextCond& cxt1, const ContextCond& cxt2, bool singleton) const
{
    if(singleton)
        return true;

    int i = cxt1.cxtSize() - 1;
    int j = cxt2.cxtSize() - 1;
    for(; i >= 0 && j>=0; i--, j--)
    {
        if(cxt1[i] != cxt2[j])
            return false;
    }
    return true;
}

/*!
 * Generate field objects for structs
 */
CxtPtSet CSPL_ContextDDA::processGepPts(const GepSVFGNode* gep, const CxtPtSet& srcPts)
{
    CxtPtSet tmpDstPts;
    for (CxtPtSet::iterator piter = srcPts.begin(); piter != srcPts.end(); ++piter)
    {

        CxtVar ptd = *piter;
        if (isBlkObjOrConstantObj(ptd.get_id()))
            {
                tmpDstPts.set(ptd);

                if (!cspl_previousGepdpm&& addrDpM_to_CVar_map.find(*cspl_previousGepdpm) == addrDpM_to_CVar_map.end())
                {
                    
                    addrDpM_to_CVar_map.insert(std::make_pair(*cspl_previousGepdpm, ptd));
                }
            }
        else
        {
            const GepStmt* gepStmt = SVFUtil::cast<GepStmt>(gep->getPAGEdge());
            if (gepStmt->isVariantFieldGep())
            {
                setObjFieldInsensitive(ptd.get_id());
                CxtVar var(ptd.get_cond(),getFIObjVar(ptd.get_id()));
                tmpDstPts.set(var);
                if (!cspl_previousGepdpm&& addrDpM_to_CVar_map.find(*cspl_previousGepdpm) == addrDpM_to_CVar_map.end())
                {
                    
                    addrDpM_to_CVar_map.insert(std::make_pair(*cspl_previousGepdpm, ptd));
                }
            }
            else
            {
                CxtVar var(ptd.get_cond(),getGepObjVar(ptd.get_id(),gepStmt->getLocationSet()));
                tmpDstPts.set(var);
                if (!cspl_previousGepdpm&& addrDpM_to_CVar_map.find(*cspl_previousGepdpm) == addrDpM_to_CVar_map.end())
                {
                    
                    addrDpM_to_CVar_map.insert(std::make_pair(*cspl_previousGepdpm, ptd));
                }
            }
        }
    }

    DBOUT(DDDA, outs() << "\t return created gep objs ");
    DBOUT(DDDA, outs() << srcPts.toString());
    DBOUT(DDDA, outs() << " --> ");
    DBOUT(DDDA, outs() << tmpDstPts.toString());
    DBOUT(DDDA, outs() << "\n");
    return tmpDstPts;
}

CxtPtSet ContextDDA::processGepPts(const GepSVFGNode* gep, const CxtPtSet& srcPts)
{
    CxtPtSet tmpDstPts;
    for (CxtPtSet::iterator piter = srcPts.begin(); piter != srcPts.end(); ++piter)
    {

        CxtVar ptd = *piter;
        if (isBlkObjOrConstantObj(ptd.get_id()))
            tmpDstPts.set(ptd);
        else
        {
            const GepStmt* gepStmt = SVFUtil::cast<GepStmt>(gep->getPAGEdge());
            if (gepStmt->isVariantFieldGep())
            {
                setObjFieldInsensitive(ptd.get_id());
                CxtVar var(ptd.get_cond(),getFIObjVar(ptd.get_id()));
                tmpDstPts.set(var);
            }
            else
            {
                CxtVar var(ptd.get_cond(),getGepObjVar(ptd.get_id(),gepStmt->getLocationSet()));
                tmpDstPts.set(var);
            }
        }
    }

    DBOUT(DDDA, outs() << "\t return created gep objs ");
    DBOUT(DDDA, outs() << srcPts.toString());
    DBOUT(DDDA, outs() << " --> ");
    DBOUT(DDDA, outs() << tmpDstPts.toString());
    DBOUT(DDDA, outs() << "\n");
    return tmpDstPts;
}

bool ContextDDA::testIndCallReachability(CxtLocDPItem& dpm, const SVFFunction* callee, const CallICFGNode* cs)
{
    if(getPAG()->isIndirectCallSites(cs))
    {
        NodeID id = getPAG()->getFunPtr(cs);
        PAGNode* node = getPAG()->getGNode(id);
        CxtVar funptrVar(dpm.getCondVar().get_cond(), id);
        CxtLocDPItem funptrDpm = getDPIm(funptrVar,getDefSVFGNode(node));
        PointsTo pts = getBVPointsTo(findPT(funptrDpm));
        if(pts.test(getPAG()->getObjectNode(callee->getLLVMFun())))
            return true;
        else
            return false;
    }
    return true;
}

/*!
 * get callsite id from call, return 0 if it is a spurious call edge
 * translate the callsite id from pre-computed callgraph on SVFG to the one on current callgraph
 */
CallSiteID ContextDDA::getCSIDAtCall(CxtLocDPItem&, const SVFGEdge* edge)
{

    CallSiteID svfg_csId = 0;
    if (const CallDirSVFGEdge* callEdge = SVFUtil::dyn_cast<CallDirSVFGEdge>(edge))
        svfg_csId = callEdge->getCallSiteId();
    else
        svfg_csId = SVFUtil::cast<CallIndSVFGEdge>(edge)->getCallSiteId();

    const CallICFGNode* cbn = getSVFG()->getCallSite(svfg_csId);
    const SVFFunction* callee = edge->getDstNode()->getFun();

    if(getPTACallGraph()->hasCallSiteID(cbn,callee))
    {
        return getPTACallGraph()->getCallSiteID(cbn,callee);
    }

    return 0;
}

/*!
 * get callsite id from return, return 0 if it is a spurious return edge
 * translate the callsite id from pre-computed callgraph on SVFG to the one on current callgraph
 */
CallSiteID ContextDDA::getCSIDAtRet(CxtLocDPItem&, const SVFGEdge* edge)
{

    CallSiteID svfg_csId = 0;
    if (const RetDirSVFGEdge* retEdge = SVFUtil::dyn_cast<RetDirSVFGEdge>(edge))
        svfg_csId = retEdge->getCallSiteId();
    else
        svfg_csId = SVFUtil::cast<RetIndSVFGEdge>(edge)->getCallSiteId();

    const CallICFGNode* cbn = getSVFG()->getCallSite(svfg_csId);
    const SVFFunction* callee = edge->getSrcNode()->getFun();

    if(getPTACallGraph()->hasCallSiteID(cbn,callee))
    {
        return getPTACallGraph()->getCallSiteID(cbn,callee);
    }

    return 0;
}


/// Handle conditions during backward traversing
bool ContextDDA::handleBKCondition(CxtLocDPItem& dpm, const SVFGEdge* edge)
{
    _client->handleStatement(edge->getSrcNode(), dpm.getCurNodeID());

    if (edge->isCallVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtCall(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {
                DBOUT(DDDA,outs() << "\t\t call edge " << getPTACallGraph()->getCallerOfCallSite(csId)->getName() <<
                      "=>" << getPTACallGraph()->getCalleeOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
            }
            else
            {
                if (dpm.matchContext(csId) == false)
                {
                    DBOUT(DDDA,	outs() << "\t\t context not match, edge "
                          << edge->getDstID() << " --| " << edge->getSrcID() << " \t");
                    DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    return false;
                }

                DBOUT(DDDA, outs() << "\t\t match contexts ");
                DBOUT(DDDA, dumpContexts(dpm.getCond()));
            }
        }
    }

    else if (edge->isRetVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtRet(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {
                DBOUT(DDDA,outs() << "\t\t return edge " << getPTACallGraph()->getCalleeOfCallSite(csId)->getName() <<
                      "=>" << getPTACallGraph()->getCallerOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
            }
            else
            {
                /// TODO: When this call site id is contained in current call string, we may find a recursion. Try
                ///       to solve this later.
                if (dpm.getCond().containCallStr(csId))
                {
                    outOfBudgetQuery = true;
                    SVFUtil::writeWrnMsg("Call site ID is contained in call string. Is this a recursion?");
                    return false;
                }
                else
                {
                    assert(dpm.getCond().containCallStr(csId) ==false && "contain visited call string ??");
                    if(dpm.pushContext(csId))
                    {
                        DBOUT(DDDA, outs() << "\t\t push context ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                    else
                    {
                        DBOUT(DDDA, outs() << "\t\t context is full ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                }
            }
        }
    }

    return true;
}




/// we exclude concrete heap given the following conditions:
/// (1) concrete calling context (not involved in recursion and not exceed the maximum context limit)
/// (2) not inside loop
bool ContextDDA::isHeapCondMemObj(const CxtVar& var, const StoreSVFGNode*)
{
    const MemObj* mem = _pag->getObject(getPtrNodeID(var));
    assert(mem && "memory object is null??");
    if(mem->isHeap())
    {
        if (!mem->getValue())
        {
            PAGNode *pnode = _pag->getGNode(getPtrNodeID(var));
            if(GepObjVar* gepobj = SVFUtil::dyn_cast<GepObjVar>(pnode))
            {
                assert(SVFUtil::isa<DummyObjVar>(_pag->getGNode(gepobj->getBaseNode())) && "emtpy refVal in a gep object whose base is a non-dummy object");
            }
            else
            {
                assert((SVFUtil::isa<DummyObjVar>(pnode) || SVFUtil::isa<DummyValVar>(pnode)) && "empty refVal in non-dummy object");
            }
            return true;
        }
        else if(const Instruction* mallocSite = SVFUtil::dyn_cast<Instruction>(mem->getValue()))
        {
            const Function* fun = mallocSite->getFunction();
            const SVFFunction* svfFun = LLVMModuleSet::getLLVMModuleSet()->getSVFFunction(fun);
            if(_ander->isInRecursion(svfFun))
                return true;
            if(var.get_cond().isConcreteCxt() == false)
                return true;
            if(_pag->getICFG()->isInLoop(mallocSite))
                return true;
        }
    }
    return false;
}


bool CSPL_ContextDDA::handleBKCondition(CxtLocDPItem& dpm, const SVFGEdge* edge)
{   
    // SVFUtil::outs() <<"AXWDE\n";
    _client->handleStatement(edge->getSrcNode(), dpm.getCurNodeID()); //just a callback?

    if (edge->isCallVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtCall(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {       
                SVFUtil::outs() <<"recurrr_call\n";
                DBOUT(DDDA,outs() << "\t\t call edge " << getPTACallGraph()->getCallerOfCallSite(csId)->getName() <<
                      "=>" << getPTACallGraph()->getCalleeOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
                SVFUtil::outs() <<"end ---l\n";
            }
            else
            {
                if (popContextDpm(dpm,csId) == false)
                {
                    DBOUT(DDDA,	outs() << "\t\t context not match, edge "
                          << edge->getDstID() << " --| " << edge->getSrcID() << " \t");
                    DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    return false;
                }
                // if (dpm.matchContext(csId) == false)
                // {
                //     DBOUT(DDDA,	outs() << "\t\t context not match, edge "
                //           << edge->getDstID() << " --| " << edge->getSrcID() << " \t");
                //     DBOUT(DDDA, dumpContexts(dpm.getCond()));
                //     return false;
                // }

                DBOUT(DDDA, outs() << "\t\t match contexts ");
                DBOUT(DDDA, dumpContexts(dpm.getCond()));
            }
        }
    }

    else if (edge->isRetVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtRet(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {    SVFUtil::outs() <<"recurrr_ret\n";
                DBOUT(DDDA,outs() << "\t\t return edge " << getPTACallGraph()->getCalleeOfCallSite(csId)->getName() <<
                      "=>" << getPTACallGraph()->getCallerOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
                 SVFUtil::outs() <<"end ---l\n";
            }
            else
            {
                /// TODO: When this call site id is contained in current call string, we may find a recursion. Try
                ///       to solve this later.
                if (dpm.getCond().containCallStr(csId))
                {
                    outOfBudgetQuery = true;
                    SVFUtil::writeWrnMsg("Call site ID is contained in call string. Is this a recursion?");
                    return false;
                }
                else
                {
                    // assert(dpm.getCond().containCallStr(csId) ==false && "contain visited call string ??");
                    if(pushContextDpm(dpm,csId))
                    {
                        DBOUT(DDDA, outs() << "\t\t push context ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                    else
                    {
                        DBOUT(DDDA, outs() << "\t\t context is full ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                  
                }
            }
        }
    }
    // SVFUtil::outs() <<"sdcdwcewcew\n";

    return true;
}

bool CSPL_ContextDDA::pushContextDpm(CxtLocDPItem& dpm,CallSiteID csId){
    const ContextCond* const_ccond=&dpm.getCond();
        ContextCond* ccond=(ContextCond*)const_ccond;
        const CallStrCxt* const_context=&ccond->getContexts();
            CallStrCxt* context=(CallStrCxt*)const_context;
         if(context->size() < ccond->maximumCxtLen||fullcontext)
        {

            // CallStrCxt* actual_context=&CxtLocDPItem::actual_contextMap[dpm.cspl_id].getContexts();
            CallStrCxt* actual_context=&dpm.actual_context.getContexts();
            if(actual_context->empty()){
                actual_context->push_back(csId);
            }else{
                if(actual_context->back()>0){
                    actual_context->push_back(csId);
                }else if(actual_context->back()==-csId){
                    actual_context->pop_back();
                }else{
                    actual_context->push_back(csId);
                }
            }

            if(!context->empty() && context->back()<0){
                    if(csId==-context->back()){
                        context->pop_back();
                        // CxtLocDPItem::actual_contextMap[dpm.cspl_id].pop_back();
                    }else{
                        int tmp=csId;
                        if(context_length(context,1)<ccond->maximumCxtLen){
                            context->push_back(csId);
                            // CxtLocDPItem::actual_contextMap[dpm.cspl_id].push_back(csId);
                        }else{

                            ccond->setNonConcreteCxt();
                            context->push_back(csId);
                            int idex=get_first(context,1);
                            context->erase(context->begin()+idex);
                            // CxtLocDPItem::actual_contextMap[dpm.cspl_id].push_back(csId);
                        }
                        
                    }
            }else{
                int tmp=csId;
                        if(context_length(context,1)<ccond->maximumCxtLen){
                            context->push_back(csId);
                            // CxtLocDPItem::actual_contextMap[dpm.cspl_id].push_back(csId);
                        }else{

                            ccond->setNonConcreteCxt();
                            context->push_back(csId);
                            int idex=get_first(context,1);
                            context->erase(context->begin()+idex);
                            // CxtLocDPItem::actual_contextMap[dpm.cspl_id].push_back(csId);
                        }
            }
            


            if(context->size() > ccond->maximumCxt)
                ccond->maximumCxt = context->size();
            return true;
        }
        else   /// handle out of context limit case
        {
            if(!context->empty())
            {
                ccond->setNonConcreteCxt();
                
                if(!context->empty() && context->back()<0){
                    if(csId==-context->back()){
                        context->pop_back();
                    }else{
                        context->erase(context->begin());
                        context->push_back(csId);
                    }
                }else{
                    context->erase(context->begin());
                    context->push_back(csId);
                }

                // context->push_back(ctx);
            }
            return false;
        }
}

bool CSPL_ContextDDA::popContextDpm(CxtLocDPItem& dpm,CallSiteID csId){
    const ContextCond* const_ccond=&dpm.getCond();
        ContextCond* ccond=(ContextCond*)const_ccond;
        const CallStrCxt* const_context=&ccond->getContexts();
            CallStrCxt* context=(CallStrCxt*)const_context;

        


        if(context->empty()){
            int tmp=-csId;
            // context->push_back(tmp);
            context->push_back(tmp);
            
        }else{
            if(context->back() == csId){
                context->pop_back();
                // return true;
            }else if(context->back()<0){
                int tmp=-csId;
             if(context_length(context,-1)<ccond->maximumCxtLen){
                context->push_back(tmp);
            }else{
                ccond->setNonConcreteCxt();
                context->push_back(tmp);
                            int idex=get_first(context,-1);
                            context->erase(context->begin()+idex);
            }
            }else{
                context->push_back(-csId);
                // return false;
            }
        }
        //  CallStrCxt* actual_context=&CxtLocDPItem::actual_contextMap[dpm.cspl_id].getContexts();
        CallStrCxt* actual_context=&dpm.actual_context.getContexts();
            if(actual_context->empty()){
                actual_context->push_back(-csId);
                return true;
            }else{
                if(actual_context->back()>0){
                    if(actual_context->back()!=csId){
                        return false;
                    }else{
                         actual_context->pop_back();   
                         return true;
                    }
                    
                }else {
                    actual_context->push_back(-csId);
                    return true;
                }
            }
     


}

int CSPL_ContextDDA::context_length(CallStrCxt* context,int csId){
    // return context->size()

    int count=0;
    for(auto c: *context){
        if (c*csId>0){
            count++;
        }
    }
    return count;
    // ccond->maximumCxtLen
}

  int CSPL_ContextDDA::get_first(CallStrCxt* context,int sign){
        //   int count=0;
          if(sign>0){
            for(int i=0;i<context->size();i++){
                if((*context)[i]>0){
                    return i;
                }
            }
          }

            if(sign<0){
            for(int i=context->size()-1;i>=0;i--){
                if((*context)[i]<0){
                    return i;
                }
            }
          }
          
    // for(auto c: *context){
    //     if (c*csId>0){
    //         return count;
    //     }
    //     count++;
    // }
    return -1;
  }

const CxtPtSet& CSPL_ContextDDA::computeDDAPts(const CxtVar& var){
        resetQuery();
    LocDPItem::setMaxBudget(Options::CxtBudget);

    NodeID id = var.get_id();
    PAGNode* node = getPAG()->getGNode(id);
    
  
    rootDPM=new CxtLocDPItem(getDPIm(var, getDefSVFGNode(node)));
    CxtLocDPItem& dpm=*rootDPM;
    // start DDA analysis
    DOTIMESTAT(double start = DDAStat::getClk(true));
    const CxtPtSet& cpts = findPT(dpm);
    DOTIMESTAT(ddaStat->_AnaTimePerQuery = DDAStat::getClk(true) - start);
    DOTIMESTAT(ddaStat->_TotalTimeOfQueries += ddaStat->_AnaTimePerQuery);



    if(isOutOfBudgetQuery() == false)
        unionPts(var,cpts);
    else
        handleOutOfBudgetDpm(dpm);

    if (this->printStat())
        DOSTAT(stat->performStatPerQuery(id));
    DBOUT(DGENERAL, stat->printStatPerQuery(id,getBVPointsTo(getPts(var))));
    // PImToDPImSetMap&  dPImToDPImSetMap=cdda->dPImToDPImSetMap;
   printed_DPTItems.clear();
//    printdpm(dpm);
    // if(root_to_str_pt_pairMap
    const CxtPtSet& resultPt=this->getPts(var);
 if(0){
    for (CxtPtSet::iterator piter = resultPt.begin(); piter != resultPt.end(); ++piter)
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
                const CxtLocDPItem* strDPI= getOldDPI(ptd);

            }
  }
    // {
    //       for(auto& item:addrDpM_to_CVar_map){
    //                     if(item.second==var){
    //                         errs()<<"Found: "<<item.first.get_id()<<"\n";
    //                    }
    //     }
    // }
    return this->getPts(var);
   }
    void CSPL_ContextDDA::printdpm(CSPL_CxtLocDPItem dpm){

        if(printed_DPTItems.find(dpm)!=printed_DPTItems.end()){
            return;
        }else{
            printed_DPTItems.insert(dpm);
            // CSPL_CxtLocDPItem tdpm=dpm;
            // tdpm.cspl_id=76;
            //  SVFUtil::outs() <<"etsting: "<<dpm.cspl_id<<"   "<<(printed_DPTItems.find(dpm)!=printed_DPTItems.end())<<"  :"<<tdpm.cspl_id<<" "<<(printed_DPTItems.find(tdpm)!=printed_DPTItems.end())<<"\n";
        }

             if(dPImToDPImSetMap.find(dpm)!=dPImToDPImSetMap.end()){
        DPTItemSet dPTItemSet=dPImToDPImSetMap[dpm];
        DPTItemSet nexts=getAliasNext(dpm);
        for(auto&t:nexts){
            dPTItemSet.insert(t);
        }
   
        SVFUtil::outs() <<"Node Print: "<<dpm.getLoc()->getId()<<","<<dpm.getCondVar().get_id()<<dpm.getCond().toString()<<", Inst Print:"<<dpm.getLoc()->toString()<<" :\n";
        for(auto newdpm:dPTItemSet){
              SVFUtil::outs() <<"Sub Node Print: "<<newdpm.getLoc()->getId()<<", Cond Var"<<newdpm.getCondVar().get_id()<<", ContextString: "<<newdpm.getCond().toString()<<" ;";
          
        }
         SVFUtil::outs() <<"\n";
        for(auto newdpm:dPTItemSet){
            
            printdpm(newdpm);
        }

    }
    }