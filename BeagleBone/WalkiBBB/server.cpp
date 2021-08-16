#include "server.h"

#include <netdb.h>
#include <linux/tcp.h>

#include "public_definitions.h"
#include "lib/debugstream.h"
#include "lib/utils.h"

using namespace std;
using namespace chrono;

#define ACCEPT_QUEUE_SIZE 5
#define SERVER_RX_BUFFER_SIZE 1000 ///< Size of the RX buffer [B].
#define WRITE_RETRY_DELAY microseconds(100) ///< Time delay before retrying to write the data to the socket [us].
#define WRITE_TIMEOUT microseconds(1000) ///< Maximum time allowed to write the data to the socket [us].

const int OS_SEND_BUFFER_SIZE = 1048576; // [B].

static Server* instance = nullptr;

/**
 * @brief Constructor.
 * @param port TCP port.
 */
Server::Server(int port)
{
    instance = this;

    this->port = port;
    serverSocket = -1;

    // Ignore the "broken pipe" signal, that could occur if the socket
    // closes during a write operation.
    signal(SIGPIPE, SIG_IGN);
}

/**
 * @brief Destructor.
 */
Server::~Server()
{
    stop();
}

/**
 * @brief Starts the server.
 * @return true if the server started properly, false otherwise.
 */
bool Server::start()
{
    // Create the server socket object.
    serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

    if(serverSocket < 0)
        return false;

    // Setup the socket to re-use the socket, if already existing.
    int enable = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable,
                  sizeof(int)) < 0)
    {
        return false;
    }

    // Disable Nagle's algorithm, to reduce latency.
#ifdef __arm__
    enable = 1;
    if((setsockopt(serverSocket, SOL_SOCKET, TCP_NODELAY, &enable,
                   sizeof(enable))) < 0)
    {
        return false;
    }
#endif

    // Free the communication port.
    Utils::execBashCommand("fuser -k " + to_string(port) + "/tcp");
    this_thread::sleep_for(milliseconds(100));

    // Start listening to incoming connections.
    memset(&socketInfos, 0, sizeof(socketInfos));

    socketInfos.sin_family = AF_INET;
    socketInfos.sin_addr.s_addr = INADDR_ANY;
    socketInfos.sin_port = htons(port);

    if(bind(serverSocket, (sockaddr*)&socketInfos, sizeof(socketInfos)) < 0)
        return false;

    listen(serverSocket, ACCEPT_QUEUE_SIZE);

    // Display connection information to the user.
    debug << "Server started on port " << port << ". Interfaces:" << endl;

    for(auto &ip : getIPs())
        debug << ip.first << ": " << ip.second << endl;

    debug << endl;

    return true;
}

/**
 * @brief Stops the server.
 */
void Server::stop()
{
    // Close the server socket.
    if(serverSocket >= 0)
    {
        close(serverSocket);
        serverSocket = 0;
    }

    debug << "Server stopped." << endl;
}

/**
 * @brief Updates the server status.
 * Checks the client connection state and acquires the bytes received.
 */
void Server::update()
{
    if(serverSocket < 0)
        return;

    // Check if a client just connected.
    sockaddr_in socketInfos;
    socklen_t socketInfosSize = sizeof(socketInfos);
    int clientSocket = accept4(serverSocket, (sockaddr*)&socketInfos,
                               &socketInfosSize, SOCK_NONBLOCK);

    if(clientSocket >= 0)
    {
        shared_ptr<ClientSocket> client(new ClientSocket(clientSocket,
                                                         socketInfos));
        newClientSockets.push_back(client);

        debug << "Client connected: " << client->getClientIpAddress() << "."
              << endl;
    }
}

/**
 * @brief Gets the list of the clients that connected since the last call.
 * @return The list of the clients that connected to the server since the last
 * call to this method.
 */
vector<std::shared_ptr<ClientSocket>> Server::getNewClients()
{
    auto ncs = newClientSockets;
    newClientSockets.clear();
    return ncs;
}

/**
 * @brief Retrieves possibles IP addresses to contact this server.
 * Builds a list of IP addresses, associated with their network interface.
 * @return the list of IP addresses and corresponding network interfaces.
 */
vector<pair<string, string>> Server::getIPs()
{
    vector<pair<string, string>> interfacesIPs;

    // Get the linked list of all network interfaces.
    ifaddrs *netInterfaces;

    if(getifaddrs(&netInterfaces) == -1)
        return interfacesIPs;

    for(ifaddrs *ifa = netInterfaces; ifa != nullptr; ifa = ifa->ifa_next)
    {
        // Only keep valid IP V4 network interfaces/addresses.
        if(ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        // Retrieve the host address of the network interface.
        char hostAddress[NI_MAXHOST];
        int err = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                              hostAddress, NI_MAXHOST, nullptr, 0,
                              NI_NUMERICHOST);

        if(err != 0)
            continue;

        // Add the interface infos to the list.
        pair<string, string> interfaceIP(ifa->ifa_name, string(hostAddress));
        interfacesIPs.push_back(interfaceIP);
    }

    freeifaddrs(netInterfaces);

    return interfacesIPs;
}

