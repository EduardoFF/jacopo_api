#ifndef _FOOTBOTNAV_H_
#define _FOOTBOTNAV_H_

#include <iostream>
#include <fstream>
#include <argos2/common/control_interface/ci_controller.h>
#include <argos2/common/utility/logging/argos_log.h>
#include <argos2/common/utility/argos_random.h>
#include <argos2/common/utility/datatypes/color.h>
#include <argos2/common/control_interface/swarmanoid/footbot/ci_footbot_wheels_actuator.h>
#include <argos2/common/control_interface/swarmanoid/footbot/ci_footbot_leds_actuator.h>
#include <argos2/common/control_interface/swarmanoid/footbot/ci_footbot_beacon_actuator.h>
#include <argos2/common/utility/argos_random.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <math.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/un.h>

#include <argos2/common/control_interface/swarmanoid/footbot/ci_footbot_encoder_sensor.h>
#include <navigation/client/nav_client.h>

#include <config_lcm_handler/config_lcm_handler.h>

using namespace argos;
using namespace std;

#include <argos2/common/utility/datatypes/datatypes.h>

#define MAX_UNIX_SOCKET_BUFFER_SIZE 512
#define MAX_UDP_SOCKET_BUFFER_SIZE 2048

struct NeighborEntry
{
  UInt8 counter;
  UInt8 nh;
};
  
  

class FootbotJacopoAPI: public CCI_Controller
{
  private:
    UInt32 RandomSeed;
    std::string m_MyIdStr;
    UInt64 m_Steps;
    CARGoSRandom::CRNG* m_randomGen;
    UInt64 m_sendPackets;
    UInt8 m_myID;
    bool m_started;
    RVONavClient *m_navClient;
    UInt8 m_maxCounter;

    CCI_WiFiSensor* m_pcWifiSensor;
    CCI_WiFiActuator* m_pcWifiActuator;
    std::map< UInt8, NeighborEntry > m_neighborTable;

    CCI_FootBotBeaconActuator* m_beaconActuator;
   
    int m_receivedPacketsAll;
    int m_sentPackets;
    char *m_socketMsg;
    int m_sendSocket;              // Network socket
    int m_recvSocket;              // Network socket

    struct sockaddr_in m_remote_addr;
    std::string m_remoteHost;
    UInt16 m_remotePort;
    char *m_rcvMsg;
    bool m_networkEnabled;
    void GetNetMessages();
    
    //! Communication stuff
    void initComm(TConfigurationNode& t_tree );
    //void configureComm();
    void start();
    void stop();
    void processHello(std::string &);
    bool processPacket(std::string &);
    

    void sendTimerFired();
    #ifndef FOOTBOT_SIM
    void sendPacket(char *data, UInt32 size);
    void makeMsg();
    #endif
    string makeNeighborPacket();
    void checkNeighbors();
    string makeHelloPacket();
  public:

    /* Class constructor. */
    FootbotJacopoAPI();

    /* Class destructor. */
    virtual ~FootbotJacopoAPI() {
    }

    virtual void Init(TConfigurationNode& t_tree);

    virtual void ControlStep();
    virtual void Destroy();
    virtual bool IsControllerFinished() const;

    std::string getTimeStr();
    UInt64 getTime();
    void sendPacket(string &str);
    CVector3 randomWaypoint();
    /// make it static because in simulation robots can share the same config handler
    static TimestampedConfigMsgHandler *m_configHandler;
    void processConfig(std::string);

    void notifyPosition();
    void notifyNeighbors();
    void setNeighborDistance(UInt8 nid, UInt8 nh);
    void resetNeighborCounter(UInt8 nid);
    
  

};

#endif
