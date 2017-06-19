#ifndef _FBMAPH_
#define _FBMAPH_
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "common/Queue.h"
#include "common/Base.h"

class FrameBuffer;

struct Job {
	bool start_;
	int64_t crId_;
	std::string url_;
	Job(bool start, int64_t crId, const std::string &url) {
		start_ = start;
		crId_ = crId;
		url_ = url;
	}
};
typedef boost::shared_ptr<Job> JobP;

class FBMap{
private:

public:
	static boost::shared_ptr<FBMap> Instance();
	void Init(const GenFrameFun &gff, const std::string &conffile);
	~FBMap();
public:
	void Start();
	void StartAll(const std::vector<JobP> &jobs);
	void StopAll();
	int64_t GetCurPlayTm(void *fb);
	int64_t GetBufferSize();
private:
	void StartCore();
	FrameBuffer *GetFB(int64_t id);
    	void StartFB(int64_t crId, const std::string &url);
    	void StopFB(int64_t crId);

private:
	static boost::shared_ptr<FBMap> self_;
	std::map<int64_t, FrameBuffer*> fbs_;
	boost::mutex mutex_;
	boost::thread *th_;
	Queue<JobP> jobs_;
	GenFrameFun genFrameFun_;
};



#endif
