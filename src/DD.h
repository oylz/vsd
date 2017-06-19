#ifndef _DDH_
#define _DDH_

struct SStatu{
private:
	bool statu_;
	boost::mutex mutex_;
public:
	SStatu(){
		statu_ = false;
	}
	void SetStatu(bool statu){
		boost::mutex::scoped_lock lock(mutex_);
		statu_ = statu;	
	}
	bool GetStatu(){
		boost::mutex::scoped_lock lock(mutex_);
		return statu_;
	}
};

class DD{
public:
	static std::string landmarkIp_;
private:
	boost::thread *thDetect_;
	ThreadStatu tsDetect_;
	DQueue<MNode> *mqueue_;
	int64_t crId_;
	SStatu *dstatu_;

public:
	~DD() {
		delete thDetect_;
		mqueue_->Clear();
	}
	DD(int64_t crId, DQueue<MNode> *mqueue, SStatu *dstatu){
		crId_ = crId;
		mqueue_ = mqueue;
		dstatu_ = dstatu;
		thDetect_ = NULL;
	}
	void Start(){
		thDetect_ = new boost::thread(boost::bind(&DD::Detect, this));
	}
	void Stop(){
		tsDetect_.SetStopping();
		tsDetect_.WaitStopFinish();
		LOG(INFO) << "[--" << crId_ << "--]sstopped detect thread\n";
		LOG(INFO) << "[--" << crId_ << "--]closed lm\n";
	
	}

private:
	bool MqueueGetTail(int64_t &lastNum, MNode &mnode){
		while(1){
			if(tsDetect_.IsStopping()){
				return false;
			}
			bool statu = false;
#ifndef QUEUEWAIT
			mqueue_->GetTail(mnode, cr_->GetId());
			statu = true;
#else
			mqueue_->GetTail(mnode, crId_, statu);
#endif
			if(statu){
				if(lastNum == mnode->num_){ // for camera offline time range: the tail is always the same frame
					continue;
				}
				lastNum = mnode->num_;
				break;
			}
			//if(tsDetect_.IsStopping()){
			//	return false;
			//}
		}
		return true;
	}
	void Detect(){
		MNode mnode;
		int64_t lastNum = 0;
		if(!MqueueGetTail(lastNum, mnode)){
			tsDetect_.SetStopFinish();
			return;
		}
		while(1){
			if(tsDetect_.IsStopping()){
				break;
			}
			Mat &frame = mnode->mat_;
			int64_t t1 = gtm();
#ifdef LOGM
			LOG(INFO) << "[Detect--" << crId_ <<
				"--" << mnode->num_ << "]Detect---beg-----------num:" << mnode->num_;
#endif
			//Mat mmm;
			//resize(frame, mmm, Size(frame.cols/2, frame.rows/2), 0, 0, INTER_NEAREST);
			Mat &nnn = frame;//mmm.clone();
			std::vector<unsigned char> frame_vect;
			if (!cv::imencode(".jpg", nnn, frame_vect)) {
				return;
			}   
			std::string imgs = std::string(reinterpret_cast<char *>(frame_vect.data()),
								frame_vect.size());
			std::vector<std::string> words;
			// to do get words from frame
			bool re = false;
			if(!re){
				if(!MqueueGetTail(lastNum, mnode)){
					break;
				}
				dstatu_->SetStatu(false);
				continue;
			}
			dstatu_->SetStatu(true);
			mnode->SetWords(words);
			int64_t t2 = gtm();
			LOG(INFO) << "[Detect--" << crId_
				<< "--" << mnode->num_
				<<"]Detect---end-----------num:"
				<< mnode->num_
				<< ", casttime:"
				<< t2 - t1;
			MNode mn;
			//mqueue_.GetTail(mn, cr_->GetId());
			if(!MqueueGetTail(lastNum, mn)){
				break;
			}
			mnode->nextNum_ = mn->num_;

			//long t3 = gtm();
			mnode = mn;

		}		
		LOG(INFO) << "[--"
			<< crId_
			<< "--]set detect stop finish\n";
		tsDetect_.SetStopFinish();
	
	}
	
};

#endif
