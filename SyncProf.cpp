#include "llvm/Pass.h"
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
	Constant *logFunc = F.getParent()->getOrInsertFunction("logop", Type::getVoidTy(Ctx), Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx), Type::getInt8Ty(Ctx), NULL);
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
							/*IRBuilder<> builder(&(*BI));
			    				Value* args[3] = {builder.getInt32(1), builder.getInt32(InstructionID++), builder.getInt8('c')};
			    				builder.CreateCall(logFunc, args);
							builder.SetInsertPoint((*BI).getNextNode());
							builder.CreateCall(logFunc, args);*/
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

char SyncProf::ID = 0;

static void registerSyncProf(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SyncProf());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSyncProf);

static RegisterPass<SyncProf> X("SyncProf", "Synchronization profiler", false, false);
