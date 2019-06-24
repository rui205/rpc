cmake_minimum_required(VERSION 3.9.6)
project(rpcServer)



include_directories(
	/home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/include
)

find_package(GFLAGS REQUIRED)

#set(GFLAGS_INCLUDE_PATH "/home/wangpeng/WorkSpaceForMe/PlatformForMe/Framework/3rd")

#execute_process(
#    COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
#    OUTPUT_VARIABLE GFLAGS_NS
#)


#if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
#    execute_process(
#        COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
#        OUTPUT_VARIABLE GFLAGS_NS
#    )
#endif()

#set(CMAKE_CXX_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -pipe -Wall -g -W")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fstrict-aliasing -Wno-invalid-offsetof -Wno-unused-parameter -fno-omit-frame-pointer -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -lgflags -lglog -levent")

set(BUTIL_SOURCES
	${PROJECT_SOURCE_DIR}/rpc/Util.cpp
	#${PROJECT_SOURCE_DIR}/rpc/Dao.cpp
	${PROJECT_SOURCE_DIR}/rpc/DynamicThreadPool.cpp
	${PROJECT_SOURCE_DIR}/rpc/Policy.cpp
	${PROJECT_SOURCE_DIR}/rpc/MemoryPool.cpp
	${PROJECT_SOURCE_DIR}/rpc/TaskQueue.cpp
	${PROJECT_SOURCE_DIR}/rpc/Channel.cpp
    ${PROJECT_SOURCE_DIR}/rpc/TcpServer.cpp
    ${PROJECT_SOURCE_DIR}/rpc/test.cpp
)

add_executable(${PROJECT_NAME} ${BUTIL_SOURCES})
target_link_libraries(rpcServer /home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/lib/gflags/libgflags.a)
target_link_libraries(rpcServer /home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/lib/glog/libglog.a)
target_link_libraries(rpcServer /home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/lib/libevent/libevent.a)
target_link_libraries(rpcServer /home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/lib/hiredis/libhiredis.a)
target_link_libraries(rpcServer /home/wangpeng/WorkSpaceForMe/PlatformForMe/MyCloud/rpc/lib/mysql/libmysqlclient.so.20)


#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#file(COPY ${PROJECT_SOURCE_DIR}/src/
#          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/inc
#          FILES_MATCHING
#          PATTERN "*.h"
#          PATTERN "*.hpp"
#)

