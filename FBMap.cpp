#include "include/FBMap.h"
#include "src/FrameBuffer.h"
#include "include/common/DParam.h"

std::string DD::landmarkIp_ = "";
boost::shared_ptr<DParam> DParam::self_;
int FrameBuffer::BUFFERSIZE = 25;
boost::shared_ptr<FBMap> FBMap::self_;

boost::shared_ptr<FBMap> FBMap::Instance(){
	if(self_.get() == NULL){
		self_ = boost::shared_ptr<FBMap>(new FBMap());
	}
	return self_;
}
FBMap::~FBMap() {
	std::map<int64_t, FrameBuffer*>::iterator it;
	for (it = fbs_.begin(); it != fbs_.end(); ++it) {
		delete it->second;
	}
	delete th_;
}
void FBMap::Init(const GenFrameFun &gff, 
	const std::string &conffile){
	LOG(ERROR) << "FBMap::Init1";
	genFrameFun_ = gff;

	DParam::Instance()->Init(conffile);
	DD::landmarkIp_ = DParam::Instance()->GetDetectIp();
	std::vector<std::string> files = DParam::Instance()->GetFiles();
	for(int i = 0; i < files.size(); i++){
		LOG(INFO) << "file:"
			<< files[i].c_str();
	}
	double threshold = DParam::Instance()->GetThreshold();
	FrameBuffer::BUFFERSIZE = DParam::Instance()->GetFrameBufferSize();
	LOG(ERROR) << "FBMap::Init2";
	LOG(ERROR) << "FBMap::Init3";
	VLCCore::Init();
	LOG(ERROR) << "FBMap::Init6";
}
void FBMap::Start(){
	th_ = new boost::thread(boost::bind(&FBMap::StartCore, this));
}
void FBMap::StartAll(const std::vector<JobP> &jobs){
	for(int i = 0; i < jobs.size(); i++){		
		jobs_.Push(jobs[i]);
	}
}
void FBMap::StopAll(){
	boost::mutex::scoped_lock lock(mutex_);
	std::map<int64_t, FrameBuffer*>::iterator it;
	for (it = fbs_.begin(); it != fbs_.end(); ++it) {
		LOG(INFO) << "STOP CAMERA:"
			<< it->first;
		FrameBuffer *fb = it->second;
		fb->Stop();
		fbs_.erase(it);
		delete fb;
	}
}	
void FBMap::StartCore(){
	while(1){
		bool statu = false;
		JobP job;
#ifndef QUEUEWAIT
		jobs_.Pop(job);
		statu = true;
#else
		jobs_.Pop(job, statu);	
#endif
		if(!statu){
			continue;
		}	
		if(job->start_){
			this->StartFB(job->crId_, job->url_);	
			continue;
		}
		this->StopFB(job->crId_);
	}		
}
FrameBuffer *FBMap::GetFB(int64_t id){
	boost::mutex::scoped_lock lock(mutex_);
	std::map<int64_t, FrameBuffer*>::iterator it;
	it = fbs_.find(id);
	if (it == fbs_.end()) {
		return NULL;
	}
	return it->second;
}


void GenFrameFunInFBMap(Mat &frame, int64_t num){
	
}
void FBMap::StartFB(int64 crId, const std::string &url){
	FrameBuffer *fb = new FrameBuffer(GenFrameFunInFBMap, url, crId);
	{
		boost::mutex::scoped_lock lock(mutex_);
		fbs_.insert(std::make_pair(crId, fb));
	}
	fb->Start();
}
void FBMap::StopFB(int64_t crId){
	boost::mutex::scoped_lock lock(mutex_);
	int64_t id = crId;
	std::map<int64_t, FrameBuffer*>::iterator it = fbs_.find(id);
	if(it == fbs_.end()){
		return;
	}
	FrameBuffer *fb = it->second;
	fb->Stop();
	fbs_.erase(it);
	delete fb;
}
int64_t FBMap::GetCurPlayTm(void *fb) {
	FrameBuffer *fbc = (FrameBuffer*)(fb);
	return fbc->GetCurPlayTm();
}
int64_t FBMap::GetBufferSize() {
	return FrameBuffer::BUFFERSIZE;
}
