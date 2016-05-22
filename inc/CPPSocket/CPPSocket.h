/* 
 * File:   CPPSocket.h
 * Author: Barath Kannan
 * Description: Thread-safe light-weight C++11 socket class
 * Created on 30 May 2015, 1:23 PM
 */

#ifndef CPPSOCKET_H
#define	CPPSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <mutex>

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

class CPPSocket {
public:
    CPPSocket();
    CPPSocket(int fd); // constructs a socket from a file descriptor
    ~CPPSocket();
    bool close();
    int recv(void *buf, size_t len, int flags = 0);
    int send(void *buf, size_t len, int flags = 0);
    bool hasData(int timeout = 10000); //milliseconds
    bool setBlocking(bool block, bool lock = true);
    bool isBlocking();
    bool setSocketOption(int optionLevel, int optionName, void *optionValue, size_t optionLength, bool lock = true);
    bool setSocketReceiveTimeout(int timeout = 1000); //milliseconds
    int getSock();
    bool isOpen();
    operator int();
    template<typename T> int operator <<(T someType);
    template<typename T> int operator >>(T someType);
    
protected:
    virtual bool open(int type, int af = AF_INET);
    int m_sock;
    std::mutex sendLock;
    std::mutex recvLock;
    bool sigClose;
    int interruptPipe[2];
};

#endif	/* CPPSOCKET_H */

