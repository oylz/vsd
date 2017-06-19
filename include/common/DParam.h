#ifndef _DPARAMH_
#define _DPARAMH_
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <exception>
#include <glog/logging.h>

using namespace boost::property_tree; 

class DParam{
public:
	~DParam() {
		pt_.clear();
	}
	static boost::shared_ptr<DParam> Instance(){
		if(self_.get() == NULL){
			self_ = boost::shared_ptr<DParam>(new DParam());
		}
		return self_;
	}
	bool Init(const std::string conffile){
		std::string content;
		Read(conffile, content);
		LOG(INFO) << "content:" << content.c_str();
		std::stringstream ss(content);  
  		try{      
			read_json(ss, pt_);  
  		}  
  		catch(ptree_error & e) {  
			LOG(ERROR) << "parse json error\n";
    			return false;   
  		}  		
		return true;
	}
	std::string GetDetectIp(){
		std::string re = GetStr("detectip");
		return re;
	}
	std::string GetMatchIp(){
		std::string re = GetStr("matchip");
		return re;
	}
	std::vector<std::string> GetFiles(){
		std::vector<std::string> re;
		std::string line = GetStr("files");
		std::vector<std::string> cols;
		splitStr(line, ";", cols);
		for(int i = 0; i < cols.size(); i++){
			std::string tmp = cols[i];	
			tmp = trim(tmp);
			if(tmp == ""){
				continue;
			}
			//tmp = "file://" + tmp;
			re.push_back(tmp);
		}
		return re;
	}
	double GetThreshold(){
		double re = GetDouble("threshold");
		return re;
	}
	int GetFrameBufferSize(){
		int re = GetInt("framebuffersize");
		return re;
	}
	std::string GetGImage() {
		std::string re = GetStr("gimage");
		return re;
	}
	std::string GetRImage() {
		std::string re = GetStr("rimage");
		return re;
	}
	std::string GetWImage() {
		std::string re = GetStr("wimage");
		return re;
	}
	int GetMaxPersonNo() {
		int re = GetInt("maxpersonno");
		return re;
	}
private:
	std::string GetStr(const std::string &key){
		std::string re;
		try{
			re = pt_.get<std::string>(key);
		}
		catch(std::exception &e){
			std::string err(e.what());
			LOG(ERROR) << "get " << key << "error:" << err;
			exit(0);
		}
		return re;
	}
	int GetInt(const std::string &key){
		int re = 0;
		try{
			re = pt_.get<int>(key);
		}
		catch(std::exception &e){
			std::string err(e.what());
			LOG(ERROR) << "get " << key << "error:" << err;
			exit(0);
		}
		return re;
	}
	double GetDouble(const std::string &key){
		double re = 0;;
		try{
			re = pt_.get<double>(key);
		}
		catch(std::exception &e){
			std::string err(e.what());
			LOG(ERROR) << "get " << key << "error:" << err;
			exit(0);
		}
		return re;
	}
	void Read(const std::string &conffile, std::string &content){
		FILE *fl = fopen(conffile.c_str(), "rb");
		fseek(fl, 0, SEEK_END);
		int len = ftell(fl);
		LOG(INFO) << "len:" << len;
		fseek(fl, 0, SEEK_SET);
		char *buf = new char[len+1];
		memset(buf, 0, len+1);
		fread(buf, 1, len, fl);
		content = std::string(buf);
		delete []buf;
		fclose(fl);
	}

private:
	DParam(){
	}
	ptree pt_;
	static boost::shared_ptr<DParam> self_;
};

#endif
