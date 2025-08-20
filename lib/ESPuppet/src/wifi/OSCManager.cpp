#include "OSCManager.h"

OSCManager::OSCManager(uint16_t listeningPort,
                       uint16_t targetPort,
                       IPAddress targetIP,
                       bool broadcast,
                       long oscPingTimeoutMs,
                       bool oscSendDebug,
                       bool oscReceiveDebug) : Component("osc", true),
                                               listeningPort(listeningPort),
                                               targetPort(targetPort),
                                               targetIP(targetIP),
                                               oscPingTimeoutMs(oscPingTimeoutMs),
                                               oscSendDebug(oscSendDebug),
                                               oscReceiveDebug(oscReceiveDebug),
                                               doBroadcast(broadcast),
                                              //  broadcastIP(IPAddress()),
                                              //  gatewayIP(IPAddress()),
                                               isOpen(false)
{
}

void OSCManager::update()
{
  if(!isOpen)  return;

  // TODO silent send
  if (millis() > lastSentPingMs + oscPingTimeoutMs)
  {
    sendOSC("/ping");
    
    OSCMessage m("/ip");
    m.add(WiFi.localIP()==IPAddress(0, 0, 0, 0)? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str());
    sendMessage(m, doBroadcast, true);
    
    OSCMessage m2("/port");
    m2.add((int32_t)listeningPort);
    sendMessage(m2, doBroadcast, true);

    lastSentPingMs = millis();
  }

  OSCMessage msg;
  int size;
  if ((size = udp.parsePacket()) > 0)
  {
    while (size--)
      msg.fill(udp.read());
    if (!msg.hasError())
    {
      if (oscReceiveDebug)
        log("got message: " + String(msg.getAddress()));

      if (msg.match("/targetPort")) 
      {
        if (msg.isInt(0)) 
        {
          targetPort = msg.getInt(0);
          log("NEW TARGET PORT : "+String(targetPort));
        }
      }
      else if (msg.match("/houdini_light") || msg.match("/houdini_fire")) 
      {
      // FIXME filter commands with board ID instead
      // we should send message as /Dobby/board/...
      }
      else if (msg.match("/yo")) 
      {
        if (targetIP != udp.remoteIP())
        {
          targetIP = udp.remoteIP();
          doBroadcast = false;
          dbg("new target: " + targetIP.toString()+ ":" + String(targetPort));
        } else 
          dbg("yo but same ");
      }
      else
      {
        // when receiving messages from new IP, makes this the new target
        if (targetIP != udp.remoteIP())
        {
          targetIP = udp.remoteIP();
          doBroadcast = false;
          dbg("new target: " + targetIP.toString()+ ":" + String(targetPort));
        }
        sendEvent(Command(&msg));
      }
    }
  }
}

void OSCManager::open()//IPAddress broadcastIP, IPAddress gatewayIP)
{
  if (isOpen) close();
  dbg("open port "+String(listeningPort));
  udp.begin(listeningPort);
  udp.flush();
  // this->broadcastIP = broadcastIP;
  // this->gatewayIP = gatewayIP;
  lastSentPingMs = millis();
  isOpen = true;
  if (doBroadcast) dbg("ready to broadcast to "+(WiFi.getMode() == WIFI_MODE_AP)?WiFi.softAPBroadcastIP().toString():WiFi.broadcastIP().toString());
  else dbg("ready to send to "+targetIP.toString());
}

void OSCManager::close()
{
  dbg("close");
  udp.flush();
  udp.stop();
  isOpen = false;
}

void OSCManager::sendOSC(String address)
{
  OSCMessage m(address.c_str());
  sendMessage(m, doBroadcast);
}

void OSCManager::sendMessage(OSCMessage &msg, bool broadcast, bool silent)
{
  if (!isOpen)
  {
    err("Can't send OSC message yet");
    return;
  }
  String fullAddress = "/" + FileManager::getCurrentConfigName() + String(msg.getAddress());
  msg.setAddress(fullAddress.c_str());

  switch (WiFi.status())
  {
    case WL_CONNECTED: // connected to STA
      if (broadcast)
      {
         if (oscSendDebug && !silent) log("Broadcast message to " + WiFi.broadcastIP().toString()+ "/" + WiFi.gatewayIP().toString() + ":" + String(targetPort)  + " : " + fullAddress);
        udp.beginPacket(WiFi.broadcastIP(), targetPort);
        msg.send(udp);
        udp.endPacket();
        
        udp.beginPacket(WiFi.gatewayIP(), targetPort);
        msg.send(udp);
        int ok = udp.endPacket();
        if (!ok) flush();
        
      }
      else
      {
        if (oscSendDebug && !silent) log("Send message to " + targetIP.toString() + ":" + String(targetPort) + " : " + fullAddress);
        udp.beginPacket(targetIP, targetPort);
        msg.send(udp);
        int ok = udp.endPacket();
        if (!ok) flush();
      }
      break;

    case WL_NO_SHIELD: // active hotspot
      if (broadcast)
      {
         if (oscSendDebug && !silent) log("Broadcast message to " + WiFi.softAPBroadcastIP().toString() + "@" + String(targetPort) + " : " + fullAddress);
          udp.beginPacket(WiFi.softAPBroadcastIP(), targetPort);
          msg.send(udp);
        int ok = udp.endPacket();
        if (!ok) flush();
      } else
      {
        if (oscSendDebug && !silent) log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
        udp.beginPacket(targetIP, targetPort);
        msg.send(udp);
        int ok = udp.endPacket();
        if (!ok) flush();
      }
      break;

    default:
      dbg("Can't send OSC message, Wifi is not connected");
      break;
  }
}

void OSCManager::flush()
{
  dbg("FLUSH");
  udp.flush();
  udp.begin(listeningPort);
  udp.flush();
}