#ifndef _FRAME_BUFFERH_
#define _FRAME_BUFFERH_
#include <opencv2/opencv.hpp>
#include <memory>
#include <deque>
#include "VLCCore.h"
#include <boost/thread/thread.hpp>
#include "VLCCore.h"
#include "../include/common/DQueue.h"
#include "../include/common/Queue.h"
#include "../include/common/Base.h"
#include "../include/common/ThreadStatu.h"
#include "DD.h"

using namespace cv;

#define PLATEIMAGE

class FrameBuffer{
public:
	static int BUFFERSIZE;
private:
#ifdef PLATEIMAGE
	Mat plateg_;
	Mat plater_;
	Mat platew_;
#endif
#ifdef USEMR
	Mat2Rtmp *mr_;
#endif
	SStatu dstatu_;


	DQueue<MNode> mqueue_;
	GenFrameFun genFrameFun_;
	VLCCore *vlccore_;

	boost::thread *thGet_;
	boost::thread *thPop_;
	ThreadStatu tsGet_;
	ThreadStatu tsPop_;
	int64_t crId_;
	DD *dd_;
	boost::mutex mutex_;
	int64_t curPlayTm_;
	void SetCurPlayTm(int64_t num){
		boost::mutex::scoped_lock lock(mutex_);
		curPlayTm_ = num;	
	}
	
public:
	FrameBuffer() {
		mqueue_.Clear();
		delete vlccore_;
		delete thGet_;
		delete thPop_;
		delete dd_;
	}
	int64_t GetCurPlayTm(){
		boost::mutex::scoped_lock lock(mutex_);
		return curPlayTm_;	
	}

private:
	void ShowOffline(){
		dstatu_.SetStatu(false);// 20170516 add, fix bug
		Mat mm(Size(1280,720), CV_8UC3);
		putText(mm, "offline", Point(10, 180),CV_FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255, 0, 0)); 
		genFrameFun_(mm, crId_);
		usleep(100*1000);
	}
	void Get(){
		int fcount = 0;
		while(1){
			if(tsGet_.IsStopping()){
				break;
			}
			MNode frame;
			bool statu = false;
			if(vlccore_->IsExit()){
				ShowOffline();
				if(tsGet_.IsStopping()){
					break;
				}
				continue;
			}
#ifndef QUEUEWAIT
			vlccore_->GetFrame(frame);
			statu = true;
#else
			vlccore_->GetFrame(frame, statu);
#endif
			if(!statu){
				if(fcount > 1){
					ShowOffline();
					continue;
				}
				fcount++;
				continue;
			}
			fcount = 0;
			frame->fb_ = (void*)this;
			// add to detect thread
			// add to queue and pop queue head
			mqueue_.Push(frame);
		}
		LOG(INFO) << "[--"
			<< crId_
			<< "--]set get stop finish\n";
		tsGet_.SetStopFinish();
	}
	void Pop(){
		bool lastHas = false;
		int count = 0;
		while(1){
			if(tsPop_.IsStopping()){
				break;	
			}
			MNode tmp;
			bool statu = false;
#ifndef QUEUEWAIT
			mqueue_.PopFront(tmp, BUFFERSIZE, cr_->GetId());
			statu = true;
#else
			mqueue_.PopFront(tmp, BUFFERSIZE, crId_, statu);
#endif
			if(!statu){
				continue;
			}
#ifdef LOGM
			LOG(INFO) << "[Pop--"
				<< crId_
				<< "--"
				<< tmp->num_
				<< "]Pop--------------count:"
				<< mqueue_.Count()
				<< ", tm:"
				<< gtmstr().c_str();
#endif
			SetCurPlayTm(tmp->tm_);
			std::vector<std::string> words = tmp->GetWords();
			for(int i = 0; i < words.size(); i++){
				lastHas = true;
				std::string &word = words[i];
				this->DrawFrame(tmp->mat_, word);
			}
			genFrameFun_(tmp->mat_, crId_);
		}
		LOG(INFO) << "[--"
			<< crId_
			<< "--]set pop stop finish\n";
		tsPop_.SetStopFinish();
	}
	void DrawFrame(Mat &frame, const std::string &word){
		// to do
	}
public:
	int64_t GetPos(){
		return crId_;
	}
	FrameBuffer(const GenFrameFun &genFrameFun, 
		const std::string &url, int64_t crId){ 
		thPop_ = NULL;
		thGet_ = NULL;
		genFrameFun_ = genFrameFun;
		vlccore_ = new VLCCore(url, crId, genFrameFun);
		crId_ = crId;
		dd_ = new DD(crId_, &mqueue_, &dstatu_);
		curPlayTm_ = 0;
	}
	void Start(){
		vlccore_->Start();
		thGet_ = new boost::thread(boost::bind(&FrameBuffer::Get, this));
		thPop_ = new boost::thread (boost::bind(&FrameBuffer::Pop, this));
		dd_->Start();	
	}
	~FrameBuffer(){
		delete vlccore_;
		delete thGet_;
		delete thPop_;
		delete dd_;
	}
	void Stop() {
		LOG(INFO) << "[--" << crId_ << "--]framebuffer begin stop all\n";
		vlccore_->Stop();
		LOG(INFO) << "[--" << crId_ << "--]sstopped vlccore thread\n";

		tsGet_.SetStopping();
		tsGet_.WaitStopFinish();
		LOG(INFO) << "[--" << crId_ << "--]sstopped get thread\n";

		LOG(INFO) << "begin stop dd";

		dd_->Stop();
		LOG(INFO) << "finish stop dd";


		LOG(INFO) << "finish stop tt";


		LOG(INFO) << ("begin stop pop");
		tsPop_.SetStopping();
		tsPop_.WaitStopFinish();
		LOG(INFO) << ("end stop pop");
		LOG(INFO) << "[--" << crId_ << "--]stopped pop thread\n";

	}

};
#endif
