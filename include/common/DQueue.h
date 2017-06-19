#ifndef _DQUEUEH_
#define _DQUEUEH_
#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>


template <typename T>
class DQueue{
private:
	std::deque<T> queue_;
	boost::mutex mutex_;
	boost::condition_variable cv_;
public:
	void Push(const T &t){
		boost::mutex::scoped_lock lock(mutex_);
		queue_.push_back(t);
		cv_.notify_one();
	}	
#ifndef QUEUEWAIT
	void PopFront(T &t, int size, int pos){
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.size() <= size){
			cv_.wait(lock);
		}
		t = queue_.front();
		queue_.pop_front();
	}
	void GetTail(T &t, int pos){
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.empty()){//queue_.size()<=24){//queue_.empty()){
			cv_.wait(lock);
		}
		t = queue_.back();
	}	
#else
	void TryPopFront(T &t, bool &statu) {
		boost::mutex::scoped_lock lock(mutex_);
		int count = queue_.size();
		if (count == 0) {
			statu = false;
			return;
		}
		t = queue_.front();
		if(count > 1){
			queue_.pop_front();
		}
		statu = true;
	}

	void PopFront(T &t, int size, int pos, bool &statu){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.size() <= size){
			cv_.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(500));
		}
		if(queue_.size() <= size){
			statu = false;
			return;
		}
		t = queue_.front();
		queue_.pop_front();
		statu = true;
	}
	void GetTail(T &t, int pos, bool &statu){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.empty()){//queue_.size()<=24){//queue_.empty()){
			cv_.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(500));
		}
		if(queue_.empty()){
			statu = false;
			return;
		}
		t = queue_.back();
		statu = true;
	}	
#endif
	int Count(){
		boost::mutex::scoped_lock lock(mutex_);
		return queue_.size();
	}
	void Clear() {
		boost::mutex::scoped_lock lock(mutex_);
		queue_.clear();
	}
};
#endif
