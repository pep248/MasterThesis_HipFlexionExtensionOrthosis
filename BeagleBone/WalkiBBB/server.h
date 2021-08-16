#ifndef DEF_SERVER_H
#define DEF_SERVER_H

#include <vector>
#include <string>
#include <utility>
#include <chrono>
#include <memory>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/**
 * @defgroup Server Server
 * @brief TCP server to let remote clients connect and communicate.
 * @ingroup Main
 * @{
 */

class ClientSocket;

/**
 * @brief TCP server to create communication sockets with remote clients.
 * @ingroup Main
 */
class Server
{
public:
    Server(int port);
    ~Server();

    bool start();
    void stop();
    void update();

    std::vector<std::shared_ptr<ClientSocket>> getNewClients();
    std::vector<std::pair<std::string, std::string>> getIPs();

private:
    int port;
    int serverSocket;
    sockaddr_in socketInfos;
    std::vector<std::shared_ptr<ClientSocket>> newClientSockets;
};

/**
 * @brief Communication socket, to send and receive data a remote client.
 */
class ClientSocket
{
public:
    ClientSocket(int socket, sockaddr_in socketInfos);
    ~ClientSocket();

    void send(const std::vector<uint8_t> &data);
    const std::vector<uint8_t> &receive();

    bool isActive();

    std::string getClientIpAddress() const;

private:
    void socketError(std::string description);

    const int socket;
    const sockaddr_in socketInfos;
    std::vector<uint8_t> rxBytes;
    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::nanoseconds> lastRxByteTime;
    bool socketClosed;

    std::string ipAddress;
};

/**
 * @}
 */

#endif
