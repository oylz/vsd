#ifndef _QUEUEH_
#define _QUEUEH_
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

template <typename T>
class Queue{
private:
	std::queue<T> queue_;
	boost::mutex mutex_;
	boost::condition_variable cv_;
public:
	void Push(const T &t){
		boost::mutex::scoped_lock lock(mutex_);
		queue_.push(t);
		lock.unlock();
		cv_.notify_one();
	}
#ifndef QUEUEWAIT
	void Pop(T &t){
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.empty()){
			cv_.wait(lock);
		}	
		t = queue_.front();
		queue_.pop();
	}
	bool TryPop(T &t){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.empty()){
			return false;
		}
		t = queue_.front();
		queue_.pop();
		return true;
	}
	void PopList(int size, std::queue<T> &out){
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.empty()){
			cv_.wait(lock);
		}
		for(int i = 0; i < size; i++){
			T t = queue_.front();
			queue_.pop();
			out.push(t);
			if(queue_.empty()){
				break;
			}
		}
	}
#else
	void Pop(T &t, bool &statu){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.empty()){
			cv_.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(500));
		}	
		if(queue_.empty()){
			statu = false;
			return;
		}
		t = queue_.front();
		queue_.pop();
		statu = true;
	}
	bool TryPop(T &t){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.empty()){
			return false;
		}
		t = queue_.front();
		queue_.pop();
		return true;
	}
	void PopList(int size, std::queue<T> &out, bool &statu){
		boost::mutex::scoped_lock lock(mutex_);
		if(queue_.empty()){
			cv_.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(500));
		}
		if(queue_.empty()){
			statu = false;
			return;
		}
		for(int i = 0; i < size; i++){
			T t = queue_.front();
			queue_.pop();
			out.push(t);
			if(queue_.empty()){
				break;
			}
		}
		statu = true;
	}
#endif
	void PushCap(int cap, const T&t){
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.size() > cap){
			queue_.pop();
		}
		queue_.push(t);
		cv_.notify_one();
	}
	int Count(){
		boost::mutex::scoped_lock lock(mutex_);
		return queue_.size();
	}
	void Clear() {
		boost::mutex::scoped_lock lock(mutex_);
		while (!queue_.empty()) {
			queue_.pop();
		}
	}
};
#endif
