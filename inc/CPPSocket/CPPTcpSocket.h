/* 
 * File:   CPPTCPSocket.h
 * Author: Barath Kannan
 * Description: Thread-safe light-weight C++11 TCP Socket classes
 * Created on 4 June 2015, 12:34 PM
 */

#ifndef CPPTCPSOCKET_H
#define	CPPTCPSOCKET_H

#include <string.h>
#include <netinet/tcp.h>
#include "CPPSocket/CPPSocket.h"

class CPPTcpClientSocket : public CPPSocket{
public:
    CPPTcpClientSocket();
    
    //spin flag indicates that the method will re-attempt to connect on
    //any error within the timeout interval
    bool connect(short port, unsigned int addr, int timeout = 10000, bool spin = true);
    bool connect(short port, std::string addr, int timeout = 10000, bool spin = true);
protected:
    bool open();
    short m_port;
    unsigned int m_addr;
};

class CPPTcpServerSocket : public CPPSocket{
public:
    CPPTcpServerSocket();
    bool listen(short port, int maxConnections = 1);
    int accept(int timeout = 10000); //returns file descriptor
protected:
    bool open();
    short m_port;
};

#endif	/* CPPTCPSOCKET_H */
