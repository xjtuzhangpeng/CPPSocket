#include "CPPSocket/CPPTcpSocket.h"
#include <thread>

CPPTcpClientSocket::CPPTcpClientSocket()
: CPPSocket()
{}

bool CPPTcpClientSocket::open(){
    return CPPSocket::open(SOCK_STREAM);
}

bool CPPTcpClientSocket::connect(short port, unsigned int addr, int timeout){
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!open())
        return false;

    m_port = port;
    m_addr = addr;
    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = m_addr;
    remote.sin_port = htons(m_port);
    double tremaining = timeout;
    auto start = std::chrono::system_clock::now();
    do{
        if (::connect(m_sock, (struct sockaddr *)&remote, sizeof(remote)) == 0){
            return true;
        }
        std::chrono::duration<double, std::milli> dur = std::chrono::system_clock::now() - start;
        tremaining = timeout - dur.count();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }while ((tremaining > 0.0 || timeout < 0) && !sigClose);
    ::close(m_sock);
    m_sock=-1;
    return false;
}

bool CPPTcpClientSocket::connect(short port, std::string addr, int timeout){
    return connect(port, inet_addr(addr.c_str()), timeout);
}

CPPTcpServerSocket::CPPTcpServerSocket()
: CPPSocket()
{}

bool CPPTcpServerSocket::open(){
    return CPPSocket::open(SOCK_STREAM);
}

bool CPPTcpServerSocket::listen(short port, int maxConnections){
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    struct sockaddr_in local;
    memset(&local, 0, sizeof(sockaddr_in));
    m_port = port;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (!open())
        return false;

    int on=1;
    if (!setSocketOption(SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on), false))
        return false;

    // Bind socket to local address
    if(::bind(m_sock, (struct sockaddr *)&local, sizeof(struct sockaddr_in))==-1) {
        return false;
    }

    // Create queue for client connection requests
    if(::listen(m_sock, maxConnections)==-1) {
        return false;
    }
    
    m_port = port;
    return true;
}

int CPPTcpServerSocket::accept(int timeout){
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!isOpen()) return -1;
    struct pollfd pfd = {m_sock, POLLIN|POLLPRI, 0};
    if (poll(&pfd, 1, timeout) > 0){
        struct sockaddr_in remote;
        socklen_t size = sizeof(struct sockaddr_in);
        return ::accept(m_sock, (struct sockaddr *)&remote, &size);
    }
    return -1;
}
