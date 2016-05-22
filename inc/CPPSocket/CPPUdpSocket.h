/* 
 * File:   CPPUdpSocket.h
 * Author: Barath Kannan
 * Description: Thread-safe light-weight C++11 UDP socket class
 * Created on 14 June 2015, 4:29 PM
 */

#ifndef CPPUDPSOCKET_H
#define	CPPUDPSOCKET_H

#include <string.h>
#include <list>
#include "CPPSocket/CPPSocket.h"

class CPPUdpTransmitSocket : public CPPSocket{
public:
    CPPUdpTransmitSocket();
    bool connect(short port, unsigned int addr, unsigned int ifAddr = INADDR_ANY, int txBuf = 0x000004000);
    bool connect(short port, std::string addr, unsigned int ifAddr = INADDR_ANY, int txBuf = 0x000004000);
protected:
    bool open();
    short m_port;
    unsigned int m_ifaddr;
    unsigned int m_addr;
    bool isMulticast;
};

class CPPUdpReceiveSocket : public CPPSocket{
public:
    CPPUdpReceiveSocket();
    bool connect(short port, unsigned int addr, unsigned int ifAddr = INADDR_ANY, int rxBuf = 0x000004000);
    bool connect(short port, std::string addr, unsigned int ifAddr = INADDR_ANY, int rxBuf = 0x000004000);
    bool addMembership(unsigned int addr, unsigned int ifAddr);
    bool dropMembership(unsigned int addr, unsigned int ifAddr);
    
protected:
    bool open();
    short m_port;
    unsigned int m_ifaddr;
    unsigned int m_addr;
    std::list<ip_mreq> m_subscribedGroups;
};

#endif	/* CPPUDPSOCKET_H */

