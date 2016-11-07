#ifndef _CONFIG_LCM_HANDLER_H_
#define _CONFIG_LCM_HANDLER_H_
/*
 * Copyright (C) 2014, IDSIA (Institute Dalle Molle for Artificial Intelligence), http://http://www.idsia.ch/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <argos2/common/utility/datatypes/datatypes.h>
#include <navigation/lcm/protectedmutex.h>
#include <stdio.h>
#include <map>
#include <set>
#include <lcm/lcm-cpp.hpp>
#include <configmsg/config_msg_t.hpp>
#include <pthread.h>
#include <argos2/common/utility/math/vector2.h>
#include <argos2/common/utility/math/vector3.h>

using namespace std;
using namespace argos;
using namespace configmsg;
/**
 * @brief LCM engine - (see more information at https://code.google.com/p/lcm/)
 *
 * @details Basically this class provides the necessary methods for using LCM
 * based engine.
 *
 *
 */

typedef std::string ConfigMsg;
struct TimestampedConfigMsg
{
  UInt64 timestamp;
  std::string msg;
TimestampedConfigMsg(UInt64 t, const std::string &s):
  timestamp(t),
    msg(s)
  {}
TimestampedConfigMsg():
  timestamp(0),
    msg("NO_CMD")
  {}
  
};

//std::pair< UInt64, ConfigMsg > TimestampedConfigMsg;
struct TimestampedConfigMsgCompare
{
  bool 
    operator()(const TimestampedConfigMsg & left, const TimestampedConfigMsg & right) const
    {
      return left.timestamp <= right.timestamp;
    }
};


typedef 
std::set< TimestampedConfigMsg, TimestampedConfigMsgCompare > TimestampedConfigMsgList;

class TimestampedConfigMsgHandler 
{

  public:

    /**
     * Default LCM constructor
     */
    TimestampedConfigMsgHandler();
    static const UInt8 to_all;
    
    /**
     * LCM constructor.
     *
     * @param url char, the LCM URL
     * @param channel string, the LCM channel
     */
    TimestampedConfigMsgHandler(const char * url, const string &channel, bool autorun);

    bool run();
    /**
     * This method is in charge of to handle the message obtained from a specific LCM channel.
     *
     * @param rbuf
     * @param chan string, the specified channel.
     * @param msg poselcm::pose_list_t, the message obtained from the channel (automatically created by LCM engine from a previous specification)
     */
    void handleMessage(const lcm::ReceiveBuffer* rbuf, 
		       const std::string& chan, 
		       const config_msg_t* msg);

    void publishMessage(TimestampedConfigMsg &mgs);
    void publishMessage(string channel, int, TimestampedConfigMsg &mgs);


    /**
     * Checks if LCM is ready.
     *
     * @return true if yes or false it not
     */
    inline bool isLCMReady(); 
    /**
     * To subscribe to a specific LCM channel to get messages from it.
     *
     * @param channel string, name of the channel.
     */
    inline void subscribeToChannel(const string & channel) ;

    /**
     * Checks if there are messages from the channel for a pre-specified time.
     *
     * @param timeout
     * @return status int, > 0 there were messages, == 0 timeout expired and < 0 an error occurred
     */
    int getAvailableMessagesTimeout(int timeout) 
    {
      return m_lcm.handleTimeout(timeout);
    }

    /**
     * Blocking call for waiting messages
     * @return status int, > 0 there were messages, == 0 timeout expired and < 0 an error occurred
     */
    int getAvailableMessages() 
    {
      return m_lcm.handle();
    }


    /* Getters & setters */

    const string& getLcmChannel() const {
      return m_lcmChannel;
    }

    void setLcmChannel(const string& lcmChannel) 
    {
      this->m_lcmChannel = lcmChannel;
    }

    const char* getLcmUrl() const 
    {
      return m_lcmURL;
    }

    void setLcmUrl(const char* lcmUrl) 
    {
      m_lcmURL = lcmUrl;
    }

    bool hasConfigMsgs(UInt8);

    std::pair< bool, TimestampedConfigMsg >  getNextConfigMsg(UInt8 i, UInt64 t);
    std::pair< bool, TimestampedConfigMsg >  popNextConfigMsg(UInt8 i, UInt64 t);
  private:
    std::map<UInt8,TimestampedConfigMsgList> m_msgList;
    static UInt64 getTime();

    /**
     * LCM URL
     */
    const char * m_lcmURL;

    /**
     * LCM channel
     */
    string m_lcmChannel;

    /**
     * LCM instance
     */
    lcm::LCM m_lcm;

    /**
     * Mutex to control the access to the listAllNodes
     */
    pthread_mutex_t m_mutex;
    /**
     * Thread
     */
    pthread_t m_thread;

    static void * internalThreadEntryFunc(void * ptr) 
    {
      (( TimestampedConfigMsgHandler *) ptr)->internalThreadEntry();
      return NULL;
    }
    void internalThreadEntry();
};

 #endif /* _CONFIG_LCM_HANDLER_H_ */
