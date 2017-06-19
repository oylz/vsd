#ifndef _BASEH_
#define _BASEH_

#include <stdio.h>
#include <opencv2/opencv.hpp>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <time.h>

static int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;

	return (0);
}
static void usleep(int64_t us) {
	int64_t s = us / 1000;
	Sleep(s);
}

#else
#include <sys/time.h>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#ifdef INUI
#include <QMainWindow>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <map>
#include <string>
#include <QPixmap>
#include <QPushButton>
#endif
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 
#ifdef _DEBUG  
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif
#endif
using namespace cv;
#ifdef OPENCV24
typedef Rect Rect2d;
#endif



static std::string toStr(int in){
	char chr[20] = {0};
	sprintf(chr, "%d", in);
	std::string re(chr);
	return re;
}
static std::string toStr(int64_t in){
	char chr[20] = {0};
	sprintf(chr, "%lld", in);
	std::string re(chr);
	return re;
}
static std::string to3dStr(int in){
	char chr[20] = {0};
	sprintf(chr, "%03d", in);
	std::string re(chr);
	return re;
}
static int toInt(const std::string &in){
	int re = 0;
	sscanf(in.c_str(), "%d", &re);
	return re;
}
static int64_t toIntL(const std::string &in){
	int64_t re = 0;
	sscanf(in.c_str(), "%lld", &re);
	return re;
}
static std::string toStr(float in){
	char chr[20] = {0};
	sprintf(chr, "%f", in);
	std::string re(chr);
	return re;
}
static float toFloat(const std::string &in){
	float re = 0;
	sscanf(in.c_str(), "%f", &re);
	return re;
}
static void splitStr(const std::string& inputStr, const std::string &key, std::vector<std::string>& outStrVec) {  
	if(inputStr == ""){
		return;
	}
	int pos = inputStr.find(key);
	int oldpos = 0;
	if(pos > 0){
		std::string tmp = inputStr.substr(0, pos);
		outStrVec.push_back(tmp);
	}
	while(1){
		if(pos < 0){
			break;
		}
		oldpos = pos;
		int newpos = inputStr.find(key, pos + key.length());
		std::string tmp = inputStr.substr(pos + key.length(), newpos - pos - key.length());
		outStrVec.push_back(tmp);
		pos = newpos;
	}
	int tmplen = 0;
	if(outStrVec.size() > 0){
		tmplen = outStrVec.at(outStrVec.size() - 1).length();
	}
	if(oldpos+tmplen < inputStr.length()-1){
		std::string tmp = inputStr.substr(oldpos + key.length());
		outStrVec.push_back(tmp);
	}
} 

static std::string rplStr(std::string &str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

static std::string trim(std::string &s){  
    if (s.empty()){  
        return s;  
    }  
  
    s.erase(0,s.find_first_not_of(" "));  
    s.erase(s.find_last_not_of(" ") + 1);  
    return s;  
}  

static int64_t gtm(){
	struct timeval tm;
	gettimeofday(&tm, 0);
	int64_t re = ((int64_t)tm.tv_sec)*1000*1000 + tm.tv_usec;
	return re;
}
static std::string gtmstr(){
	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);
	return buf;
}
static std::string gtmstr(int64_t inusec){
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];
	int64_t sec = inusec/1000000;
	int64_t usec = inusec%1000000;
	nowtime = sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06lld", tmbuf, usec);
	return buf;
}

static double _SameThreshold = 0.5;


struct MNodeD{
        Mat mat_;
		int64_t num_;
		int64_t nextNum_;
		int64_t tm_;
	void *fb_;
	int64_t pkt_;
        MNodeD(){
                num_ = 0;
                nextNum_ = 0;
		tm_ = 0;
		fb_ = NULL;
		pkt_ = 0;
        }
        ~MNodeD(){
        }
		void SetWords(const std::vector<std::string> &words) {
			boost::mutex::scoped_lock lock(mutex_);
			std::copy(words.begin(), words.end(), std::back_inserter(words_));
		}
		std::vector<std::string> &GetWords() {
			return words_;
		}
private:
	boost::mutex mutex_;
	std::vector<std::string> words_;
};
typedef boost::shared_ptr<MNodeD> MNode;
typedef void (*GenFrameFun)(Mat &frame, int64_t num);

#endif
