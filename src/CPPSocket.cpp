#include "CPPSocket/CPPSocket.h"

CPPSocket::CPPSocket(){
    m_sock = -1;
    sigClose = false;
    ::pipe(interruptPipe);
}

CPPSocket::CPPSocket(int fd){
    m_sock = fd;
    sigClose = false;
    ::pipe(interruptPipe);
}

CPPSocket::~CPPSocket(){
    close();
    ::close(interruptPipe[0]);
    ::close(interruptPipe[1]);
}

bool CPPSocket::isOpen(){
    return (m_sock >= 0);
}

bool CPPSocket::open(int type, int af){
    return (isOpen() ? false : ((m_sock = (int)socket(af, type, 0)) > 0));
}

bool CPPSocket::close(){
    sigClose = true;
    char interrupt = '\0';
    ::write(interruptPipe[1], &interrupt, 1);
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    sigClose = false;
    ::read(interruptPipe[0], &interrupt, 1);
    bool ret = isOpen() ? (::close(m_sock) == 0) : false;
    m_sock = -1;
    return ret;
}

int CPPSocket::recv(void *buf, size_t len, int flags){
    std::lock_guard<std::mutex> lockR(recvLock);
    return (isOpen() ? ::recv(m_sock, buf, len, flags) : -1);
}

int CPPSocket::send(void *buf, size_t len, int flags){
    std::lock_guard<std::mutex> lockS(sendLock);
    return (isOpen() ? ::send(m_sock, buf, len, flags) : -1);
}

bool CPPSocket::hasData(int timeout){
    std::lock_guard<std::mutex> lockR(recvLock);
    if (!isOpen()) return false;
    
    struct pollfd pfd[2];
    pfd[0] = {m_sock, POLLIN|POLLPRI, 0};
    pfd[1] = {interruptPipe[0], POLLIN|POLLPRI, 0};
    
    if (poll(pfd, 2, timeout) > 0){
        return (!sigClose);
    }
    return false;
}

bool CPPSocket::setBlocking(bool block, bool lock){   
    std::unique_lock<std::mutex> lockR(recvLock, std::defer_lock), lockS(sendLock, std::defer_lock);
    if (lock) std::lock(lockR, lockS);
    if (!isOpen()) return false;
    long arg = fcntl(m_sock, F_GETFL, NULL); 
    block ? (arg &= (~O_NONBLOCK)) : (arg |= O_NONBLOCK); 
    return ((fcntl(m_sock, F_SETFL, arg)) != -1);
}

bool CPPSocket::isBlocking(){
        long arg = fcntl(m_sock, F_GETFL, NULL);
        return (arg == (arg & (~O_NONBLOCK)));
}

bool CPPSocket::setSocketOption(int optionLevel, int optionName, void *optionValue, size_t optionLength, bool lock){
    std::unique_lock<std::mutex> lockR(recvLock, std::defer_lock), lockS(sendLock, std::defer_lock);
    if (lock) std::lock(lockR, lockS);
    if (!isOpen()) return false;
    
    return (::setsockopt(m_sock, optionLevel, optionName, optionValue, optionLength) != -1);
}

bool CPPSocket::setSocketReceiveTimeout(int timeout) {
    std::lock_guard<std::mutex> lockR(recvLock);
    
    std::chrono::duration<int, std::milli> dur(timeout);
    std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
    
    timeval tv_to;
    tv_to.tv_sec = sec.count();
    tv_to.tv_usec = (std::chrono::duration_cast<std::chrono::microseconds>(dur-sec)).count();
    return setSocketOption(SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_to, sizeof(tv_to), false);
}


int CPPSocket::getSock(){
    return m_sock;
}

CPPSocket::operator int(){
    return getSock();
}

template <typename T> int CPPSocket::operator <<(T someType){
    return recv(&someType, sizeof(T));
}

template <typename T> int CPPSocket::operator >>(T someType){
    return send(&someType, sizeof(T));
}
