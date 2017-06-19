#ifndef _VLCCOREH_
#define _VLCCOREH_

#include "../include/common/Base.h"
#include <memory>
#include <boost/shared_ptr.hpp>
#include "../include/common/Queue.h"
#include <boost/thread/thread.hpp>

extern "C" {
#include <libavutil/opt.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}
#include "../include/common/ThreadStatu.h"




static int ICB(void *ctx);
class VLCCore{
public:
	int64_t GetDev(int64_t pkt){
		int64_t now = av_gettime() - startTime_;
		int64_t dev = pkt - now;
		return dev;
	}
        VLCCore(const std::string &url, int64_t pos, GenFrameFun fun){
                th_ = NULL;
                num_ = 0;
                url_ = url;
                pos_ = pos;
                fun_ = fun;
		lastReadTm_ = 0;
		isRead_ = false;
		startTime_ = 0;
        }
        int64_t pos_;
        GenFrameFun fun_;
	int64_t lastReadTm_;
	bool isRead_;
private:
	AVFormatContext *input_ctx_ = nullptr;
	AVCodecContext *decode_ctx_ = nullptr;
	AVFrame *avframe_bgr_ = nullptr;
	AVFrame *avframe_ = nullptr;
	SwsContext *sws_ctx_ = nullptr;

        Queue<MNode> frames_;
        boost::thread *th_;
		int64_t num_;
        std::string url_;
	ThreadStatu ts_;
	int64_t startTime_;
	Mat GetCalibrationImg(Mat &src)  
	{
		CvPoint2D32f center;  
		center.x = float(src.cols/2);  
		center.y = float(src.rows/2);  

		int degree = -90;//(int)(double)(-90*180/CV_PI);
		Mat M = getRotationMatrix2D(center, degree, 1);  
  
		// rotate  
		Mat dst;  
		warpAffine(src, dst, M, cvSize(src.cols, src.rows), CV_INTER_LINEAR);  
	
		return dst;  
	} 
public:
        MNode AddFrame(Mat &frame, int64_t pkt){
		MNodeD *mnd = new MNodeD();	
                MNode mnode(mnd);
#ifdef  USEMATREF
                mnode->mat_ = frame;//frame.clone();//GetCalibrationImg(frame).clone();
#else
				mnode->mat_ = frame.clone();
#endif
                mnode->num_ = num_++;
		{
					frame.release();
		}
		mnode->tm_ = gtm();
		mnode->pkt_ = pkt;
                frames_.Push(mnode);
		return mnode;
        }
#ifndef QUEUEWAIT
	void GetFrame(MNode &re){
		frames_.Pop(re);
	}
#else
        void GetFrame(MNode &re, bool &statu){
			//LOG(INFO) << "frames_.Pop before, size is:" << frames_.Count();
                frames_.Pop(re, statu);
        }
#endif
public:
	~VLCCore() {
		frames_.Clear();
		delete th_;
	}
	static void Init(){
		av_register_all();
		avformat_network_init();
	}
        void Start(){
		if(th_){ delete th_;
		}
		if(ts_.IsStopping()){// 20170516 for rescan
			ts_.SetStopFinish();
			return;
		}
		ts_.Reset();
                th_ = new boost::thread(boost::bind(&VLCCore::DoCore, this));
        }
	void Stop(){
		LOG(INFO) << "vlccore stop 1\n";
		ts_.Print();
		if(IsExit()){
			LOG(INFO) << "vlccore stop 2\n";
			return;
		}
		ts_.SetStopping();
		LOG(INFO) << "vlccore stop 3\n";
		ts_.WaitStopFinish();
		LOG(INFO) << "vlccore stop 4\n";
	}
	bool IsExit(){
		return ts_.IsExit();
	}
private: 
	void Beg(){
                lastReadTm_ = gtm();
                isRead_ = true;
	}
	void End(){
		isRead_ = false;
	}
	void DoCore(){
		LOG(INFO) << "VLCCORE:DoCore:url" << url_.c_str();
		if(url_.empty()){
			return;
		}
		bool isFile = false;
		std::string head = "file://";
		if (url_.length() > head.length()) {
			if (url_.substr(0, head.length()) == head) {
				isFile = true;
			}
		}
		while (1) {//begin while
			if (ts_.IsStopping()) {
				LOG(ERROR) << ("VLCCORE: NNN\n");
				Release();
				break;
			}
			std::string infile = url_.c_str();
			if (isFile) {
				infile = url_.substr(head.length());
			}

			int stream_index = 0;
			int video_width = 0;
			int video_height = 0;
			

			int ret;
			input_ctx_ = avformat_alloc_context();
			if (!input_ctx_) {
				LOG(ERROR) << ("VLCCORE: AAA\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
			{
				input_ctx_->interrupt_callback.callback = ICB;
				input_ctx_->interrupt_callback.opaque = (void*)this;
			}

			AVDictionary *opts = nullptr;
			ret = av_dict_set(&opts, "rtsp_transport", "tcp", 0);
			if (ret != 0) {
				LOG(ERROR) << ("VLCCORE: BBB\n");
				//return;
				Release();
				continue;
			}
			Beg();
			ret = avformat_open_input(&input_ctx_, infile.c_str(), NULL, &opts);
			End();
			if (ret != 0) {
				LOG(ERROR) << "VLCCORE: CCC("
					<< infile.c_str();
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
			Beg();
			ret = avformat_find_stream_info(input_ctx_, NULL);
			End();
			if (ret < 0) {
				LOG(ERROR) << ("VLCCORE: DDD\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}

			AVStream *video_stream = nullptr;
			Beg();
			ret = av_find_best_stream(input_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
			End();
			if (ret < 0) {
				LOG(ERROR) << ("VLCCORE: EEE\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}

			stream_index = ret;
			video_stream = input_ctx_->streams[ret];
			{
				AVRational fr = video_stream->r_frame_rate;
				long sleepus = fr.den*1000*1000/fr.num;
			}
			AVCodec *codec = NULL;

			//³¢ÊÔÓÃnvidia½âÂë
#if 0
			if (video_stream->codecpar->codec_id == AV_CODEC_ID_H264) {
			codec = avcodec_find_decoder_by_name("h264_qsv");
			}
#else
			if (!codec) {
#ifdef FFMPEG33
                       	 	//codec = avcodec_find_decoder_by_name("h264_cuvid");
                        	if(!codec){
					//LOG(ERROR) << ("find h264_cuvid decoder error!\n");
					//exit(0);
                                	codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
                        	}
#else
                        	//codec = avcodec_find_decoder_by_name("h264_qsv");//cuvid");
                        	if(!codec){
                                	codec = avcodec_find_decoder(video_stream->codec->codec_id);
                        	}
#endif
			}
#endif
			if (!codec) {
				LOG(ERROR) << ("VLCCORE: FFF\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
#ifdef FFMPEG33
			decode_ctx_ = avcodec_alloc_context3(codec);
			if (!decode_ctx_) {
				LOG(ERROR) << ("VLCCORE: GGG\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
			ret = avcodec_parameters_to_context(decode_ctx_, video_stream->codecpar);
			if (ret < 0) {
				LOG(ERROR) << ("VLCCORE: HHH\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
#else
			decode_ctx_ = video_stream->codec;
#endif
			ret = avcodec_open2(decode_ctx_, codec, nullptr);
			if (ret < 0) {
				LOG(ERROR) << ("VLCCORE: III\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
#ifdef FFMPEG33
			video_width = video_stream->codecpar->width;
			video_height = video_stream->codecpar->height;
#else
			video_width = video_stream->codec->width;
			video_height = video_stream->codec->height;
#endif
			avframe_bgr_ = av_frame_alloc();
			if (!avframe_bgr_) {
				LOG(ERROR) << ("VLCCORE: JJJ\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}

			/*ret = av_image_alloc(avframe_bgr->data, avframe_bgr->linesize, video_width,
				video_height, AV_PIX_FMT_BGR24, 1);
			if (ret < 0) {
				return;
			}
			*/
			std::vector<uint8_t> framebuf(avpicture_get_size(AV_PIX_FMT_BGR24, video_width, video_height));
			avpicture_fill(reinterpret_cast<AVPicture*>(avframe_bgr_), framebuf.data(), AV_PIX_FMT_BGR24, video_width, video_height);

			avframe_ = av_frame_alloc();
			if (!avframe_) {
				LOG(ERROR) << ("VLCCORE: KKK\n");
				//ts_.SetStopFinish();
				//return;
				Release();
				continue;
			}
			AVPacket packet;
			av_init_packet(&packet);
			bool isErr = false;
			startTime_ = av_gettime();
			AVRational timeBase = video_stream->time_base;
			AVRational baseq = {1, AV_TIME_BASE};
			while (true) {
				if(ts_.IsStopping()){
					LOG(ERROR) << ("VLCCORE: LLL\n");
					av_packet_unref(&packet);
					av_free_packet(&packet);

					break;
				}
				Beg();
				int ret = av_read_frame(input_ctx_, &packet);
				End();
				if (ret != 0) {
					isErr = true; 
					LOG(ERROR) << ("av_read_frame error\n");
					av_packet_unref(&packet);
					av_free_packet(&packet);

					break;
				}

				if (packet.stream_index != stream_index) { // packet is not video
					av_packet_unref(&packet);
					av_free_packet(&packet);
					continue;
				}
#if 0
				ret = avcodec_send_packet(decode_ctx, &packet);
				if (ret == AVERROR(EAGAIN)) {
					continue;
				}
				else if (ret != 0) {
					break;
				}

				ret = avcodec_receive_frame(decode_ctx, avframe);
				if (ret != 0) {
					if (ret == AVERROR(EAGAIN)) {
						continue;
					}
					else {
						break;
					}
				}
#else
				int got_pic = 0;
				int64_t tm1 = gtm();
				avcodec_decode_video2(decode_ctx_, avframe_, &got_pic, &packet);
				int64_t tm2 = gtm();
				if (!got_pic) {
					LOG(ERROR) << "avcodec_decode_video2 error";
					av_packet_unref(&packet);
					av_free_packet(&packet);
					continue;
				}
#endif

				if (!sws_ctx_) {
					std::string pfn = av_get_pix_fmt_name((AVPixelFormat)avframe_->format);
					LOG(INFO) << "avframe->format:"
						<< pfn.c_str() << "video_width:" << video_width << "video_height:" << video_height;
					sws_ctx_ = sws_getContext(video_width, video_height,
						static_cast<enum AVPixelFormat>(avframe_->format),
						video_width, video_height, AV_PIX_FMT_BGR24,
						SWS_FAST_BILINEAR,//SWS_BICUBIC, 
						NULL, NULL, NULL);
					LOG(INFO) << "SWD_CTX END";
				}
				if (!sws_ctx_) {
					isErr = true; 
					LOG(ERROR) << "sws_ctx is null";
					av_packet_unref(&packet);
					av_free_packet(&packet);
					break;
				}
				//LOG(INFO) << "DoCore-----383";
				int64_t tm3 = gtm();
				if (sws_scale(sws_ctx_, avframe_->data, avframe_->linesize, 0, video_height,
					avframe_bgr_->data, avframe_bgr_->linesize) <= 0) {
					isErr = true; 
					LOG(ERROR) << "sws_scale error";
					av_packet_unref(&packet);
					av_free_packet(&packet);
					break;
				}
				int64_t tm4 = gtm();
				cv::Mat image(video_height, video_width, CV_8UC3, framebuf.data(), avframe_bgr_->linesize[0]);
				int64_t tm5 = gtm();
				//	tm1, tm2, tm2-tm1, tm3, tm4, tm4-tm3, tm4, tm5, tm5-tm4, tm5-tm1);
				int64_t pkt = av_rescale_q(packet.pts, timeBase, baseq);
				//LOG(INFO) << "add frame:" << num_;
				MNode mnode = this->AddFrame(image, pkt);
				av_packet_unref(&packet);
				av_free_packet(&packet);
				if(isFile){
					//int64_t pkt = av_rescale_q(packet.pts, timeBase, baseq);
					//int64_t now = av_gettime() - startTime_;
					//int64_t dev = pkt - now;
					int64_t dev = GetDev(pkt);
					if(dev > 0){
						av_usleep(dev);
					}
				}
			}
			Release();
#if 0
			if(isErr){
				if(ts_.IsStopping()){// 20170516 for rescan
					LOG(INFO) << ("----set stop vlccore finish\n");
					ts_.SetStopFinish();
					return;
				}
				this->Start();
				return;
			}
#endif
			if(ts_.IsStopping()){
				LOG(ERROR) << ("VLCCORE: MMM\n");
				break;
			}
			if(!isFile){
				break;
			}
			LOG(INFO) << "reread file again!(cur num_:" << num_ << ")";
		}// end while
		LOG(INFO) << ("set stop vlccore finish\n");
		ts_.SetStopFinish();
	}
	void Release() {
		if (sws_ctx_) {
			sws_freeContext(sws_ctx_);
			sws_ctx_ = nullptr;
		}
		if (avframe_) {
			av_frame_free(&avframe_);
			avframe_ = nullptr;
		}
		if (avframe_bgr_) {// 2017.06.16
			av_frame_free(&avframe_bgr_);
			avframe_bgr_ = nullptr;
		}
#ifdef FFMPEG33
		if (decode_ctx_) {
			avcodec_free_context(&decode_ctx_);
			decode_ctx_ = nullptr;
		}
#endif
		if (input_ctx_) {
			avformat_close_input(&input_ctx_);
			avformat_free_context(input_ctx_);
			input_ctx_ = nullptr;
		}

	}
};

static int ICB(void *ctx){
	return 0;

	VLCCore *vlccore = (VLCCore*)ctx;
	if(!vlccore->isRead_){
		return 0;
	}
	// do only for read frame
	int64_t tm = gtm();
	if(tm-vlccore->lastReadTm_ > 5000){
		LOG(ERROR) << ("vlccore read timeout\n");
		return 1;
	}
	return 0;
}

#endif

