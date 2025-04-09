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
    String addr = (boardName=="")?"":"/" + boardName + "/ping";
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
      {
        targetIP = udp.remoteIP();
        dbg("new target: " + String(targetPort) + "@" + targetIP.toString());
    }

    if (msg.match("/yo"))
    {
      // sendEvent(Command(Command::Type::HANDSHAKE));
    }
    else
    {
      char addr[64];
      msg.getAddress(addr);
      String address = String(addr).substring(1);
      int separatorIndex = address.indexOf('/');
      String target = separatorIndex == -1 ? "root" : address.substring(0, separatorIndex); 
      String command = address.substring(separatorIndex + 1);

      const int numData = 8;
      var data[numData];

      for (int i = 0; i < msg.size() && i < numData; i++)
          data[i] = OSCArgumentToVar(msg, i);

      sendEvent(Command(target, command, data, msg.size() + 2));
    }
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

void OSCManager::sendMessage(OSCMessage &msg)
{
  char addr[32];
  msg.getAddress(addr);
  String fullAddress = "/"+boardName+String(addr);
  msg.setAddress(fullAddress.c_str());
  if (oscSendDebug)
    log("Send message to " + targetIP.toString() + "@" + String(targetPort) + " : " + String(addr));
  udp.beginPacket(targetIP, targetPort);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
}

void OSCManager::sendMessage(const String &source, const String &command, var *data, int numData)
{
    OSCMessage msg = createMessage(source, command, data, numData, true);
    sendMessage(msg);
}

OSCMessage OSCManager::createMessage(const String &source, const String &command, const var *data, int numData, bool addID)
{
    OSCMessage msg((source + "/" + command).c_str());
    if (addID)
        msg.add(boardName.c_str());
    for (int i = 0; i < numData; i++)
    {
        switch (data[i].type)
        {
        case 'f':
            msg.add(data[i].floatValue());
            break;

        case 'i':
            msg.add((int32_t)data[i].intValue());
            break;

        case 's':
            msg.add(data[i].stringValue().c_str());
            break;

        case 'b':
            msg.add(data[i].boolValue());
            break;

        case 'T':
            msg.add(true);
            break;

        case 'F':
            msg.add(false);
            break;
        }
    }

    return msg;
}

var OSCManager::OSCArgumentToVar(OSCMessage &m, int index)
{
    if (m.isString(index))
    {
        char str[32];
        m.getString(index, str);
        return str;
    }
    else if (m.isInt(index))
        return (int)m.getInt(index);
    else if (m.isFloat(index))
        return m.getFloat(index);
    else if (m.isBoolean(index))
        return m.getBoolean(index);

    err("OSC Type not supported !");
    return var(0);
}
