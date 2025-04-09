#include "OSCManager.h"

OSCManager::OSCManager(uint16_t listeningPort,
                       uint16_t targetPort,
                       IPAddress targetIP,
                       String boardName,
                       long oscPingTimeoutMs,
                       bool oscSendDebug,
                       bool oscReceiveDebug) : Component("osc", true),
                                               listeningPort(listeningPort),
                                               targetPort(targetPort),
                                               targetIP(targetIP),
                                               boardName(boardName),
                                               oscPingTimeoutMs(oscPingTimeoutMs), 
                                               oscSendDebug(oscSendDebug), 
                                               oscReceiveDebug(oscReceiveDebug)
{
}

void OSCManager::update()
{
  if (millis() > lastSentPingMs + oscPingTimeoutMs)
  {
    String addr = (boardName=="")?"/":"" + boardName + "/ping";
    OSCMessage pingMsg(addr.c_str());
    sendMessage(pingMsg);
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

      // when receiving messages from new IP, makes this the new target
      if (targetIP != udp.remoteIP())

        targetIP = udp.remoteIP();
      // if (overrideTargetIp)
      dbg("new target: " + String(targetPort) + "@" + targetIP.toString());
    }

    if (msg.match("/yo"))
    {
      // sendEvent(OSCEvent(OSCEvent::Type::HANDSHAKE));
    }
    else if (msg.match("/ping"))
    {
      // sendEvent(OSCEvent(OSCEvent::Type::PING_ALIVE));
    }
    else
    {
      // sendEvent(OSCEvent(&msg));
    }
  }
}

void OSCManager::open()
{
  dbg("open OSC");
  udp.begin(listeningPort);
  udp.clear();
  lastSentPingMs = millis();
}

void OSCManager::close()
{
  udp.clear();
  udp.stop();
}

void OSCManager::sendOSC(String address)
{
  OSCMessage m(address.c_str());
  sendMessage(m);
}

// TODO include config name
void OSCManager::sendMessage(OSCMessage &msg)
{
  char addr[32];
  msg.getAddress(addr);
  if (oscSendDebug)
    log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + String(addr));
  udp.beginPacket(targetIP, targetPort);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
}