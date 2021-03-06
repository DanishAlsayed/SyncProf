/*#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
//#include "../lib/SyncKey.h"
using namespace llvm;

namespace {
	struct SyncProf : public FunctionPass {
	static char ID;
	
	SyncProf() : FunctionPass(ID) {}
	virtual bool runOnFunction(Function &F) {
	static std::vector <std::string> syncs;
	//mutex
	syncs.push_back("pthread_mutex_init");
	syncs.push_back("pthread_mutex_lock");
	syncs.push_back("pthread_mutex_unlock");
	syncs.push_back("pthread_mutex_trylock");
	syncs.push_back("pthread_mutex_destroy");
	syncs.push_back("pthread_mutexattr_getkind_np");
	syncs.push_back("pthread_mutexattr_setkind_np");
	syncs.push_back("pthread_mutexattr_gettype");
	syncs.push_back("pthread_mutexattr_settype");
	syncs.push_back("pthread_mutexattr_getpshared");
	syncs.push_back("pthread_mutexattr_setpshared");
	//cond
	syncs.push_back("pthread_cond_init");
	syncs.push_back("pthread_cond_wait");
	syncs.push_back("pthread_cond_timedwait");
	syncs.push_back("pthread_cond_signal");
	syncs.push_back("pthread_cond_broadcast");
	syncs.push_back("pthread_cond_destroy");
	syncs.push_back("pthread_condattr_getpshared");
	syncs.push_back("pthread_condattr_setpshared");
	syncs.push_back("pthread_condattr_getkind_np");
	syncs.push_back("pthread_condattr_setkind_np");
	//rw
	syncs.push_back("pthread_rw_init");
	syncs.push_back("pthread_rw_rdlock");
	syncs.push_back("pthread_rw_tryrdlock");
	syncs.push_back("pthread_rw_wrlock");
	syncs.push_back("pthread_rw_trywrlock");
	syncs.push_back("pthread_rw_unlock");
	syncs.push_back("pthread_rw_destroy");
	syncs.push_back("pthread_rwlockattr_getpshared");
	syncs.push_back("pthread_rwlockattr_setpshared");
	//sem
	syncs.push_back("sem_init");
	syncs.push_back("sem_wait");
	syncs.push_back("sem_post");
	syncs.push_back("sem_getvalue");
	syncs.push_back("sem_destroy");
	//rcu
	syncs.push_back("rcu_start");
	syncs.push_back("rcu_stop");
	syncs.push_back("rcu_stop");
	syncs.push_back("check_rcu");
	syncs.push_back("record_rcu");
	syncs.push_back("free_rcu");
	LLVMContext &Ctx = F.getContext();
	Constant *logFunc = F.getParent()->getOrInsertFunction("_logop", Type::getVoidTy(Ctx), Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx), Type::getInt8Ty(Ctx), NULL);
	//Constant *logFunc = F.getParent()->getOrInsertFunction("std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count", Type::getVoidTy(Ctx), Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx), Type::getInt8Ty(Ctx), NULL);
	int InstructionID = 0;
	for(Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
                {
			for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI){
				if(isa<CallInst>(&(*BI)) ) {
					Function* func= cast<CallInst>(&(*BI))->getCalledFunction();
					if(func != NULL){
						StringRef fname = func->getName();
						if(std::find(syncs.begin(),syncs.end(),fname) != syncs.end()){
							errs()<<fname + " was called\n";
							//Instruction *newInst = CallInst::Create(logFunc,"");
							//BB->getInstList().insert(BI, newInst);
							
							IRBuilder<> builder(&(*BI));
			    				Value* args[3] = {builder.getInt32(1), builder.getInt32(InstructionID++), builder.getInt8('c')};
			    				builder.CreateCall(logFunc, args);
							builder.SetInsertPoint((*BI).getNextNode());
							builder.CreateCall(logFunc, args);
						}
					}

				}
			}
                }


      return false;
    }
	void initialize(int code, int InstId, char c){
		std::cout <<"initialize was inserted\n";
	}
  };
}
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
*/
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
//#include "llvm/Instruction.h"
#include "llvm/PassManager.h"
#include "llvm/PassRegistry.h"
#include <time.h>
#include "llvm/IR/IRBuilder.h"
#include <vector>
#include <string>
#include <iostream>

using namespace llvm;

