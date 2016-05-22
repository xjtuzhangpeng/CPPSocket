#include "CPPSocket/CPPUdpSocket.h"

CPPUdpTransmitSocket::CPPUdpTransmitSocket()
: CPPSocket()
{}

bool CPPUdpTransmitSocket::open(){
    return CPPSocket::open(SOCK_DGRAM);
}

bool CPPUdpTransmitSocket::connect(short port, unsigned int addr, unsigned int ifAddr, int txBuf) {
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!open())
        return false;
    m_port = port;
    m_addr = addr;
    m_ifaddr = ifAddr;
    isMulticast = false;
    
    if (!setSocketOption(SOL_SOCKET, SO_SNDBUF, &txBuf, sizeof(txBuf), false))
        return false;
    
    // If this is a multicast address
    if((ntohl(m_addr)&0xf0000000)==0xe0000000) {
        isMulticast = true;
        struct ip_mreq ipMreq;
        memset(&ipMreq, 0, sizeof(ipMreq));
        ipMreq.imr_multiaddr.s_addr = m_addr;
        ipMreq.imr_interface.s_addr = m_ifaddr;
        
        if(ifAddr != 0 && !setSocketOption(IPPROTO_IP, IP_MULTICAST_IF, &ipMreq, sizeof(ipMreq), false)){
            return false;
        }
    }

    // Connect to the remote address
    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = m_addr;
    remote.sin_port = htons(m_port);

    if(::connect(m_sock, (struct sockaddr *)&remote, sizeof(remote))==SOCKET_ERROR)
        return false;

    return true;
}

bool CPPUdpTransmitSocket::connect(short port, std::string addr, unsigned int ifAddr, int txBuf) {
    return connect(port, inet_addr(addr.c_str()), ifAddr, txBuf);
}

CPPUdpReceiveSocket::CPPUdpReceiveSocket() 
: CPPSocket()
{ }

bool CPPUdpReceiveSocket::open() {
    return CPPSocket::open(SOCK_DGRAM);
}

bool CPPUdpReceiveSocket::connect(short port, unsigned int addr, unsigned int ifAddr, int rxBuf) {
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!open())
        return false;
    m_port = port;
    m_addr = addr;
    m_ifaddr = ifAddr;
    
    int on =1;
    
    if (!setSocketOption(SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on), false))
        return false;
    
    if (!setSocketOption(SOL_SOCKET, SO_RCVBUF, &rxBuf, sizeof(rxBuf), false))
        return false;
    
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = ((ntohl(m_addr)&0xf0000000)==0xe0000000)?INADDR_ANY:m_ifaddr;
    local.sin_port = htons(m_port);
    if(::bind(m_sock, (struct sockaddr *)&local, sizeof(local))==SOCKET_ERROR){
        return false;
    }

    // If we are to receive from a multicast address
    if((ntohl(m_addr)&0xf0000000)==0xe0000000) {
        struct ip_mreq ipMreq;
        memset(&ipMreq, 0, sizeof(ip_mreq));
        ipMreq.imr_multiaddr.s_addr = m_addr;
        ipMreq.imr_interface.s_addr = m_ifaddr;

        // Join the multicast group
        if (!setSocketOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &ipMreq, sizeof(ipMreq), false))
            return false;

        m_subscribedGroups.push_back(ipMreq);
    }
    
    return true;
}

bool CPPUdpReceiveSocket::addMembership(unsigned int addr, unsigned int ifAddr){
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!isOpen())
        return false;
    
    struct ip_mreq ipMreq;
    memset(&ipMreq, 0, sizeof(ipMreq));
    ipMreq.imr_multiaddr.s_addr = addr;
    ipMreq.imr_interface.s_addr = ifAddr;

    // Join the multicast group
    if (!setSocketOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &ipMreq, sizeof(ipMreq), false))
        return false;

    m_subscribedGroups.push_back(ipMreq);
    return true;
}

bool CPPUdpReceiveSocket::dropMembership(unsigned int addr, unsigned int ifAddr){
    std::lock_guard<std::mutex> lockR(recvLock), lockS(sendLock);
    if (!isOpen())
        return false;
    
    for (auto it=m_subscribedGroups.begin(); it != m_subscribedGroups.end(); it++){
        if (it->imr_multiaddr.s_addr == addr && it->imr_interface.s_addr == ifAddr){
            m_subscribedGroups.erase(it);
            return true;
        }
    }
    return false;
}

bool CPPUdpReceiveSocket::connect(short port, std::string addr, unsigned int ifAddr, int rxBuf) {
    return connect(port, inet_addr(addr.c_str()), ifAddr, rxBuf);
}
