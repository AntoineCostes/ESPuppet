#include "OSCManager.h"

OSCManager::OSCManager(uint16_t listeningPort,
                       uint16_t targetPort,
                       IPAddress targetIP,
                       bool broadcast,
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
                                               oscReceiveDebug(oscReceiveDebug),
                                               broadcast(broadcast),
                                               broadcastIP(IPAddress()),
                                               gatewayIP(IPAddress())
{
}

void OSCManager::update()
{
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

      // when receiving messages from new IP, makes this the new target
      if (targetIP != udp.remoteIP())
      {
        targetIP = udp.remoteIP();
        broadcast = false;
        dbg("new target: " + String(targetPort) + "@" + targetIP.toString());
      }

      if (msg.match("/yo"))
      {
        OSCMessage m("/yo");
        m.add(WiFi.localIP().toString());
        sendMessage(m);
      }
      else
      {
        sendEvent(Command(&msg));
      }
    }
  }
}

void OSCManager::open()
{
  dbg("open OSC");
  udp.begin(listeningPort);
  udp.flush();
  lastSentPingMs = millis();
}

void OSCManager::close()
{
  udp.flush();
  udp.stop();
}

void OSCManager::setBroadcastIPs(IPAddress broadcastIP, IPAddress gatewayIP)
{
  this->broadcastIP = broadcastIP;
  this->gatewayIP = gatewayIP;
}

void OSCManager::sendOSC(String address)
{
  OSCMessage m(address.c_str());
  sendMessage(m);
}

void OSCManager::sendMessage(OSCMessage &msg)
{
  String fullAddress = "/" + boardName + String(msg.getAddress());
  msg.setAddress(fullAddress.c_str());

  if (broadcast)
  {
    if (oscSendDebug)
      log("Bradcast message to " + broadcastIP.toString() + " and " + gatewayIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
    udp.beginPacket(broadcastIP, targetPort);
    msg.send(udp);
    udp.endPacket();

    udp.beginPacket(gatewayIP, targetPort);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
  }
  else
  {
    if (oscSendDebug)
      log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + fullAddress);
    udp.beginPacket(targetIP, targetPort);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
  }
}