/**
 * @brief Constructor.
 * @param socket network socket to send and receive data with the client.
 * @param socketInfos infos structure about the socket (IP address, etc.),
 * generally obtained when calling accept().
 */
ClientSocket::ClientSocket(int socket, sockaddr_in socketInfos) :
    socket(socket), socketInfos(socketInfos)
{
    socketClosed = false;
    lastRxByteTime = steady_clock::now();
    rxBytes.reserve(SERVER_RX_BUFFER_SIZE);

    // Setup the socket to use a large send buffer. This is useful because
    // when the SyncVars list is sent, a lot of data has to be sent at the same
    // time. This avoids the need for a dedicated sending thread.
    setsockopt(socket, SOL_SOCKET, SO_SNDBUF,
               &OS_SEND_BUFFER_SIZE, sizeof(OS_SEND_BUFFER_SIZE));

    // Get the client IP address.
    ipAddress.resize(100);

    inet_ntop(AF_INET, &socketInfos.sin_addr,
              ipAddress.data(), ipAddress.size());

    size_t strEndIndex = ipAddress.find('\0');
    if(strEndIndex == string::npos)
        ipAddress.clear();
    else
        ipAddress.resize(strEndIndex);
}

/**
 * @brief Destructor.
 */
ClientSocket::~ClientSocket()
{
    close(socket);
}

/**
 * @brief Sends data to the remote client.
 * @param data the data bytes to send.
 */
void ClientSocket::send(const std::vector<uint8_t> &data)
{
    // Check if the socket is still open.
    if(!isActive())
        return;

    // Write the data bytes to the socket.
    ssize_t nBytesWritten = 0;
    duration<int64_t, micro> operationTime = microseconds(0);

    while(nBytesWritten < (ssize_t)data.size())
    {
        ssize_t n = write(socket, &data[nBytesWritten],
                          data.size() - nBytesWritten);

        if(n < 0)
        {
            int e = errno;

            if(e == EAGAIN)
            {
                // The socket could not be written now, because the buffer
                // is full. Retry.
                operationTime += WRITE_RETRY_DELAY;

                if(operationTime <= WRITE_TIMEOUT)
                {
                    this_thread::sleep_for(WRITE_RETRY_DELAY);
                    continue;
                }
                else // A write timeout occured.
                {
                    socketError("write timeout");
                    break;
                }
            }
            else // A write error occured.
            {
                socketError("write error");
                break;
            }
        }
        else
            nBytesWritten += n;
    }
}

/**
 * @brief Gets the bytes received.
 * @return the bytes received since the previous call to this method.
 */
const std::vector<uint8_t> &ClientSocket::receive()
{
    // Check if the socket is still OK.
    if(!isActive())
    {
        rxBytes.resize(0);
        return rxBytes;
    }

    // Get the received bytes.
    rxBytes.resize(SERVER_RX_BUFFER_SIZE);
    int nRxBytes = read(socket, rxBytes.data(), SERVER_RX_BUFFER_SIZE);

    if(nRxBytes < 0)
        nRxBytes = 0;

    rxBytes.resize(nRxBytes);

    // If nothing has been received from the client for a long time, it
    // probably means that it has been disconnected.
    if(nRxBytes > 0)
        lastRxByteTime = steady_clock::now();
    else
    {
        auto timeWithoutRx = steady_clock::now() - lastRxByteTime;

        if(timeWithoutRx > milliseconds(TCP_MAX_TIME_WITHOUT_RX))
            socketError("client disconnected (timeout)");
    }

    return rxBytes;
}

/**
 * @brief Gets if the socket is still active.
 * @return true if the socket is still active, false if it was closed.
 */
bool ClientSocket::isActive()
{
    if(socketClosed)
        return false;
    else
    {
        int error = 0;
        socklen_t len = sizeof(error);
        int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len);

        if(err == 0)
            return true;
        else
        {
            socketError("socket error");
            return false;
        }
    }
}

/**
 * @brief Gets the IP address of the client.
 * @return The IP address of the client, as a string.
 */
string ClientSocket::getClientIpAddress() const
{
    return ipAddress;
}

/**
 * @brief Flags the object to be deleted, and prints an error message.
 * @param description description of the error, to be printed.
 */
void ClientSocket::socketError(string description)
{
    socketClosed = true;
    debug << "Client socket closed (" << ipAddress << "): " << description
          << "." << endl;
}
