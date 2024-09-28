

#include "pac_enforce.h"

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

// static cl::list<const PassInfo*, bool, PassNameParser>
// PassList(cl::desc("Optimizations available:"));

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
char *arg0 = "/home/pac/toolchain/SVF/Release-build/bin/dvf";
char *arg1 = "-cxt";
char *arg2 = "--cpts";
char *arg3 = "-max-cxt=2";
char *arg4 = "-flow-bg=10000";
char *arg5 = "-cxt-bg=10000";
char *arg6 = "--fdaa";

char *arg7 = "-query=0";

void dumpJson(const json &jsonData)
{

    std::cout << "Points to:" << std::endl;
    for (auto &[key, value] : jsonData["points_to"].items())
    {
        std::cout << "Key: " << key << std::endl;
        std::cout << "  Check Value: ";
        for (int checkValue : value["check_value"])
        {
            std::cout << checkValue << " ";
        }
        std::cout << std::endl;
        std::cout << "  Context Level: " << value["ctx_level"] << std::endl;
    }

    // Iterating over the "pointer_def" array
    std::cout << "\nPointer Def:" << std::endl;
    for (const std::string &def : jsonData["pointer_def"])
    {
        std::cout << def << " ";
    }
    std::cout << std::endl;

    // Iterating over the "pointer_use" array
    std::cout << "\nPointer Use:" << std::endl;
    for (const std::string &use : jsonData["pointer_use"])
    {
        std::cout << use << " ";
    }
    std::cout << std::endl;
}

bool Pac_enforcePass::runOnModule(llvm::Module &M)
{

    arg_value[0] = arg0;
    arg_value[1] = arg1;
    arg_value[2] = arg2;
    arg_value[3] = arg3;
    arg_value[4] = arg4;
    arg_value[5] = arg5;
    arg_value[6] = arg6;
    arg_value[7] = arg7;
    //    arg_value[8]=arg8;
    char *filename = (char *)M.getModuleIdentifier().c_str();
    errs() << "Bitcode Filename: " << filename << "\n";
    arg_value[8] = filename;
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Demand-Driven Points-to Analysis\n");

    std::vector<std::string> moduleNameVec;
    std::string tmp(filename);
    moduleNameVec.push_back(tmp);

    // SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    SVFModule *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(M);
    svfModule->buildSymbolTableInfo();

    SVFIRBuilder builder;
    SVFIR *svfir = builder.build(svfModule);

    SymbolTableInfo *symInfo = SymbolTableInfo::SymbolInfo();
    SymbolTableInfo::ValueToIDMapTy &valSymMap = symInfo->valSyms();

    std::ifstream jsonFile("optimal_policy.txt");

    // Check if the file was opened successfully
    if (!jsonFile.is_open())
    {
        std::cerr << "Could not open the file!" << std::endl;
        return 1;
    }

    // Parse the file into a JSON object
    json jsonData;
    jsonFile >> jsonData;
    jsonFile.close();
    // dumpJson(jsonData  );

    // errs()<<"jsonData: "<<jsonData.to<<"\n";
    for (SVFModule::iterator fit = svfModule->begin(), efit = svfModule->end();
         fit != efit; ++fit)
    {
        const SVFFunction &fun = **fit;
        /// collect return node of function fun

        for (Function::iterator bit = fun.getLLVMFun()->begin(), ebit = fun.getLLVMFun()->end();
             bit != ebit; ++bit)
        {
            BasicBlock &bb = *bit;
            for (BasicBlock::iterator it = bb.begin(), eit = bb.end();
                 it != eit;)
            {

                Instruction &inst = *it;
                if (it != eit)
                {
                    ++it;
                }
                // CallBase* callInst = llvm::dyn_cast<llvm::CallBase>(&inst);
                if (CallBase *callInst = llvm::dyn_cast<llvm::CallBase>(&inst))
                {
                    if (callInst->getCalledFunction() &&
                        callInst->getCalledFunction()->getName().startswith("pac_log_function"))
                    {

                        // errs() << "CallBase: " << *callInst << "\n";
                        processInst(M, callInst, jsonData, it);

                        callInst->eraseFromParent();
                    }
                    // csdda.logReturnProtectToFile(returnInst);
                }
            }
        }
    }

    return true;
}