namespace{
    struct SyncProf : public ModulePass{
static char ID;  
        Function *hook;
	Function *tfunc;
        SyncProf() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M)
        {	
		Constant *hookFunc;
		hookFunc = M.getOrInsertFunction("_Z8profileri", Type::getInt64Ty(M.getContext()),NULL);
		hook= cast<Function>(hookFunc);
		Constant *hookFunc2;
		hookFunc2 = M.getOrInsertFunction("_Z9profiler2xii", Type::getVoidTy(M.getContext()),Type::getInt64Ty(M.getContext()),Type::getInt32Ty(M.getContext()),Type::getInt32Ty(M.getContext()),NULL);
		tfunc = cast<Function>(hookFunc2);
            for(Module::iterator F = M.begin(), E = M.end(); F!= E; ++F)
            {
		Function* func = cast<Function>(&(*F));
		if(func!=NULL){
			std::string fname = F->getName();
			if(F->getName()=="rcu_start"||F->getName()=="rcu_stop")
				std::cout<<"***********************"<<fname<<std::endl;
		}

                for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
                {
                    SyncProf::runOnBasicBlock(BB);
                }
            }
		/*Constant *output = M.getOrInsertFunction("_Z8outputResultsv", Type::getVoidTy(M.getContext()),NULL);
		Function *resultsOutput = cast<Function>(output);
		IRBuilder<> builder(M.end());
		builder.CreateCall(resultsOutput);*/
            return false;
        }
        virtual bool runOnBasicBlock(Function::iterator &BB)
        {
		static std::vector <std::string> syncs;
		syncs.push_back("pthread_mutex_lock");
		syncs.push_back("pthread_cond_wait");
		syncs.push_back("sem_wait");
            for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)          
            {
                    if(isa<CallInst>(&(*BI)) )
                    {
				Function* func= cast<CallInst>(&(*BI))->getCalledFunction();
					if(func != NULL){
						if(func->getName() == "pthread_mutex_lock" || func->getName() =="pthread_cond_wait"||func->getName() =="sem_wait"||func->getName().find("rcu_quiesce")!=std::string::npos){
							std::string fname= func->getName();
							CallInst *CI = dyn_cast<CallInst>(BI);
							std::cout<<fname<<" called\n";
							IRBuilder<> builder(&(*BI));
							Value* args [1] = {builder.getInt32(reinterpret_cast<uintptr_t>(CI))};
							Instruction *newInst = CallInst::Create(hook, args);
                           				BB->getInstList().insert((Instruction*)CI, newInst);
							builder.SetInsertPoint((*BI).getNextNode());
							std::cout<<CI<<std::endl;
							if(func->getName()=="pthread_mutex_lock"){
								Value* args [3] = {newInst, builder.getInt32(reinterpret_cast<uintptr_t>(CI)), builder.getInt32(1)};
								Instruction *newInst2 = CallInst::Create(tfunc, args);
								BB->getInstList().insert((CI)->getNextNode(), newInst2);
								//builder.CreateCall(tfunc, args);
							}
							else if(func->getName()=="pthread_cond_wait"){
								Value* args[3] = {newInst, builder.getInt32(reinterpret_cast<uintptr_t>(CI)), builder.getInt32(2)};
								Instruction *newInst2 = CallInst::Create(tfunc, args);
								BB->getInstList().insert((CI)->getNextNode(), newInst2);
								//builder.CreateCall(tfunc, args);
							}
							else if(func->getName()=="sem_wait"){
								Value* args[3] = {newInst, builder.getInt32(reinterpret_cast<uintptr_t>(CI)), builder.getInt32(3)};
								Instruction *newInst2 = CallInst::Create(tfunc, args);
								BB->getInstList().insert((CI)->getNextNode(), newInst2);
								//builder.CreateCall(tfunc, args);
							}
							else if(func->getName().find("rcu_quiesce")!=std::string::npos){
								Value* args[3] = {newInst, builder.getInt32(reinterpret_cast<uintptr_t>(CI)), builder.getInt32(4)};
								Instruction *newInst2 = CallInst::Create(tfunc, args);
								BB->getInstList().insert((CI)->getNextNode(), newInst2);
								//builder.CreateCall(tfunc, args);
							}
                           				/*Instruction *newInst = CallInst::Create(hook, "");
                           				BB->getInstList().insert((Instruction*)CI, newInst);
							Instruction *newInst2 = CallInst::Create(tfunc, (Value*)newInst,"");
							BB->getInstList().insert((Instruction*)CI->getNextNode(), newInst2);*/
						}
					}
                    }
                  
            }
            return true;
        }
    };
}
char SyncProf::ID = 0;

static void loadPass(const PassManagerBuilder &,
                         PassManagerBase &PM) {
  PM.add(new SyncProf());
}
static RegisterStandardPasses clangtoolLoader_0x(PassManagerBuilder::EP_OptimizerLast, loadPass);
static RegisterStandardPasses clangtoolLoader_o0(PassManagerBuilder::EP_EnabledOnOptLevel0, loadPass);
//static RegisterStandardPasses
  //RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
    //             registerSyncProf);

static RegisterPass<SyncProf> X("SyncProf", "Synchronization profiler", false, false);
