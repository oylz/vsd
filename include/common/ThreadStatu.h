#ifndef _THREADSTATUH_
#define _THREADSTATUH_
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>


class ThreadStatu{
private:
	enum TS{
		TS_NONE = 0,
		TS_RUN,
		TS_STOPPING,
		TS_STOPFINISH
	};

	boost::mutex mutex_;
	boost::condition_variable cv_;
	TS ts_; 
public:
	void Print(){
		printf("ts:%d\n", (int)ts_);
	}
	ThreadStatu(){
		ts_ = TS_RUN;
	}
	void Reset(){
		boost::mutex::scoped_lock lock(mutex_);
		ts_ = TS_RUN;
	}	
	void SetStopping(){
		boost::mutex::scoped_lock lock(mutex_);
		if(ts_ == TS_RUN){
			ts_ = TS_STOPPING;
		}
	}
	bool IsStopping(){
		boost::mutex::scoped_lock lock(mutex_);
		bool re = (ts_==TS_STOPPING);
		return re;
	}
	void SetStopFinish(){
		boost::mutex::scoped_lock lock(mutex_);
		ts_ = TS_STOPFINISH;
		lock.unlock();
		cv_.notify_one();
	}
	void WaitStopFinish(){
		boost::mutex::scoped_lock lock(mutex_);
		while(ts_ != TS_STOPFINISH){
			cv_.wait(lock);
		}
	}
	bool IsExit(){
		boost::mutex::scoped_lock lock(mutex_);
		return (ts_==TS_STOPFINISH);
	}
};
#endif
