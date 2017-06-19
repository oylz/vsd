###################################################################### 
######################################################################
TARGET = vsd

TEMPLATE = app

win32{
	QMAKE_CXXFLAGS += /source-charset:utf-8
}
INCLUDEPATH += .
HEADERS += include/common/Base.h \
		   include/FBMap.h \
		   include/common/Queue.h \
           include/common/DQueue.h \
           src/FrameBuffer.h \
           src/VLCCore.h \
	   src/DD.h \
	   include/common/DParam.h \




SOURCES += FBMap.cpp \
		Main.cpp
		
	   

		   

win32{
	INCLUDEPATH += ..\libs\opencv\xyzwin1\install\include
	INCLUDEPATH += ..\libs\glog-master\src\windows
	INCLUDEPATH += ..\libs\ffmpeg-20170422-207e6de-win64-dev\include
	INCLUDEPATH += ..\libs\boost_1_55_0
}
unix{
	INCLUDEPATH += /usr/local/include

	INCLUDEPATH += ../opencvlib/include
	LIBS += -L../opencvlib/lib
	LIBS += -lopencv_corexyz -lopencv_imgprocxyz -lopencv_highguixyz
}

win32{
	LIBS += -llibglog
}
unix{
	LIBS += -lglog
}
LIBS += -lavcodec
LIBS += -lavdevice
LIBS += -lavfilter
LIBS += -lavformat
LIBS += -lavutil
LIBS += -lswscale


CONFIG += debug_and_release
CONFIG(debug, debug|release){
	win32{
		LIBS += -L..\libs\opencv\xyzwin1\lib\Debug -L..\libs\ffmpeg-20170422-207e6de-win64-dev\lib -L..\libs\boost_1_55_0\stage\lib -L..\libs\glog-master\x64\Debug
		LIBS += -lopencv_core2413d -lopencv_highgui2413d -lopencv_imgproc2413d
		QMAKE_POST_LINK += copy /y ..\libs\ffmpeg-20170422-207e6de-win64-shared\bin\*.dll  .\debug\ &
		QMAKE_POST_LINK += copy /y ..\libs\opencv\xyzwin1\bin\Debug\*.dll .\debug\ &
		QMAKE_POST_LINK += copy /y ..\libs\glog-master\x64\Debug\libglog.dll .\debug\
	}
	unix{
		QMAKE_CXXFLAGS += -fsanitize=leak -fsanitize=address -g -fsanitize=undefined
		LIBS += -fsanitize=leak -fsanitize=address -fsanitize=undefined
		TARGET = bin/vsdd
	}
}
else{
	win32{
		LIBS += -L..\libs\opencv\xyzwin1\lib\Release -L..\libs\ffmpeg-20170422-207e6de-win64-dev\lib -L..\libs\boost_1_55_0\stage\lib -L..\libs\glog-master\x64\Release
		LIBS += -lopencv_core2413 -lopencv_highgui2413 -lopencv_imgproc2413
		QMAKE_POST_LINK += copy /y ..\libs\ffmpeg-20170422-207e6de-win64-shared\bin\*.dll  .\release\ &
		QMAKE_POST_LINK += copy /y ..\libs\opencv\xyzwin1\bin\Release\*.dll .\release\ &
		QMAKE_POST_LINK += copy /y ..\libs\glog-master\x64\Release\libglog.dll .\release\	
	}
	unix{
		TARGET = bin/vsd
	}
}

CONFIG += c++14




DEFINES += QUEUEWAIT
DEFINES += FFMPEG33
DEFINES += OPENCV24