// type 1:
void Pac_enforcePass::processInst(llvm::Module &M, CallBase *callInst, const json &jsonData, BasicBlock::iterator &it)
{

    // Function * called=callInst->getCalledFunction();
    Value *arg1 = callInst->getArgOperand(0);
    Value *arg2 = callInst->getArgOperand(1);
    int arg1_val;
    int arg2_val;
    // Check if the arguments are constants (for example, i32 constants)
    if (ConstantInt *constArg1 = dyn_cast<ConstantInt>(arg1))
    {
        arg1_val = constArg1->getValue().getSExtValue();
        // errs() << "arg1: " << constArg1->getValue() << "\n";
    }
    else
    {
        // errs() << "arg1 is not constant\n";
        return;
    }

    if (ConstantInt *constArg2 = dyn_cast<ConstantInt>(arg2))
    {
        arg2_val = constArg2->getValue().getSExtValue();
        // errs() << "arg2: " << constArg2->getValue() << "\n";
    }
    else
    {
        // errs() << "arg2 is not constant\n";
        return;
    }

    // int
    // errs() << "arg1_val: " << arg1_val << " arg2_val: " << arg2_val << "\n";

    if (arg1_val == 4)
    {
        return;
    }
    else if (arg1_val == 3)
    {
        bool keep = false;
        for (const std::string &cts : jsonData["context"])
        {
            // continue;
            // errs()<<arg2_val<<" cts: "<<cts<<"\n";
            if (cts == to_string(arg2_val))
            {
                keep = true;
                break;
            }
            // std::cout << def << " ";
        }
        if (!keep)
        {
            return;
        }
        // errs() << "Cal--lBase: " << *it << "\n";
        BasicBlock::iterator it_bp=it;
        // Instruction *nextcallInst = &*it;
        
        while(it!=it->getParent()->end()){
            // errs() << "cotinue: " << *it << "\n";
            if (CallBase *nextcallInst = llvm::dyn_cast<llvm::CallBase>(&*it))
            {
                // errs() << "Cal--lBase: " << *nextcallInst << "\n";
                if (nextcallInst->getCalledFunction() &&
                    !nextcallInst->getCalledFunction()->getName().startswith("pac_log_function"))
                {
                    break;
                }
            }
             errs() << "cotinue: " << *it << "\n";
            it++;
        }
        if(it==it->getParent()->end()){
            return;
        }
        // errs() << "now:  -\n";
        Instruction *nextcallInst = &*it;
        LLVMContext &Context = M.getContext();
        llvm::IRBuilder<> builder_before(nextcallInst);
        Function *pacContextRecordFunc = M.getFunction("pac_context_record");
        if (!pacContextRecordFunc)
        {
            // Declare the function if it doesn't exist
            FunctionType *funcType = FunctionType::get(Type::getVoidTy(Context), {Type::getInt32Ty(Context)}, false);
            pacContextRecordFunc = Function::Create(funcType, Function::ExternalLinkage, "pac_context_record", &M);
        }

        // Insert the call to pac_context_pop after callInst
        Value *constantVal = ConstantInt::get(Type::getInt32Ty(Context), arg2_val);

        // Insert the call to pac_context_record(55) after callInst
        builder_before.CreateCall(pacContextRecordFunc, {constantVal});

        llvm::IRBuilder<> builder_after(nextcallInst->getNextNode());

        Function *pacContextPopFunc = M.getFunction("pac_context_pop");
        if (!pacContextPopFunc)
        {
            // Declare the function if it doesn't exist
            FunctionType *funcType = FunctionType::get(Type::getVoidTy(Context), false);
            pacContextPopFunc = Function::Create(funcType, Function::ExternalLinkage, "pac_context_pop", &M);
        }

        // Insert the call to pac_context_pop after callInst
        builder_after.CreateCall(pacContextPopFunc);
        it=it_bp;
    }
    else if (arg1_val == 2)
    {
       
        bool keep = false;
        for (const std::string &def : jsonData["pointer_def"])
        {
           
            if (def == to_string(arg2_val))
            {   
               
                keep = true;
                
                break;
            }
           
        }
        if (!keep)
        {
            return;
        }
        Value *arg3 = callInst->getArgOperand(2);
        
        LLVMContext &Context = M.getContext();
        llvm::IRBuilder<> builder_before(callInst);
        Function *pacContextDefFunc = M.getFunction("pac_pointer_def");
        if (!pacContextDefFunc)
        {
            // Declare the function if it doesn't exist
            FunctionType *funcType = FunctionType::get(Type::getInt8PtrTy(M.getContext()), Type::getInt32Ty(Context),arg3->getType());
            pacContextDefFunc = Function::Create(funcType, Function::ExternalLinkage, "pac_pointer_def", &M);
        }

        // Insert the call to pacContextDefFunc after callInst
        Value *constantVal = ConstantInt::get(Type::getInt32Ty(Context), arg2_val);

        // Insert the call to pac_context_record(55) after callInst
        Value* newcall=builder_before.CreateCall(pacContextDefFunc, {constantVal,arg3});
        for (User *U : callInst->users()){
            U->replaceUsesOfWith(callInst, newcall);
        }
    }

    else if (arg1_val == 1)

    {
        bool keep = false;
        for (const std::string &def : jsonData["pointer_use"])
        {
           
            if (def == to_string(arg2_val))
            {   
               
                keep = true;
                
                break;
            }
           
        }
        if (!keep)
        {
            return;
        }
        Value *arg3 = callInst->getArgOperand(2);

        llvm::IRBuilder<> builder(callInst);
        LLVMContext &Context = M.getContext();


        // Define the table length
        int tableLength = 0;
            vector<int> checkValues;
          for (const int &cv : jsonData["points_to"][to_string(arg2_val)]["check_value"])
            {
                tableLength++;
                checkValues.push_back(cv);
            }
            

        // Create an array type [N x i32] where N = tableLength
        ArrayType *arrayType = ArrayType::get(Type::getInt32Ty(Context), tableLength);

        // Initialize the elements of the array with the values 1, 2, 3, 4, 5
        std::vector<Constant *> values;
        for (int i = 1; i <= tableLength; i++)
        {
            values.push_back(ConstantInt::get(Type::getInt32Ty(Context), checkValues[i]));
        }

        // Create a constant array with the initialized values
        Constant *initializer = ConstantArray::get(arrayType, values);

        // Create the global array with the initialized values
        GlobalVariable *globalArray = new GlobalVariable(
            M, arrayType, false, GlobalValue::ExternalLinkage,
            initializer, "pacEC_" + to_string(arg2_val));

        // Prepare for calling the `pac_check` function
        Function *pacCheckFunc = M.getFunction("pac_pointer_check");
        if (!pacCheckFunc)
        {
            // If the function doesn't exist, declare it
            std::vector<Type *> args;
            args.push_back(llvm::IntegerType::getInt32Ty(Context)); // use id
            args.push_back(arg3->getType()); // to check pointer value
     
            args.push_back(Type::getInt32PtrTy(Context)); // int* table
            args.push_back(Type::getInt32Ty(Context));    // int table length
            args.push_back(Type::getInt32Ty(Context));    // int ctx length
            FunctionType *pacCheckType = FunctionType::get(llvm::Type::getInt8PtrTy(M.getContext()), args, false);
            pacCheckFunc = Function::Create(pacCheckType, Function::ExternalLinkage, "pac_pointer_check", M);
        }

        // Insert the call to `pac_check` in the first basic block of the function
        // BasicBlock &entryBlock = F.getEntryBlock();
        // IRBuilder<> builder(&*entryBlock.getFirstInsertionPt());

        // Get the pointer to the first element of the global array
        Value *zero = ConstantInt::get(Type::getInt32Ty(Context), 0);
        std::vector<Value *> indices = {zero, zero};
        Value *arrayPtr = builder.CreateGEP(globalArray->getValueType(), globalArray, indices, "pacTablePtr_" + to_string(arg2_val));

        // Create the call to `pac_check(globalTable, tableLength)`
        Value *tableLengthVal = ConstantInt::get(Type::getInt32Ty(Context), tableLength);
        Value *contextLengthVal = ConstantInt::get(Type::getInt32Ty(Context), jsonData["points_to"][to_string(arg2_val)]["ctx_level"]);
        Value *constantVal = ConstantInt::get(Type::getInt32Ty(Context), arg2_val);
        builder.CreateCall(pacCheckFunc, {constantVal,arg3,arrayPtr, tableLengthVal,contextLengthVal});
    }
}

char Pac_enforcePass::ID = 0;
static llvm::RegisterPass<Pac_enforcePass> X("pac_enforcePass", "pac_enforce Pass",
                                             false /* Only looks at CFG */,
                                             false /* Analysis Pass */);

static llvm::RegisterStandardPasses
    Y(llvm::PassManagerBuilder::EP_EarlyAsPossible,
      [](const llvm::PassManagerBuilder &Builder,
         llvm::legacy::PassManagerBase &PM)
      { PM.add(new Pac_enforcePass()); });

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
