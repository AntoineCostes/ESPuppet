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
                                               broadcastIP(IPAddress()),
                                               gatewayIP(IPAddress()),
                                               isOpen(false)
{
}

void OSCManager::update()
{
  if(!isOpen)  return;
  
  if (millis() > lastSentPingMs + oscPingTimeoutMs)
  {
    sendOSC("/ping");
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

      if (msg.match("/yo")) 
      {
        if (msg.isInt(0)) log("NEW PORT");
        if (msg.isInt(0)) targetPort = msg.getInt(0);
        sendYo(); 
      }

      // when receiving messages from new IP, makes this the new target
      if (targetIP != udp.remoteIP())
      {
        targetIP = udp.remoteIP();
        doBroadcast = false;
        dbg("new target: " + String(targetPort) + "@" + targetIP.toString());
      }
      else sendEvent(Command(&msg));
    }
  }
}

void OSCManager::open(IPAddress broadcastIP, IPAddress gatewayIP)
{
  if (isOpen) close();
  dbg("open");
  udp.begin(listeningPort);
  udp.flush();
  this->broadcastIP = broadcastIP;
  this->gatewayIP = gatewayIP;
  lastSentPingMs = millis();
  isOpen = true;
  sendYo();
}

void OSCManager::close()
{
  dbg("close");
  udp.flush();
  udp.stop();
  isOpen = false;
}

void OSCManager::sendYo()
{
  OSCMessage m("/yo");
  m.add(WiFi.localIP().toString().c_str());
  m.add((int32_t)listeningPort); // TODO broadcast on multiport ?
  sendMessage(m, true);
}

void OSCManager::sendOSC(String address)
{
  OSCMessage m(address.c_str());
  sendMessage(m, doBroadcast);
}

void OSCManager::sendMessage(OSCMessage &msg, bool broadcast)
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
         if (oscSendDebug) log("Broadcast message to " + broadcastIP.toString() + "@" + String(targetPort) + " and gateway " + gatewayIP.toString() +" : " + fullAddress);
        udp.beginPacket(broadcastIP, targetPort);
        msg.send(udp);
        udp.endPacket();
        
        udp.beginPacket(gatewayIP, targetPort);
        msg.send(udp);
        udp.endPacket();
        
      }
      else
      {
        if (oscSendDebug) log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
        udp.beginPacket(targetIP, targetPort);
        msg.send(udp);
        udp.endPacket();
      }
      break;

    case WL_NO_SHIELD: // active hotspot
      if (broadcast)
      {
         if (oscSendDebug) log("Broadcast message to " + broadcastIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
          udp.beginPacket(broadcastIP, targetPort);
          msg.send(udp);
          udp.endPacket();
      } else
      {
        if (oscSendDebug) log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
        udp.beginPacket(targetIP, targetPort);
        msg.send(udp);
        udp.endPacket();
      }
      break;

    default:
      dbg("Can't send OSC message, Wifi is not connected");
      break;
  }
}
