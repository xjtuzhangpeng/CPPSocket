
/* 
 * File:   SocketTest.h
 * Author: kannanb
 *
 * Created on April 21, 2016, 2:47 PM
 */

#ifndef SOCKETTEST_H
#define SOCKETTEST_H

#include <gtest/gtest.h>
#include <list>

#include "CPPSocket/CPPTcpSocket.h"
#include "CPPSocket/CPPUdpSocket.h"

using std::list;

class SocketTest : public testing::Test{
public:
    virtual void SetUp();
    virtual void TearDown();
};

class SocketTest_TCP : public SocketTest{
public:
    void initializeOne();
    void initializeMany(uint32_t numClients);
    
    //Server socket
    CPPTcpServerSocket server;
    
    //Client sockets from client view
    list<CPPTcpClientSocket> clientSockets;
    
    //Client sockets from server view
    list<CPPSocket> connectedClients;
};

#endif /* SOCKETTEST_H */

