#include "canbus.h"
#include "../config/config.h"
#include "../lib/utils.h"
#include "../lib/debugstream.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define CAN_OVERLAY "BB-CAN1"
#define CAN_PATH "/dev/can1"

using namespace std;

/**
 * @brief Constructor.
 */
CanBus::CanBus()
{
    // Load can drivers
    system("modprobe can");
    system("modprobe can-dev");
    system("modprobe can-raw");

    // Set the pins muxing.
    try
    {
        Utils::loadOverlay(CAN_OVERLAY,"");
    }
    catch(runtime_error)
    {
        debug<<"CANdriver -> load overlay error"<<endl;
        return;
    }

    // Set-up the can network
    system("sudo ip link set can0 up type can bitrate 1000000");
    system("sudo ip link set up can0");

    // to check up, can0 should appear as a network in the terminal.
    //system("ifconfig");

    debug<<"CANdriver -> load overlay OK"<<endl;

    // Create an empty socket API with PF_CAN as the protocol family:
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        debug<<"CANdriver -> socket"<<endl;
        state = PeripheralState::FAULT;
        return;
    }

    // Bind the socket to one of the interfaces (vcan0 or canx x=0,1):
    struct ifreq ifr;
    struct sockaddr_can addr;

    const char *ifname = "can0";

    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error in socket bind");
        return;
    }

    // Set to non blocking
    //fcntl(s, F_SETFL, O_NONBLOCK);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000; // microseconds
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0)//SOL_CAN_RAW
    {
        printf("CAN setsockopt failed");
    }
}

/**
 * @brief Destructor.
 */
CanBus::~CanBus()
{
    // Step 4 – Closing the socket: Give the following command:
    close(s);
}

/**
 * @brief Send the first can_frame stored in pending_request.
 */
void CanBus::Send()
{
    // Step 3a – Writing CAN frames: To send a frame with the ID 0x101 and a two byte payload of 0x41, 0x42, use the following commands:

    struct can_frame frame;

    if(pending_request.size()>0)
    {
        frame = pending_request.at(0); //send first element
        int nbytes = write(s, &frame, sizeof(struct can_frame));
        if(nbytes < 0)
        {
            perror("can raw socket write");
            return;
        }
        else
        {
            //printf("CAN sent: ID %x - DATA %x - %x - %x - %x - %x - %x - %x - %x\n", frame.can_id& 0x1fffffffu, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
            pending_request.erase(pending_request.begin());//remove first element from vector
        }
    }
}

/**
 * @brief receive a can_frame an store it in receive_memory.
 */
void CanBus::Receive()
{
    // Step 3b – Reading CAN frames: Use the following commands in this step:
    struct can_frame frame;
    int nbytes = read(s, &frame, sizeof(struct can_frame));

    if (nbytes < 0)
    {
        perror("can raw socket read");
        return;
    }
    else
    {
        //Extract frame.id, frame.can_dlc, frame.data[i]
        //printf("CAN received: ID %x - DATA %x - %x - %x - %x - %x - %x - %x - %x\n", frame.can_id& 0x1fffffffu, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
        receive_memory[(unsigned int)(frame.can_id & 0x1fffffffu)].push_back(frame);
    }
}

/**
 * @brief Do send and receive until pending request is empty.
 */
void CanBus::Update()
{
    while(pending_request.size()!=0)
    {
        Send();
        Receive();
    }
}

/**
 * @brief Pull the can_frame corresponding to a specific ID from receive_memory
 * @param id: can id of the messages to retreive
 * @param rxFrames: vector of can frames to store the received can frames
 */
int CanBus::PullMessage(unsigned int id, std::vector<can_frame>& rxFrames)
{
    int ret = false;
    if(receive_memory[id].size()>0)
    {
        copy(receive_memory[id].begin(), receive_memory[id].end(), back_inserter(rxFrames));// = receive_memory[id];
        ret = true;
    }
    //remove all frames coresponding to the id
    receive_memory[id].clear();
    return ret;
}

/**
 * @brief Push the can_frame to pending request
 * @param frame: the can_frame to send
 */
void CanBus::PushMessage(can_frame frame)
{
    pending_request.push_back(frame);
    return;
}
