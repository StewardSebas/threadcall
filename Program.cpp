#include "VSTDThreadCall.hpp"
#include "VSTDThreadFunction.hpp"
#include <map>

using namespace std;
/*

class CounterThread : public VSTD::ThreadCall
{
public:
	int counter;

	virtual bool onFunction()
	{
	  mutex.lock();
	  ++counter;
//	  printf("thread inner %d\n",counter);
	  mutex.unlock();
	  return true;
	}

	int GetValue()
	{
		int counterValue = 0;
		mutex.lock();    // protect the counter variable
		counterValue = counter;
		mutex.unlock();
		return counter;
	}

	void reset()
	{
		mutex.lock();
		counter = 0;
		mutex.unlock();
	}

	CounterThread()
	{
   		counter=0;
	}
	~CounterThread()
	{
	}
};
*/



class ChildThread: public VSTD::ThreadFunction
{
public:
	bool Function()
	{
		pthread_t id;
		getThreadId(&id);
		mutex.lock();
		printf("\tスレッド(%ld)\n",id);
		mutex.unlock();

		return true;
	}
	ChildThread(){}
	~ChildThread(){}

};

int main(int argc, char *argv[])
{
	map<string , ChildThread *>	ThreadList;
	VSTD::ThreadCall ThreadCtrl;

	for(int i = 1 ; i < 10 ; i++)
	{
		ChildThread * Thread;
		std::string name;
		char num[2];
		sprintf(num,"%d",i);
		name = num;
		ThreadList.insert(map<string , ChildThread * >::value_type(name,Thread));
		ThreadList[name] = new ChildThread();
//		ThreadList[name].setThreadStatus(VSTD::TH_STAT_WAIT);
		ThreadCtrl.setFunction(ThreadList[name]);
		ThreadCtrl.stop();

	}
}


	//	thread.setFunction(&myTask2);


	//myTask.wait(1);*/
	//cout << "\tVSTDThreadCallのonFunctionの実装実験\n";
	//CounterThread MyThread;
	//CounterThread MyThread2;
	// カウンターが10までの間繰り返し
/*	while (MyThread.GetValue() < 100000)
	{
		MyThread.setFunction();
	}
	printf("%d\n",MyThread.GetValue());*/
	/*

	int argument=1;
	while (MyThread2.GetValue() < 20)
	{
		MyThread2.setFunction(&argument);
	}*/
/*
	CounterThread thr ;
	thr.setThreadType(VSTD::TH_TYP_EVENTDRIVEN,100);
	VSTD::Sleep(500);
	printf("Outer %d \n",thr.counter);*/
//
