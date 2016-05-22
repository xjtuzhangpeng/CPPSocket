#include "SocketTest.h"

#include <iostream>
#include <string>
#include <thread>
#include "BasicTimer.h"

using std::cout;
using std::endl;
using std::string;
using std::thread;

using namespace std::chrono_literals;

void SocketTest::SetUp(){}

void SocketTest::TearDown(){}

void SocketTest_TCP::initializeOne(){
    ASSERT_LT((int)server, 0);
    ASSERT_TRUE(server.listen(11034));
    ASSERT_GT((int)server, 0);
    
    clientSockets.emplace_back();
    auto &client = clientSockets.back();
    
    ASSERT_LT((int)client, 0);
    ASSERT_TRUE(client.connect(11034, "127.0.0.1"));
    ASSERT_GT((int)client, 0);
    
    ASSERT_TRUE(server.hasData());
    int fd = server.accept(11034);
    ASSERT_GT(fd, 0);
    
    connectedClients.emplace_back(fd);
    CPPSocket &serverClient = connectedClients.back();
    ASSERT_EQ((int)serverClient, fd);
    
    ASSERT_GT((int)client, 0);   
}

void SocketTest_TCP::initializeMany(uint32_t numClients) {
    ASSERT_LT((int)server, 0);
    ASSERT_TRUE(server.listen(11034, numClients+1));
    ASSERT_GT((int)server, 0);
    
    for (uint32_t i=0; i<numClients; i++){
        clientSockets.emplace_back();
        auto &client = clientSockets.back();
    
        ASSERT_LT((int)client, 0);
        ASSERT_TRUE(client.connect(11034, "127.0.0.1"));
        ASSERT_GT((int)client, 0);
        
        ASSERT_TRUE(server.hasData());
        int fd = server.accept(11034);
        ASSERT_GT(fd, 0);
        connectedClients.emplace_back(fd);
        CPPSocket &serverClient = connectedClients.back();
        ASSERT_EQ((int)serverClient, fd);

        ASSERT_GT((int)client, 0);
    }

    ASSERT_EQ(clientSockets.size(), numClients);
    ASSERT_EQ(connectedClients.size(), numClients);
}


TEST_F(SocketTest_TCP, TCPConnection){
    cout << "Testing TCP connection" << endl;
    initializeOne();
}

TEST_F(SocketTest_TCP, TCPSendAndReceive){
    initializeOne();
    auto &client = clientSockets.back();
    char recvBuffer[512];
    string m1 = "TEST MESSAGE FROM SERVER";
    string m2 = "TEST MESSAGE FROM CLIENT OF DIFFERENT LENGTH";
    
    ASSERT_GT((int)connectedClients.size(), 0);
    CPPSocket &s = connectedClients.back();
    ASSERT_GT((int)s, 0);
    ASSERT_GT((int)client, 0);
    
    cout << "Sending data from server" << endl;
    int result = s.send((void *)(m1.c_str()), m1.length() +1);
    ASSERT_EQ(result, int(m1.length()+1));

    cout << "Data sent: " << m1 << endl;
    
    ASSERT_TRUE(client.hasData());
    result = client.recv(&recvBuffer, m1.length()+1);
    
    cout << "Data received: " << m1 << endl;
    
    ASSERT_EQ(result, int(m1.length()+1));
    ASSERT_EQ((string)recvBuffer, m1);
    
    ASSERT_FALSE(s.hasData(0));
    ASSERT_FALSE(client.hasData(0));
    
    cout << "Sending data from client" << endl;
    result = client.send((void *)(m2.c_str()), m2.length() +1);
    ASSERT_EQ(result, int(m2.length()+1));
    
    cout << "Data sent: " << m2 << endl;
    
    ASSERT_TRUE(s.hasData());
    result = s.recv(&recvBuffer, m2.length() +1);
    
    cout << "Data received: " << m2 << endl;
    
    ASSERT_EQ(result, int(m2.length())+1);
    ASSERT_EQ((string)recvBuffer, m2);
    
    ASSERT_FALSE(s.hasData(0));
    ASSERT_FALSE(client.hasData(0));
}

TEST_F(SocketTest_TCP, MultipleConnections){
    cout << "Testing Multiple TCP connections" << endl;
    initializeMany(10);
}

TEST_F(SocketTest_TCP, DataTimeout){
    BasicTimer bt;
    ASSERT_LT((int)server, 0);
    
    clientSockets.emplace_back();
    auto &client = clientSockets.back();
    
    cout << "Attempting to connect to nonexistent server" << endl;
    bt.start();
    ASSERT_FALSE(client.connect(11034, "127.0.0.1", 1000));
    bt.stop();
    ASSERT_GE(bt.getElapsedSeconds(), 1.0);
    ASSERT_LE(bt.getElapsedSeconds(), 1.1);
    cout << "Timeout period (1000ms expected): " << bt.getElapsedMilliseconds() << "ms" << endl;
    
    ASSERT_TRUE(server.listen(11034));
    
    cout << "Attempting to accept nonexistent client" << endl;
    bt.start();
    ASSERT_EQ(server.accept(1000), -1);
    bt.stop();
    ASSERT_GE(bt.getElapsedSeconds(), 1.0);
    ASSERT_LE(bt.getElapsedSeconds(), 1.1);
    cout << "Timeout period (1000ms expected): " << bt.getElapsedMilliseconds() << "ms" << endl;
    
    ASSERT_TRUE(server.close());
    
    BasicTimer btc;
    btc.start();
    thread cc([&client, &btc](){
        ASSERT_TRUE(client.connect(11034, "127.0.0.1", 2000));
        btc.stop();
        cout << "Accept time: " << btc.getElapsedMilliseconds() << "ms" << endl;
        ASSERT_GE(btc.getElapsedSeconds(), 0.5);
        ASSERT_LE(btc.getElapsedSeconds(), 0.6);
    });
    
    cout << "Waiting for some time (500ms)" << endl;
    std::this_thread::sleep_for(500ms - btc.getElapsedDuration());
    
    cout << "Listening on server" << endl;
    ASSERT_TRUE(server.listen(11034));
    
    cout << "Attempting to accept client" << endl;
    int fd = server.accept(1000);
    cc.join();
    CPPSocket s(fd);
    
    cout << "Attempting to receive nonexistent data from client" << endl;
    bt.start();
    ASSERT_FALSE(s.hasData(1000));
    bt.stop();
    cout << "Timeout period (1000ms expected): " << bt.getElapsedMilliseconds() << "ms" << endl;
    cout << "Attempting to receive nonexistent data from server" << endl;
    bt.start();
    ASSERT_FALSE(client.hasData(1000));
    bt.stop();
    
    cout << "Timeout period (1000ms expected): " << bt.getElapsedMilliseconds() << "ms" << endl;

}
