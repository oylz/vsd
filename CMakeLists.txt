CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

PROJECT(FrameBuffer)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFFMPEG33 -DUSP -DUSETRACKING -DUSEDETET -DUSEMATCH -DLOGM")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFFMPEG33 -DUSP -DLOGM")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mavx2 -O3")




FILE(GLOB SOURCES "*.cpp" "base/*.h" "camera/*.h" "camera/*.cpp" "coreb/*.h" "coreb/*.cpp" "coreb/fdsst/*.h" "coreb/fdsst/*.cpp" "db/*.h" "db/*.cpp" "rpc/*.h" "rpc/*.cpp" "view/*.h" "view/monitor/*.h" "view/monitor/*.cpp" "include/*.h")





#ADD_EXECUTABLE(PE ${SOURCES})
ADD_LIBRARY(FrameBuffer SHARED ${SOURCES})




#pkgconfig
FIND_PACKAGE(PkgConfig REQUIRED)

#thrfit
PKG_SEARCH_MODULE(Thrift REQUIRED thrift)
INCLUDE_DIRECTORIES(${Thrift_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(FrameBuffer ${Thrift_LIBRARIES} event)

#sqlite3
PKG_SEARCH_MODULE(sqlite3 REQUIRED sqlite3)
INCLUDE_DIRECTORIES(${sqlite3_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(FrameBuffer ${sqlite3_LIBRARIES})


#boost
FIND_PACKAGE(Boost REQUIRED system thread)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(FrameBuffer ${Boost_LIBRARIES})


# ffmpeg
PKG_SEARCH_MODULE(libavcodec REQUIRED libavcodec)
INCLUDE_DIRECTORIES(${libavcodec_INCLUDE_DIRS})
LINK_DIRECTORIES(${libavcodec_LIBRARY_DIRS})

PKG_SEARCH_MODULE(libswscale REQUIRED libswscale)
INCLUDE_DIRECTORIES(${libswscale_INCLUDE_DIRS})

PKG_SEARCH_MODULE(libavformat REQUIRED libavformat)
INCLUDE_DIRECTORIES(${libavformat_INCLUDE_DIRS})

PKG_SEARCH_MODULE(libavutil REQUIRED libavutil)
INCLUDE_DIRECTORIES(${libavutil_INCLUDE_DIRS})

# xml2
PKG_SEARCH_MODULE(tinyxml2 REQUIRED tinyxml2)
INCLUDE_DIRECTORIES(${tinyxml2_INCLUDE_DIRS})

# opencv
FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
 
 
TARGET_LINK_LIBRARIES(FrameBuffer ${OpenCV_LIBRARIES})
TARGET_LINK_LIBRARIES(FrameBuffer ${libavutil_LIBRARIES} ${libavcodec_LIBRARIES} ${libavformat_LIBRARIES} ${libswscale_LIBRARIES} ${tinyxml2_LIBRARIES})








CMAKE_POLICY(SET CMP0043 NEW)
add_custom_command(TARGET FrameBuffer 
  PRE_BUILD   
  COMMAND mkdir -p data/person 
  COMMAND cp ../icon ./ -rf
  COMMAND cp ../gen.sh ./ -rf
  COMMENT "This command will be executed before build target FrameBuffer
"  
)  















