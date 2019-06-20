#include <unistd.h>

#include "Channel.h"

namespace rpc {

Channel::Channel(int fd) {
    fd_ = fd;
	bev_ = NULL;
	thr_ = NULL;
}

Channel::~Channel() {
    /*don't destroy thread here*/ 
    /*because there are multiple channels under a thread*/
    close(fd_);
	fd_ = -1;
	
	if (bev_ != NULL) {
		event_safe_free(bev_, bufferevent_free);
	}
}

thread_t* Channel::getChannelThread() {
    return thr_;
}

void Channel::setChannelThread(thread_t* thr) {
    thr_ = thr;
}

int Channel::getChannelFd() {
    return fd_;
}

/*bufferevent IO operation*/
void Channel::setBufferevent(struct bufferevent* bev) {
    bev_ = bev;
}

struct bufferevent* Channel::getBufferevent() {
	return bev_;
}

int Channel::getReadBufferLength() {
    return evbuffer_get_length(bufferevent_get_input(bev_));
}

int Channel::getWriteBufferLength() {
    return evbuffer_get_length(bufferevent_get_output(bev_));
}

int Channel::readToBuffer(char* buffer, int length) {
    return evbuffer_remove(bufferevent_get_input(bev_), buffer, length);
}

int Channel::copyToBuffer(char* buffer, int length) {
    return evbuffer_copyout(bufferevent_get_input(bev_), buffer, length);
}

int Channel::appendToBuffer(char* buffer, int length) {
    return evbuffer_add(bufferevent_get_output(bev_), buffer, length);
}

int Channel::appendBufferToBuffer(struct evbuffer* buffer) {
    return evbuffer_add_buffer(bufferevent_get_output(bev_), buffer);
}

void Channel::moveBufferReadToWrite() {
    evbuffer_add_buffer(bufferevent_get_output(bev_), bufferevent_get_input(bev_));
}

}/*end namespace rpc*/
