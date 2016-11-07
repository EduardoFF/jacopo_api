#include "config_lcm_handler.h"

#define IT(c) __typeof((c).begin())
#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)

const UInt8 TimestampedConfigMsgHandler::to_all=255;

TimestampedConfigMsgHandler::TimestampedConfigMsgHandler()
{
}

TimestampedConfigMsgHandler::TimestampedConfigMsgHandler(const char * url, 
							 const string &channel,
							 bool autorun)
{
  /// Create a new LCM instance
  m_lcm = lcm_create(url);
  /// Set LCM URL
  m_lcmURL = url;
  /// Set LCM channel
  m_lcmChannel = channel;
  /// FIXME: better outside?
  if (isLCMReady()) 
  {
    subscribeToChannel(channel);
  }
  pthread_mutex_init(&m_mutex, NULL);
  if( autorun )
    run();
}

bool
TimestampedConfigMsgHandler::run()
{
//  printf("running\n");
  int status = pthread_create(&m_thread, NULL, internalThreadEntryFunc, this);
  return (status == 0);
}

void 
TimestampedConfigMsgHandler::internalThreadEntry() 
{
  while (true) 
  {
    //printf("Waiting for messages\n");
    m_lcm.handle();
    //getAvailableMessages();
    //printf("Messages are received\n");
  }
}

inline bool 
TimestampedConfigMsgHandler::isLCMReady() 
{
  if (!m_lcm.good()) 
  {
    printf("LCM is not ready :(");
    return false;
  } 
  else 
  {
    printf("LCM is ready :)");
    return true;
  }
}

inline bool
TimestampedConfigMsgHandler::hasConfigMsgs(UInt8 id)
{
  return (m_msgList[id].size()>0);
}

std::pair< bool, TimestampedConfigMsg >  
TimestampedConfigMsgHandler::getNextConfigMsg(UInt8 id,  UInt64 t)
{
  TimestampedConfigMsg msg;
  TimestampedConfigMsgList &msgList =
    m_msgList[id];
  if( !msgList.size() )
    return make_pair(false, msg);
  TimestampedConfigMsgList::iterator it = 
    msgList.begin();
  if( t < (*it).timestamp )
  {
    /// first point in list is ahead
    return make_pair(false, (*it));
  }
  msg = (*it);
  while( it != msgList.end() )
  {
    if( t < (*it).timestamp )
      break;
    msg = (*it);
    it++;
  }
  return make_pair(true, msg);
}

std::pair< bool, TimestampedConfigMsg >  
TimestampedConfigMsgHandler::popNextConfigMsg(UInt8 id,  UInt64 t)
{
  TimestampedConfigMsg msg;
  TimestampedConfigMsgList &msgList =
    m_msgList[id];
  if( !msgList.size() )
    return make_pair(false, msg);
  TimestampedConfigMsgList::iterator it = 
    msgList.begin();
  if( t < (*it).timestamp )
  {
    /// first point in list is ahead
    return make_pair(false, (*it));
  }
  msg = (*it);
  while( it != msgList.end() )
  {
    if( t < (*it).timestamp )
      {
	break;
      }
    msg = (*it);
    it++;
  }
  msgList.erase(--it);  
  return make_pair(true, msg);
}

inline void 
TimestampedConfigMsgHandler::subscribeToChannel(const string & channel)
{
  printf("Listening to channel %s\n", channel.c_str());
  m_lcm.subscribe(channel, &TimestampedConfigMsgHandler::handleMessage, this);
}

void 
TimestampedConfigMsgHandler::handleMessage(const lcm::ReceiveBuffer* rbuf, 
						   const std::string& chan, 
						   const config_msg_t* msg)
{
  UInt64 tt=msg->timestamp;
  UInt8 rid = msg->robotid;
  //  printf("TimestampedConfigMsgHandler: [%lld] got config for %d %s\n",
  //	 tt, rid, msg->msg.c_str());
  if( rid == to_all )
    {
      FOREACH (it, m_msgList)
	{
	  (it->second).insert( TimestampedConfigMsg(tt, msg->msg));
	}
    }
  else
    m_msgList[rid].insert( TimestampedConfigMsg(tt, msg->msg));
}

void
TimestampedConfigMsgHandler::publishMessage(TimestampedConfigMsg &t_msg)
{
  config_msg_t config_msg;
  config_msg.timestamp = (int64_t) t_msg.timestamp;
  config_msg.msg = t_msg.msg;
  config_msg.robotid = to_all;
  m_lcm.publish(m_lcmChannel, &config_msg);
}

void
TimestampedConfigMsgHandler::publishMessage(string channel,
					    int id, 
					    TimestampedConfigMsg &t_msg)
{
  config_msg_t config_msg;
  config_msg.timestamp = (int64_t) t_msg.timestamp;
  config_msg.msg = t_msg.msg;
  config_msg.robotid = id;
  m_lcm.publish(channel, &config_msg);
}
