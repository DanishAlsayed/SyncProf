#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <pthread.h>
#include <string>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <atomic>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

class Record{
	public:
		int id;
		long long time;
		int code;
		int calls;
		struct timeval t_execs;
		long i_o;
		long csw;
		atomic<int> blocked;
		atomic<int> unblocked;
		Record(int,long long,int,int, timeval,long,long,int,int);
};
Record::Record(int i, long long t, int c, int ca, timeval tt, long io, long c_sw, int bl, int unbl){
	id = i;
	time = t;
	code = c;
	calls = ca;
	t_execs = tt;
	i_o = io;
	csw = c_sw;
	blocked.store(bl);
	unblocked.store(unbl);
}
pthread_mutex_t rlock;
unordered_map<int,Record*>records;
bool isFirst = false;
struct rusage r_usage;
struct timeval t_times;

void addRecord(int id, long long time, int code, timeval t_t, long io, long c_sw, int bl, int unbl){
	if(records.find(id) == records.end()){//this instruction isn't there before
		Record * r = new  Record(id, time, code, 1, t_t, io,c_sw,bl, unbl);
		pthread_mutex_lock(&rlock);//check if this locking procedure will skew the results and hence if there is a need to create and detach a thread
		records.insert(make_pair(r->id, r));
		pthread_mutex_unlock(&rlock);
	}
	else{//the instruction is there before
		records[id]->time+=time;
		records[id]->t_execs.tv_sec+=t_t.tv_sec;
		records[id]->t_execs.tv_usec+=t_t.tv_usec;
		records[id]->i_o += io;
		records[id]->csw += c_sw;
		records[id]->calls+=unbl;
		records[id]->blocked += bl;
		records[id]->unblocked += unbl;
		records[id]->code = code;
		//cout<<"blocked threads on "<<id<<" : "<<records[id]->blocked.load()<<endl;
		//cout<<"UNblocked threads on "<<id<<" : "<<records[id]->unblocked.load()<<endl;
	}
}

void printResults(){
	clock_t startTime = clock();
	clock_t timeElapsed;
	double counter;
	unordered_map<int,Record*>::iterator it;
	while(true){
		timeElapsed = clock() - startTime;
		counter = timeElapsed / (double)CLOCKS_PER_SEC;
		if(counter>=1){
			cout<<"***** SYNCPROF RESULTS *****\n";
			cout<<"ID"<<
			std::setw(25)<<"Code"<<
			std::setw(20)<<"Contention(us)"<<
			std::setw(10)<<"Calls"<<
			std::setw(20)<<"Execution(us)"<<
			std::setw(10)<<"CSP"<<
			std::setw(10)<<"I/O"<<
			std::setw(10)<<"CSW"<<
			std::setw(20)<<"Blocked Threads\n";
			string name;
			double total_csp = 0.0;
			for(it = records.begin(); it != records.end(); it++){
				if(records[it->first]->code!=1){
					if (records[it->first]->code==1)
						name = "pthread_mutex_lock";
					else if(records[it->first]->code==2)
						name = "pthread_cond_wait";
					else if(records[it->first]->code==3)
						name = "sem_wait";
					else if(records[it->first]->code==2)
						name = "rcu";
					cout<<records[it->first]->id<<
					std::setw(3)<<name<<
					std::setw(15)<<records[it->first]->time<<
					std::setw(15)<<records[it->first]->calls<<
					std::setw(15)<<records[it->first]->t_execs.tv_sec*1000000+records[it->first]->t_execs.tv_usec<<
					std::setw(20)<<(double)records[it->first]->time/(records[it->first]->t_execs.tv_sec*1000000+records[it->first]->t_execs.tv_usec)<<
					std::setw(7)<<records[it->first]->i_o<<
					std::setw(10)<<records[it->first]->csw<<
					std::setw(10)<<records[it->first]->blocked.load() - records[it->first]->unblocked.load()<<endl;
					if((records[it->first]->t_execs.tv_sec*1000000+records[it->first]->t_execs.tv_usec)>0)
						total_csp += (double)records[it->first]->time/(records[it->first]->t_execs.tv_sec*1000000+records[it->first]->t_execs.tv_usec);
				}
			}
			cout << "Total CSP:"<<total_csp <<endl;
			total_csp = 0.0;
			startTime = clock();
		}
	}
}
void outputResults(){
	thread printer(printResults);
	printer.detach();
}
long long profiler(int id) {
	long long t1 = std::chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	t_times.tv_sec = 0;
	t_times.tv_usec = 0;
	addRecord(id, 0,0,t_times, 0,0,1,0);
	return t1;
}
void profiler2(long long t1, int id, int nID) {
	long long t2 = std::chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	long long t = t2-t1;
	getrusage(RUSAGE_THREAD, &r_usage);
	t_times.tv_sec = r_usage.ru_utime.tv_sec + r_usage.ru_stime.tv_sec;
	t_times.tv_usec = r_usage.ru_utime.tv_usec + r_usage.ru_stime.tv_usec;
	addRecord(id,t,nID, t_times, (r_usage.ru_inblock+r_usage.ru_oublock), (r_usage.ru_nvcsw+r_usage.ru_nivcsw),0,1);
	if (!isFirst){
		outputResults();
		isFirst = true;
	}
	//thread recorder(addRecord,id, t, nID);
	//recorder.detach();
}

