#include "footbot_jacopo_api.h"


#ifndef FOOTBOT_SIM
#else
#include <argos2/simulator/physics_engines/physics_engine.h>
#include <argos2/simulator/simulator.h>
#endif

#define __USE_DEBUG_COMM 1
#define __USE_DEBUG_ERROR 1


#if __USE_DEBUG_COMM
#define DEBUGCOMM(m, ...) \
{\
  fprintf(stderr, "%.2f DEBUGCOMM[%d]: " m,\
	  (float) m_Steps,\
	  (int) m_myID, \
          ## __VA_ARGS__);\
  fflush(stderr);\
}
#else
#define DEBUGCOMM(m, ...)
#endif


#if __USE_DEBUG_ERROR
#define DEBUGERROR(m, ...) \
{\
  fprintf(stderr, "DEBUGERROR[%d]: " m,\
	  (int) m_myID,\
          ## __VA_ARGS__);\
  fflush(stderr);\
}
#else
#define DEBUGERROR(m, ...)
#endif

#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)

TimestampedConfigMsgHandler *FootbotJacopoAPI::m_configHandler =
  new TimestampedConfigMsgHandler("udpm://239.255.76.67:7667?ttl=1", "CONFIG", true);


FootbotJacopoAPI::FootbotJacopoAPI() :
  RandomSeed(12345),
  m_Steps(0),
  m_randomGen(0),
  m_maxCounter(3)
{
  m_networkEnabled = false;
  m_started = false;
}


  void 
FootbotJacopoAPI::Init(TConfigurationNode& t_node) 
{
  /// The first thing to do, set my ID
#ifdef FOOTBOT_SIM
  m_myID = 
    atoi(GetRobot().GetRobotId().substr(3).c_str());
#else
  m_myID = 
    atoi(GetRobot().GetRobotId().substr(7).c_str());
#endif
  printf("MyID %d\n", m_myID);

  m_beaconActuator=
    dynamic_cast<CCI_FootBotBeaconActuator*>   (GetRobot().GetActuator("footbot_beacon"));
    
  /// Random
  GetNodeAttributeOrDefault(t_node, "RandomSeed", RandomSeed, RandomSeed);
  CARGoSRandom::CreateCategory("local_rng", RandomSeed);
#ifdef FOOTBOT_SIM
  m_randomGen = CSimulator::GetInstance().GetRNG();
#else
  m_randomGen = CARGoSRandom::CreateRNG("local_rng");
#endif


  initComm(t_node);
   
  /// create the client and pass the configuration tree (XML) to it
  m_navClient = new RVONavClient(m_myID, GetRobot());
  m_navClient->init(t_node);
}



  void
FootbotJacopoAPI::initComm( TConfigurationNode& t_tree )
{

  //! get configuration parameters
  if (NodeExists(t_tree, "comm")) 
  {
    TConfigurationNode t_node = GetNode(t_tree, "comm");
    GetNodeAttributeOrDefault(t_node,
			      "enabled", m_networkEnabled, m_networkEnabled);
    
  } 
  else
  {
    printf("WARNING: Comm has no configuration - using default\n");
  }
  
  m_receivedPacketsAll = 0;
  m_sentPackets=0;
  m_socketMsg = new char[MAX_UDP_SOCKET_BUFFER_SIZE];

#ifndef FOOTBOT_SIM
  /// Create sending socket
  if( (m_sendSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
  {
    DEBUGERROR("Opening sending socket\n");
    exit(-1);
  }

  /// OPTIONAL: Set socket to allow broadcast
  int broadcastPermission = 1;
  if (setsockopt(m_sendSocket, SOL_SOCKET, 
		 SO_BROADCAST, (void *) &broadcastPermission, 
		 sizeof(broadcastPermission)) < 0) 
  {
    DEBUGERROR("setsockopt() failed while setting broadcast permission\n");
    exit(-1);
  }

  //! TODO: make it a parameter in the xml
  m_remoteHost = "10.42.43.255";
  m_remotePort = 7788;
  /// Fill in the address of the remote host
  memset(&m_remote_addr, 
	 0, sizeof(m_remote_addr));
  m_remote_addr.sin_family = AF_INET;
  m_remote_addr.sin_port = htons( (short) m_remotePort);

  bool USE_BROADCAST = false;
  if( USE_BROADCAST )
  {
    m_remote_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    DEBUGCOMM("peer addr = BROADCAST, port %d\n",
	      (int) m_remotePort);
  } 
  else
  {
    /// Resolve the server address (convert from symbolic name to IP number)
    struct hostent *host = gethostbyname(m_remoteHost.c_str());
    if (host == NULL) 
    {
      DEBUGERROR("Cannot define host address\n");
      exit(-1);
    }
    // Print a resolved address of server (the first IP of the host)
    DEBUGCOMM(
      "peer addr = %d.%d.%d.%d, port %d\n",
      host->h_addr_list[0][0] & 0xff,
      host->h_addr_list[0][1] & 0xff,
      host->h_addr_list[0][2] & 0xff,
      host->h_addr_list[0][3] & 0xff,
      (int) m_remotePort
      );
    /// Write resolved IP address of a server to the address structure
    memmove(&(m_remote_addr.sin_addr.s_addr), host->h_addr_list[0], 4);
  }

  /// Create receiving socket
  if( (m_recvSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) 
  {
    DEBUGERROR("Opening receiving socket\n");
    exit(-1);
  }
  /// Fill in the address structure containing self address
  struct sockaddr_in myaddr;
  memset(&myaddr, 0, sizeof(struct sockaddr_in));
  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons(m_remotePort);        // Port to listen
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //myaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  /// Bind a socket to the address
  int res = bind(m_recvSocket, (struct sockaddr*) &myaddr, sizeof(myaddr));
  if (res < 0) 
  {
    DEBUGERROR("Cannot bind a receiving socket\n"); exit(-1);
  }
  //! Set the "LINGER" timeout to zero, to close the listen socket
  //! immediately at program termination.
  struct linger linger_opt = { 1, 0 }; //! Linger active, timeout 0
  setsockopt(m_recvSocket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

  // Include IP Header
  int on = 1;
  setsockopt(m_recvSocket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
  m_rcvMsg = new char[MAX_UDP_SOCKET_BUFFER_SIZE];

  /*
   *************************************************
   * END OF: Initialize communication
   *************************************************
   */
//#endif
#else
  m_pcWifiSensor = dynamic_cast<CCI_WiFiSensor* >(GetRobot().GetSensor("wifi"));
  m_pcWifiActuator = dynamic_cast<CCI_WiFiActuator* >(GetRobot().GetActuator("wifi"));
#endif

}


  void 
FootbotJacopoAPI::sendTimerFired()
{
#ifdef FOOTBOT_SIM
  string msg = makeNeighborPacket();
  sendPacket(msg);
#else
  string msg = makeNeighborPacket();
  sprintf(m_socketMsg, "%s", msg.c_str());
  m_sentPackets++;
  size_t len = strlen(m_socketMsg);
  DEBUGCOMM("Sending packet %d (len %zu) @ %llu\n", 
	    m_sentPackets, len, getTime());
  sendPacket(m_socketMsg, len);
  #endif
}

#ifndef FOOTBOT_SIM

/// fills the buffer with a new msg content, taking into account the current packet size
/// valid for both simulation and real
void
FootbotJacopoAPI::makeMsg()
{
  sprintf(m_socketMsg, "HELLO %d", (int) m_myID);
}


  void 
FootbotJacopoAPI::sendPacket(char *data, UInt32 size)
{
  unsigned int r = sendto(m_sendSocket, 
		 data, 
		 size, 
		 0, 
		 (struct sockaddr *)&m_remote_addr, 
		 sizeof(m_remote_addr));

  if( r != size ) 
  {
    DEBUGERROR("Error while sending packet %s\n", strerror(errno));
  } 
  else 
  {
    DEBUGCOMM("Packet Sent!\n");
  }
}
#endif

void
FootbotJacopoAPI::start()
{
  if( m_started )
    return;
  printf("*************** STARTING *************\n");
  fflush(stdout);
  
  /// clear beacon 
  m_beaconActuator->SetColor(CColor::BLACK);

  m_started = true;
  ///start navclient
  m_navClient->start();
  m_navClient->stay();

}



string
FootbotJacopoAPI::makeNeighborPacket()
{
  std::ostringstream os(ostringstream::out);
  os << "NTABLE " << (int) m_myID;
  FOREACH(it, m_neighborTable)
    {
      if( (it->second).counter > 0 )
	os << " " << (int) it->first
	   << " " << (int) (it->second).nh;
    }
  return os.str();
    
}

string
FootbotJacopoAPI::makeHelloPacket()
{
  std::ostringstream os(ostringstream::out);
  os << "HELLO " << (int) m_myID;
  return os.str();
}

void
FootbotJacopoAPI::sendPacket(string &msg)
{
  m_sendPackets++;
  m_pcWifiActuator->BroadcastMessage(msg);
}


CVector3
FootbotJacopoAPI::randomWaypoint()
{
  /// generate point in the square (1,5) x (1,5)
  Real x = m_randomGen->Uniform(CRange<Real>(1,5));
  Real y = m_randomGen->Uniform(CRange<Real>(1,5));
  CVector3 random_point(x,y,0);
  std::cout << "new random waypoint ("
	    << random_point.GetX()
	    << ", " <<  random_point.GetY()
	    << ")" << std::endl;
  return random_point;
}

void
FootbotJacopoAPI::processConfig(std::string msg)
{
  std::stringstream ss(msg);
  std::string cmd;
  ss >> cmd;
  printf("Got cmd %s\n", msg.c_str());
  fflush(stdout);

  /// pass it entirely to nav client
  if( cmd == "START" )
    start();
  if( cmd == "STOP" )
    stop();

  m_navClient->processConfig(msg);
}

void
FootbotJacopoAPI::stop()
{
  if( !m_started )
    return;
  printf("*************** STOPPING *************\n");
  fflush(stdout);

  m_started = false;
  m_navClient->stop();

}

void
FootbotJacopoAPI::setNeighborDistance(UInt8 nid, UInt8 nh)
{
  m_neighborTable[nid].nh = nh;
}

void
FootbotJacopoAPI::resetNeighborCounter(UInt8 nid)
{
  m_neighborTable[nid].counter = m_maxCounter;
}

void
FootbotJacopoAPI::processHello(std::string &msg)
{
  if( strncmp(msg.c_str(), "HELLO", 5) == 0 )
    {
      DEBUGCOMM("%s\n", msg.c_str());
      char hello[5];
      int rid;
      sscanf("%s %d", msg.c_str(),  hello, &rid);
      if( rid != (int) m_myID )
	{
	  DEBUGCOMM("Got neighbor %d\n", rid);
	  setNeighborDistance(rid, 1);
	  resetNeighborCounter(rid);
	}

    }
  else
    {
      DEBUGCOMM("Rejected Invalid HELLO\n");
    }
}

bool
FootbotJacopoAPI::processPacket(std::string &msg)
{
  std::istringstream is(msg.c_str());
  string cmd;
  is >> cmd;
  if( cmd == "NTABLE")
    {
      int rid;
      is >> rid;
      if( rid == m_myID )
	return false;
      DEBUGCOMM("Got neighbor %d\n", rid);
      setNeighborDistance(rid, 1);
      resetNeighborCounter(rid);
      int nh;
      while ( is >> rid >> nh )
	{
	  if( rid == (int) m_myID )
	    continue;
	  if( m_neighborTable[rid].counter <= 0)
	    {
	      setNeighborDistance(rid, nh+1);
	      resetNeighborCounter(rid);
	    }
	  else
	    {
	      if( m_neighborTable[rid].nh >= nh+1 )
		{
		  setNeighborDistance(rid, nh+1);
		  resetNeighborCounter(rid);
		}
	    }
	}
    }
  else
    {
      DEBUGCOMM("Rejected Invalid Packet %s\n", cmd.c_str());
      return false;
    }
  return true;
}


  void 
FootbotJacopoAPI::ControlStep() 
{
  m_Steps+=1;

  // start after 1 second
  //  if( m_Steps == 30 )
  //  start();
  
  /// BUG WARNING - we must pop everything
  for(;;)
    {
      std::pair< bool, TimestampedConfigMsg> c_msg =
	m_configHandler->popNextConfigMsg(m_myID, getTime());
      printf("RobotId %d ControlStep %lld - config msgs? %d\n",
	     m_myID, m_Steps, c_msg.first);
      if( c_msg.first )
	{
	  //    cout << "robot " << m_myID <<  " time "
	  //	 << getTime() << "config? " << c_msg.first
	  //	 << " msg.timestamp: " << c_msg.second.timestamp << endl;
	  processConfig(c_msg.second.msg);
	}
      else
	break;
    }


  /* do whatever */

#ifdef FOOTBOT_SIM
  //searching for the received msgs
  TMessageList t_incomingMsgs;
  m_pcWifiSensor->GetReceivedMessages(t_incomingMsgs);
  for(TMessageList::iterator it = t_incomingMsgs.begin(); it!=t_incomingMsgs.end();it++)
    {
      std::string str_msg(it->Payload.begin(),
			  it->Payload.end());
      std::cout << "[" << (int) m_myID << "] Received packet: "
		  << str_msg << std::endl;
      processPacket(str_msg);
  }
#else
  GetNetMessages();
#endif

  
  ///  must call this two methods from navClient in order to
  ///  update the navigation controller
  m_navClient->setTime(getTime());
  m_navClient->update();

  /// send HELLO messages each second
  if( m_myID%10 == m_Steps%10 && m_networkEnabled)
    sendTimerFired();

  /// notify position each second
  if( m_myID%10 == m_Steps%10)
    {
      notifyPosition();
      notifyNeighbors();
      checkNeighbors();
    }
    

}

void
FootbotJacopoAPI::notifyPosition()
{
  CVector3 pos = m_navClient->currentPosition();
  CQuaternion ori = m_navClient->currentOrientation();
  TimestampedConfigMsg msg;
  msg.timestamp = getTime();
  stringstream ss;
  /// position must be in cm
  ss << "POS "
     << " " << pos.GetX()
     << " " << pos.GetY()
     << " " << pos.GetZ();
  msg.msg = ss.str();
  m_configHandler->publishMessage("API",m_myID, msg);
}

void
FootbotJacopoAPI::notifyNeighbors()
{
  TimestampedConfigMsg msg;
  msg.timestamp = getTime();
  msg.msg = makeNeighborPacket();
  m_configHandler->publishMessage("API",m_myID, msg);
}

void
FootbotJacopoAPI::checkNeighbors()
{
  FOREACH(it, m_neighborTable)
    {
      if( (it->second).counter > 0 )
	{
	  (it->second).counter--;
	}
    }
}



/// ===========  only defined for REAL robots ======= //
#ifndef FOOTBOT_SIM

void
FootbotJacopoAPI::GetNetMessages()
{
  struct sockaddr from;
  int addr_len = sizeof(struct sockaddr_in);
  int msg_size;
  struct iphdr *ip;
  struct udphdr *udp;
  char *msg_data;

  bool has_msgs = false;
  
  while ( (msg_size = recv(m_recvSocket, m_rcvMsg, MAX_UDP_SOCKET_BUFFER_SIZE, MSG_DONTWAIT)) > 0 ) 
  {

    DEBUGCOMM("Got recv msg_size %d\n", msg_size);

    /// Here, take only the packets generated by equal robotic controller
    /// 5 is for HELLO prefix
    
    if ( msg_size < (5 + sizeof(struct iphdr) + sizeof(struct udphdr) ) )
      continue;
    m_rcvMsg[msg_size] = '\0';
    
    ip = (struct iphdr*) m_rcvMsg;
    udp = (struct udphdr*) (m_rcvMsg + sizeof(struct iphdr));
    msg_data = (char *) (m_rcvMsg + sizeof(struct iphdr) + sizeof(struct udphdr) );
    int data_size = msg_size - sizeof(struct iphdr) + sizeof(struct udphdr);
    std::string str_msg(msg_data, msg_data+data_size);
    has_msgs = processPacket(str_msg);
      
    // printf("[%d] Received %d bytes from socket %d (TTL=%d): %s\n", Steps, msg_size, recvSocket,ip->ttl,msg_data);
    //printf("%d,",65 - ip->ttl);

    // printf("%s\t --> \t%s\n",
    //                 inet_ntoa(*(struct in_addr*) &ip->saddr),
    //                 inet_ntoa(*(struct in_addr *) &ip->daddr) );

    //			std::cout << "*";
    //			std::cout.flush();
    //sscanf (msg_data,"%d %*s",&mSentDataPackets);
    //printf("DECODED VALUE: %d\n",mSentDataPackets);
    //has_msgs = true;
    m_receivedPacketsAll++;
  }

  if( has_msgs )
  {
    DEBUGCOMM("Got %d packets\n", m_receivedPacketsAll);
    //if( m_beaconMode == BEACON_RECEIVE_BLINK )
    {
      m_beaconActuator->SetColor(CColor::YELLOW);
    }
  }
  else
  {
    // if( m_beaconMode == BEACON_RECEIVE_BLINK )
    {
      m_beaconActuator->SetColor(CColor::BLACK);
    }
  }
}

#endif

  void 
FootbotJacopoAPI::Destroy() 
{
  DEBUG_CONTROLLER("FootbotJacopoAPI::Destroy (  )\n");
}

/**************************************/

bool 
FootbotJacopoAPI::IsControllerFinished() const 
{
  return false;
}


std::string
FootbotJacopoAPI::getTimeStr()
{
#ifndef FOOTBOT_SIM
  char buffer [80];
  timeval curTime;
  gettimeofday(&curTime, NULL);
  int milli = curTime.tv_usec / 1000;
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
  char currentTime[84] = "";
  sprintf(currentTime, "%s:%d", buffer, milli);
  std::string ctime_str(currentTime);
  return ctime_str;
#else
  std::stringstream ss;
  ss << "Time ms " << getTime();
  return ss.str();
#endif
}


/// returns time in milliseconds
  UInt64 
FootbotJacopoAPI::getTime()
{
#ifndef FOOTBOT_SIM
  struct timeval timestamp;
  gettimeofday(&timestamp, NULL);

  UInt64 ms1 = (UInt64) timestamp.tv_sec;
  ms1*=1000;

  UInt64 ms2 = (UInt64) timestamp.tv_usec;
  ms2/=1000;

  return (ms1+ms2);
#else
  return m_Steps * CPhysicsEngine::GetSimulationClockTick() * 1000;
#endif
}


  
REGISTER_CONTROLLER(FootbotJacopoAPI, "footbot_jacopo_api")

