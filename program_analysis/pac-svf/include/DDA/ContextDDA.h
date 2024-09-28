//===- ContextDDA.h -- Context-sensitive demand-driven analysis-------------//
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
 * ContextDDA.h
 *
 *  Created on: Aug 17, 2014
 *      Author: Yulei Sui
 *
 * The implementation is based on
 * (1) Yulei Sui and Jingling Xue. "On-Demand Strong Update Analysis via Value-Flow Refinement".
 * ACM SIGSOFT International Symposium on the Foundation of Software Engineering (FSE'16)
 *
 * (2) Yulei Sui and Jingling Xue. "Value-Flow-Based Demand-Driven Pointer Analysis for C and C++".
 * IEEE Transactions on Software Engineering (TSE'18)
 */

#ifndef ContextDDA_H_
#define ContextDDA_H_

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

// using namespace llvm;

#include "MemoryModel/PointerAnalysisImpl.h"
#include "DDA/DDAVFSolver.h"
#include "Util/DPItem.h"

namespace SVF
{

    class FlowDDA;
    class DDAClient;

#define CSPL

#ifndef CSPL
    typedef CxtStmtDPItem<SVFGNode> CxtLocDPItem;
#else
    typedef CSPL_CxtStmtDPItem<SVFGNode> CxtLocDPItem;

#endif
    typedef CSPL_CxtStmtDPItem<SVFGNode> CSPL_CxtLocDPItem;

    /*!
     * Context-, Flow- Sensitive Demand-driven Analysis
     */
    class ContextDDA : public CondPTAImpl<ContextCond>, public DDAVFSolver<CxtVar, CxtPtSet, CxtLocDPItem>
    {

    public:
        /// Constructor
        ContextDDA(SVFIR *_pag, DDAClient *client);

        /// Destructor
        virtual ~ContextDDA();

        /// Initialization of the analysis
        virtual void initialize() override;

        /// Finalize analysis
        virtual inline void finalize() override
        {
            CondPTAImpl<ContextCond>::finalize();
        }

        /// dummy analyze method
        virtual void analyze() override {}

        /// Compute points-to set for an unconditional pointer
        virtual void computeDDAPts(NodeID id) override;

        /// Compute points-to set for a context-sensitive pointer
        virtual const CxtPtSet &computeDDAPts(const CxtVar &cxtVar);

        /// Handle out-of-budget dpm
        void handleOutOfBudgetDpm(const CxtLocDPItem &dpm);

        /// Override parent method
        virtual CxtPtSet getConservativeCPts(const CxtLocDPItem &dpm) override
        {
            const PointsTo &pts = getAndersenAnalysis()->getPts(dpm.getCurNodeID());
            CxtPtSet tmpCPts;
            ContextCond cxt;
            for (PointsTo::iterator piter = pts.begin(); piter != pts.end(); ++piter)
            {
                CxtVar var(cxt, *piter);
                tmpCPts.set(var);
            }
            return tmpCPts;
        }

        /// Override parent method
        virtual inline NodeID getPtrNodeID(const CxtVar &var) const override
        {
            return var.get_id();
        }
        /// Handle condition for context or path analysis (backward analysis)
        virtual bool handleBKCondition(CxtLocDPItem &dpm, const SVFGEdge *edge) override;

        /// we exclude concrete heap given the following conditions:
        /// (1) concrete calling context (not involved in recursion and not exceed the maximum context limit)
        /// (2) not inside loop
        virtual bool isHeapCondMemObj(const CxtVar &var, const StoreSVFGNode *store) override;

        /// refine indirect call edge
        bool testIndCallReachability(CxtLocDPItem &dpm, const SVFFunction *callee, const CallICFGNode *cs);

        /// get callsite id from call, return 0 if it is a spurious call edge
        CallSiteID getCSIDAtCall(CxtLocDPItem &dpm, const SVFGEdge *edge);

        /// get callsite id from return, return 0 if it is a spurious return edge
        CallSiteID getCSIDAtRet(CxtLocDPItem &dpm, const SVFGEdge *edge);

        /// Pop recursive callsites
        inline virtual void popRecursiveCallSites(CxtLocDPItem &dpm)
        {
            ContextCond &cxtCond = dpm.getCond();
            cxtCond.setNonConcreteCxt();
            CallStrCxt &cxt = cxtCond.getContexts();
            while (!cxt.empty() && isEdgeInRecursion(cxt.back()))
            {
                cxt.pop_back();
            }
        }
        /// Whether call/return inside recursion
        inline virtual bool isEdgeInRecursion(CallSiteID csId)
        {
            const SVFFunction *caller = getPTACallGraph()->getCallerOfCallSite(csId);
            const SVFFunction *callee = getPTACallGraph()->getCalleeOfCallSite(csId);
            return inSameCallGraphSCC(caller, callee);
        }
        /// Update call graph.
        //@{
        virtual void updateCallGraphAndSVFG(const CxtLocDPItem &dpm, const CallICFGNode *cs, SVFGEdgeSet &svfgEdges) override
        {
            CallEdgeMap newEdges;
            resolveIndCalls(cs, getBVPointsTo(getCachedPointsTo(dpm)), newEdges);
            for (CallEdgeMap::const_iterator iter = newEdges.begin(), eiter = newEdges.end(); iter != eiter; iter++)
            {
                const CallICFGNode *newcs = iter->first;
                const FunctionSet &functions = iter->second;
                for (FunctionSet::const_iterator func_iter = functions.begin(); func_iter != functions.end(); func_iter++)
                {
                    const SVFFunction *func = *func_iter;
                    getSVFG()->connectCallerAndCallee(newcs, func, svfgEdges);
                }
            }
        }
        //@}

        /// Return TRUE if this edge is inside a SVFG SCC, i.e., src node and dst node are in the same SCC on the SVFG.
        inline bool edgeInCallGraphSCC(const SVFGEdge *edge)
        {
            const SVFFunction *srcfun = edge->getSrcNode()->getFun();
            const SVFFunction *dstfun = edge->getDstNode()->getFun();

            if (srcfun && dstfun)
                return inSameCallGraphSCC(srcfun, dstfun);

            assert(edge->isRetVFGEdge() == false && "should not be an inter-procedural return edge");

            return false;
        }

        /// processGep node
        virtual CxtPtSet processGepPts(const GepSVFGNode *gep, const CxtPtSet &srcPts) override;

        /// Handle Address SVFGNode to add proper conditional points-to
        virtual void handleAddr(CxtPtSet &pts, const CxtLocDPItem &dpm, const AddrSVFGNode *addr) override
        {
            NodeID srcID = addr->getPAGSrcNodeID();
            /// whether this object is set field-insensitive during pre-analysis
            if (isFieldInsensitive(srcID))
                srcID = getFIObjVar(srcID);

            CxtVar var(dpm.getCond(), srcID);
            addDDAPts(pts, var);
            DBOUT(DDDA, SVFUtil::outs() << "\t add points-to target " << var << " to dpm ");
            DBOUT(DDDA, dpm.dump());
        }

        /// Propagate along indirect value-flow if two objects of load and store are same
        virtual inline bool propagateViaObj(const CxtVar &storeObj, const CxtVar &loadObj) override
        {
            return isSameVar(storeObj, loadObj);
        }

        /// Whether two call string contexts are compatible which may represent the same memory object
        /// compare with call strings from last few callsite ids (most recent ids to objects):
        /// compatible : (e.g., 123 == 123, 123 == 23). not compatible (e.g., 123 != 423)
        virtual inline bool isCondCompatible(const ContextCond &cxt1, const ContextCond &cxt2, bool singleton) const override;

        /// Whether this edge is treated context-insensitively
        bool isInsensitiveCallRet(const SVFGEdge *edge)
        {
            return insensitveEdges.find(edge) != insensitveEdges.end();
        }
        /// Return insensitive edge set
        inline ConstSVFGEdgeSet &getInsensitiveEdgeSet()
        {
            return insensitveEdges;
        }
        /// dump context call strings
        virtual inline void dumpContexts(const ContextCond &cxts)
        {
            SVFUtil::outs() << cxts.toString() << "\n";
        }

        virtual const std::string PTAName() const override
        {
            return "Context Sensitive DDA";
        }

    public:
        ConstSVFGEdgeSet insensitveEdges; ///< insensitive call-return edges
        FlowDDA *flowDDA;                 ///< downgrade to flowDDA if out-of-budget
        DDAClient *_client;               ///< DDA client
    };

    //    OrderedMap<int,ContextCond>   CxtLocDPItem::actual_contextMap ;
    // template<class LocCond>
    // int CSPL_CxtStmtDPItem<LocCond>::cspl_count=0;

    // template<class LocCond>
    // OrderedMap<int,ContextCond> CSPL_CxtStmtDPItem<LocCond>::actual_contextMap=OrderedMap<int,ContextCond>();

    class CSPL_ContextDDA : public ContextDDA
    {
        friend ContextCond;

    public:
        typedef OrderedSet<CSPL_CxtLocDPItem> DPTItemSet;
        typedef OrderedMap<CSPL_CxtLocDPItem, DPTItemSet> DPImToDPImSetMap;
        DPImToDPImSetMap dPImToDPImSetMap;
        DPImToDPImSetMap aliasSetMap;
        DPTItemSet printed_DPTItems;
        bool fullcontext;
        CSPL_CxtLocDPItem *rootDPM;

        typedef OrderedMap<CSPL_CxtLocDPItem, CxtVar> CxtVarToDPImMap;
        CxtVarToDPImMap addr_to_store_Map;
        OrderedSet<CSPL_CxtLocDPItem> previousDPM_single_set;

        typedef OrderedMap<SVFGNode*, OrderedSet<std::pair<CSPL_CxtLocDPItem,CxtVar>>> NodeTo_DPI_CtxVarMap;
        NodeTo_DPI_CtxVarMap root_to_str_pt_pairMap;
        CSPL_ContextDDA(SVFIR *_pag, DDAClient *client) : ContextDDA(_pag, client), fullcontext(true)
        {
            SVFUtil::outs() << "BOOOOO\n";
        }

        virtual bool handleBKCondition(CxtLocDPItem &dpm, const SVFGEdge *edge) override;
        bool pushContextDpm(CxtLocDPItem &dpm, CallSiteID csId);
        bool popContextDpm(CxtLocDPItem &dpm, CallSiteID csId);
        int topContextDpm(CxtLocDPItem &dpm);

        bool matchContextDpm(CxtLocDPItem &dpm, CallSiteID csId);

        int context_length(CallStrCxt *context, int csId);
        int get_first(CallStrCxt *context, int sign);

        DPTItemSet getAliasNext(CxtLocDPItem &dpm)
        {
            DPTItemSet nexts;
            if (aliasSetMap.find(dpm) != aliasSetMap.end())
            {
                DPTItemSet aSet = aliasSetMap[dpm];

                for (auto &adpm : aSet)
                {
                    if (dPImToDPImSetMap.find(adpm) != dPImToDPImSetMap.end())
                    {
                        DPTItemSet tmpSet = dPImToDPImSetMap[adpm];
                        for (auto &t : tmpSet)
                        {
                            nexts.insert(t);
                        }
                    }
                }
            }
            return nexts;
        }
        inline virtual void popRecursiveCallSites(CxtLocDPItem &dpm)
        {
            // SVFUtil::outs() <<"startinPOPRec\n";
            ContextCond &cxtCond = dpm.getCond();
            cxtCond.setNonConcreteCxt();
            CallStrCxt &cxt = cxtCond.getContexts();

            while (!cxt.empty() && isEdgeInRecursion(cxt.back() > 0 ? cxt.back() : -cxt.back()))
            {
                cxt.pop_back();
            }
            // SVFUtil::outs() <<"endingPOPRec\n";
        }
        virtual inline void resetQuery()
        {

            // SVFUtil::outs() <<"clearrrrrrrr\n";
            locToDpmSetMap.clear();
            dpmToloadDpmMap.clear();
            loadToPTCVarMap.clear();
            outOfBudgetQuery = false;
            ddaStat->_NumOfStep = 0;
            backwardVisited.clear();  ///< visited map during backward traversing
            dpmToTLCPtSetMap.clear(); ///< points-to caching map for top-level vars
            dpmToADCPtSetMap.clear(); ///< points-to caching map for address-taken vars

            aliasSetMap.clear();

            outOfBudgetDpms.clear(); ///< out of budget dpm set
            storeToDPMs.clear();
            dPImToDPImSetMap.clear();
            printed_DPTItems.clear();

            oldStoreSrc_to_newDpm_map.clear();
            addrDpM_to_CVar_map.clear();
            // CSPL_CxtLocDPItem::cspl_count=0;
            // CSPL_CxtLocDPItem::actual_contextMap.clear();
        }
        const CSPL_CxtLocDPItem *getStoreOfAddr(CxtVar cvar)
        {

            // for (auto& entry : addr_to_store_Map) {
            //     const CSPL_CxtLocDPItem& storeDPM = entry.first;
            //     if(entry.second==cvar){
            //         // SVFUtil::outs()<<"getStoreOfAddr:  ";
            //         // storeDPM.dump();
            //         //   SVFUtil::outs()<<"end\n";
            //         // cvar.dump();
            //         return &storeDPM;
            //     }
            for (auto &entry : oldStoreSrc_to_newDpm_map)
            {
                // TODO: Add your code here to iterate over the elements
                // ...
            }
            return nullptr;
        }

        virtual void backwardPropDpm(CPtSet &pts, NodeID ptr, const CxtLocDPItem &oldDpm, const SVFGEdge *edge)
        {
            previousDPM_single_set.clear();
            previousDPM_single_set.insert(oldDpm);
            CxtLocDPItem dpm(oldDpm);
            dpm.setLocVar(edge->getSrcNode(), ptr);
            DOTIMESTAT(double start = DDAStat::getClk(true));
            /// handle context-/path- sensitivity
            if (handleBKCondition(dpm, edge) == false)
            {
                DOTIMESTAT(ddaStat->_TotalTimeOfBKCondition += DDAStat::getClk(true) - start);
                DBOUT(DDDA, SVFUtil::outs() << "\t!!! infeasible path svfgNode: " << edge->getDstID() << " --| " << edge->getSrcID() << "\n");
                DOSTAT(ddaStat->_NumOfInfeasiblePath++);
                return;
            }

            /// record the source of load dpm
            if (SVFUtil::isa<IndirectSVFGEdge>(edge))
                addLoadDpmAndCVar(dpm, getLoadDpm(oldDpm), getLoadCVar(oldDpm));

            DOSTAT(ddaStat->_NumOfDPM++);
            /// handle out of budget case

            if (dPImToDPImSetMap.find(oldDpm) == dPImToDPImSetMap.end())
            {
                DPTItemSet dPTItemSet;
                dPImToDPImSetMap.insert(std::make_pair(oldDpm, dPTItemSet));
            }
            dPImToDPImSetMap[oldDpm].insert(dpm);
            // SVFUtil::outs()<<"backwardPropDpm:\n";

            // oldDpm.dump();
            // SVFUtil::outs()<<"====>\n";
            // dpm.dump();
            // SVFUtil::outs()<<"}\n";
            // errs()
            unionDDAPts(pts, findPT(dpm));
        }
        virtual const CxtPtSet &computeDDAPts(const CxtVar &cxtVar);
        void printdpm(CSPL_CxtLocDPItem dom);

    virtual CxtPtSet processGepPts(const GepSVFGNode *gep, const CxtPtSet &srcPts) override;
        virtual void handleAddr(CxtPtSet &pts, const CxtLocDPItem &dpm, const AddrSVFGNode *addr) override
        {
            NodeID srcID = addr->getPAGSrcNodeID();
            /// whether this object is set field-insensitive during pre-analysis
            if (isFieldInsensitive(srcID))
                srcID = getFIObjVar(srcID);

            CxtVar var(dpm.getCond(), srcID);
            addDDAPts(pts, var);
            DBOUT(DDDA, SVFUtil::outs() << "\t add points-to target " << var << " to dpm ");
            DBOUT(DDDA, dpm.dump());

            if (addrDpM_to_CVar_map.find(dpm) == addrDpM_to_CVar_map.end())
            {
                addrDpM_to_CVar_map.insert(std::make_pair(dpm, var));
            }
            // addrDpM_to_CVar_map.insert(std::make_pair(dpm, var));
            // dpm.dump();
            // SVFUtil::outs() << "FOlloes: \n";
          
            // var.dump();
            // SVFUtil::outs() << " ::::!!::\n ";
            // dpm.dump();
            //  SVFUtil::outs() << " ----\n ";

            //  SVFUtil::outs() << "~~~~@@@\n ";
            CxtLocDPItem newdpm = getDPIm(var, addr);
            if (dPImToDPImSetMap.find(dpm) == dPImToDPImSetMap.end())
            {
                DPTItemSet dPTItemSet;
                dPImToDPImSetMap.insert(std::make_pair(dpm, dPTItemSet));
            }
            dPImToDPImSetMap[dpm].insert(newdpm);
        }

        virtual inline CxtLocDPItem getDPImWithOldCond(const CxtLocDPItem &oldDpm, const CVar &var, const SVFGNode *loc)
        {
            CxtLocDPItem dpm(oldDpm);
            dpm.setLocVar(loc, getPtrNodeID(var));

            if (SVFUtil::isa<StoreSVFGNode>(loc))
                addLoadDpmAndCVar(dpm, getLoadDpm(oldDpm), var);

            if (SVFUtil::isa<LoadSVFGNode>(loc))
                addLoadDpmAndCVar(dpm, oldDpm, var);

            DOSTAT(ddaStat->_NumOfDPM++);

            // if(dPImToDPImSetMap.find(oldDpm)==dPImToDPImSetMap.end()){
            //     DPTItemSet dPTItemSet;
            //     dPImToDPImSetMap.insert(std::make_pair(oldDpm,dPTItemSet));
            // }
            // dPImToDPImSetMap[oldDpm].insert(dpm);

            if (aliasSetMap.find(oldDpm) == aliasSetMap.end())
            {
                DPTItemSet dPTItemSet;
                aliasSetMap.insert(std::make_pair(oldDpm, dPTItemSet));
            }
            aliasSetMap[oldDpm].insert(dpm);

            return dpm;
        }

        virtual void reComputeForEdges(const CSPL_CxtLocDPItem &dpm, const SVFGEdgeSet &edgeSet, bool indirectCall = false)
        {
            for (SVFGNode::const_iterator it = edgeSet.begin(), eit = edgeSet.end(); it != eit; ++it)
            {
                const SVFGEdge *edge = *it;
                const SVFGNode *dst = edge->getDstNode();
                typename DDAVFSolver<CxtVar, CxtPtSet, CxtLocDPItem>::LocToDPMVecMap::const_iterator locIt = getLocToDPMVecMap().find(dst->getId());
                /// Only collect nodes we have traversed
                if (locIt == getLocToDPMVecMap().end())
                    continue;
                DPTItemSet dpmSet(locIt->second.begin(), locIt->second.end());
                for (typename DPTItemSet::const_iterator it = dpmSet.begin(), eit = dpmSet.end(); it != eit; ++it)
                {
                    const CSPL_CxtLocDPItem &dstDpm = *it;
                    if (!indirectCall && SVFUtil::isa<IndirectSVFGEdge>(edge) && !SVFUtil::isa<LoadSVFGNode>(edge->getDstNode()))
                    {
                        if (dstDpm.getCurNodeID() == dpm.getCurNodeID())
                        {
                            DBOUT(DDDA, SVFUtil::outs() << "\t Recompute, forward from :");
                            DBOUT(DDDA, dpm.dump());
                            DOSTAT(ddaStat->_NumOfStepInCycle++);
                            clearbkVisited(dstDpm);

                            findPT(dstDpm);

                            bool record = true; // CSPL: here we keep only def-use
                            if (SVFUtil::isa<LoadSVFGNode>(edge->getDstNode()))
                            {
                                const LoadSVFGNode *load = SVFUtil::cast<LoadSVFGNode>(edge->getDstNode());
                                const SVFGNode *loadSrc = getDefSVFGNode(load->getPAGSrcNode());
                                if (loadSrc == edge->getSrcNode())
                                {
                                    record = false;
                                }
                            }
                            else if (SVFUtil::isa<StoreSVFGNode>(edge->getDstNode()))
                            {
                                const StoreSVFGNode *store = SVFUtil::cast<StoreSVFGNode>(edge->getDstNode());
                                const SVFGNode *storeDst = getDefSVFGNode(store->getPAGDstNode());
                                if (storeDst == edge->getSrcNode())
                                {
                                    record = false;
                                }
                            }

                            if (record)
                            {
                                if (dPImToDPImSetMap.find(dstDpm) == dPImToDPImSetMap.end())
                                {
                                    DPTItemSet dPTItemSet;
                                    dPImToDPImSetMap.insert(std::make_pair(dstDpm, dPTItemSet));
                                }
                                dPImToDPImSetMap[dstDpm].insert(dpm);
                            }
                        }
                    }
                    else
                    {
                        if (indirectCall)
                            DBOUT(DDDA, SVFUtil::outs() << "\t Recompute for indirect call from :");
                        else
                            DBOUT(DDDA, SVFUtil::outs() << "\t Recompute forward from :");
                        DBOUT(DDDA, dpm.dump());
                        DOSTAT(ddaStat->_NumOfStepInCycle++);
                        clearbkVisited(dstDpm);

                        findPT(dstDpm);

                        bool record = true; // CSPL: here we keep only def-use
                        if (SVFUtil::isa<LoadSVFGNode>(edge->getDstNode()))
                        {
                            const LoadSVFGNode *load = SVFUtil::cast<LoadSVFGNode>(edge->getDstNode());
                            const SVFGNode *loadSrc = getDefSVFGNode(load->getPAGSrcNode());
                            if (loadSrc == edge->getSrcNode())
                            {
                                record = false;
                            }
                        }
                        else if (SVFUtil::isa<StoreSVFGNode>(edge->getDstNode()))
                        {
                            const StoreSVFGNode *store = SVFUtil::cast<StoreSVFGNode>(edge->getDstNode());
                            const SVFGNode *storeDst = getDefSVFGNode(store->getPAGDstNode());
                            if (storeDst == edge->getSrcNode())
                            {
                                record = false;
                            }
                        }
                        if (record)
                        {
                            if (dPImToDPImSetMap.find(dstDpm) == dPImToDPImSetMap.end())
                            {
                                DPTItemSet dPTItemSet;
                                dPImToDPImSetMap.insert(std::make_pair(dstDpm, dPTItemSet));
                            }
                            dPImToDPImSetMap[dstDpm].insert(dpm);
                        }
                    }
                }
            }
        }

        inline void startNewPTCompFromLoadSrc(CPtSet &pts, const CSPL_CxtLocDPItem &oldDpm)
        {
            //    SVFUtil::outs()<<"fjcwiofjiow\n";
            const LoadSVFGNode *load = SVFUtil::cast<LoadSVFGNode>(oldDpm.getLoc());
            const SVFGNode *loadSrc = getDefSVFGNode(load->getPAGSrcNode());
            DBOUT(DDDA, SVFUtil::outs() << "!##start new computation from loadSrc svfgNode " << load->getId() << " --> " << loadSrc->getId() << "\n");
            const SVFGEdge *edge = getSVFG()->getIntraVFGEdge(loadSrc, load, SVFGEdge::IntraDirectVF);
            assert(edge && "Edge not found!!");
            ContextDDA::backwardPropDpm(pts, load->getPAGSrcNodeID(), oldDpm, edge);
        }
        inline void startNewPTCompFromStoreDst(CPtSet &pts, const CSPL_CxtLocDPItem &oldDpm)
        {
            const StoreSVFGNode *store = SVFUtil::cast<StoreSVFGNode>(oldDpm.getLoc());
            const SVFGNode *storeDst = getDefSVFGNode(store->getPAGDstNode());
            DBOUT(DDDA, SVFUtil::outs() << "!##start new computation from storeDst svfgNode " << store->getId() << " --> " << storeDst->getId() << "\n");
            const SVFGEdge *edge = getSVFG()->getIntraVFGEdge(storeDst, store, SVFGEdge::IntraDirectVF);
            assert(edge && "Edge not found!!");
            ContextDDA::backwardPropDpm(pts, store->getPAGDstNodeID(), oldDpm, edge);
        }

        const CxtLocDPItem *getOldDPI(CxtVar &ptd)
        {
            const CxtLocDPItem *newDpm = nullptr;
            for (auto &entry : addrDpM_to_CVar_map)
            {
                if (entry.second == ptd)
                {
                    newDpm = &entry.first;

                    break;
                }
            }

            if (!newDpm)
            {
                llvm::errs()<< "error_notCARFOund\n";
                return nullptr;
            }
            else
            {
                // SVFUtil::outs().flush();
                // SVFUtil::outs() << "getOldDPI\n";
                // SVFUtil::outs().flush();
                // newDpm->dump();

                // NodeID nid = ptd.get_id();
                // PAGNode *pn = pag->getGNode(nid);
                // const Value *tv = pn->getValue();
                // llvm::errs() << "value: " << *tv << "\n";
                // llvm::errs().flush();
            }

            const CxtLocDPItem *strDpm = nullptr;
            // {
            for (auto &entry : oldStoreSrc_to_newDpm_map)
            {
                if (entry.second == *newDpm)
                {
                    strDpm = &entry.first;
                    break;
                }
            }

            if (!strDpm)
            {
                llvm::errs().flush();
                // llvm::errs() << "error_notFOund\n";
                llvm::errs().flush();
                return nullptr;
            }
            else
            {
                // SVFUtil::outs().flush();
                // SVFUtil::outs() << "getnewDPI\n";
                // SVFUtil::outs().flush();
                // strDpm->dump();

               
                llvm::errs().flush();
            }

            // }
            return strDpm;
        }

         const Instruction *getStrInst(CxtVar &ptd){ //cspl: or this can be origiinal inst
             const CxtLocDPItem* tc=getOldDPI(ptd);
            //  SVFUtil::outs()<<tc<<"----"<<ptd.toString()<<"getStrInstPPPP\n";
            //  tc->dump();  
             SVFUtil::outs().flush();
             if(!tc){
                // llvm::errs()<<tc<<"----"<<ptd.toString()<<"getStfdcPPPP\n";
                // ptd. ->getValue()
                PAGNode *p_n = pag->getGNode(ptd.get_id());
                if(p_n&& p_n->hasValue()){
                    const Value *tv = p_n->getValue();
                    return llvm::dyn_cast<llvm::Instruction>(tv);
                }
                
                return nullptr;
             }
             const Instruction *inst = llvm::dyn_cast<llvm::Instruction>(tc->getLoc()->getValue());
            if (!inst)
            {
                // llvm::errs()<<"hcidhiuw: "<<tc->getLoc()->getValue()<<"\n";
            }
            
             return inst;
        }
    };

} // End namespace SVF

#endif /* ContextDDA_H_ */
