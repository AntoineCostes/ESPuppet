var ALPHA = "houdini_alpha";
var OMEGA = "houdini_omega";

function init() {
}

function update()
{
}

// PARAMETERS
function moduleParameterChanged(param)
{
  if (param.name == "yo") 
  {
    local.sendTo(ALPHA+".local", 12345, "/yo");
    local.sendTo(OMEGA+".local", 12345, "/yo");
    local.send("/yo");
  }
  if (param.name == "setPort")
    {
      local.send("/"+ALPHA+"/targetPort", local.parameters.oscInput.localPort.get());
      local.send("/"+OMEGA+"/targetPort", local.parameters.oscInput.localPort.get());
    }
}

// VALUES
function moduleValueChanged(value) {
}

// OSC
function oscEvent(address, args)
{
  // script.log("OSC Message received "+address+", "+args.length+" arguments");
  if (address.matches("/Dobbby/"+ALPHA+"/ip") && args.length == 1) local.parameters.oscOutputs.alpha.remoteHost.set(args[0]);
  if (address.matches("/Dobbby/"+OMEGA+"/ip") && args.length == 1) local.parameters.oscOutputs.omega.remoteHost.set(args[0]);
  if (address.matches("/Dobbby/"+ALPHA+"/port") && args.length == 1) local.parameters.oscOutputs.alpha.remotePort.set(args[0]);
  if (address.matches("/Dobbby/"+OMEGA+"/port") && args.length == 1) local.parameters.oscOutputs.omega.remotePort.set(args[0]);


}

// COMMANDS

// leds
function setLeds(strip, mode, color, param, speed, brightness)
{
  if (strip == 4) // all
  {
    setLedAlpha(0, mode, color, param, speed, brightness);
    setLedAlpha(1, mode, color, param, speed, brightness);
    setLedAlpha(2, mode, color, param, speed, brightness);
    setLedAlpha(3, mode, color, param, speed, brightness);
  } else
    setLedAlpha(strip, mode, color, param, speed, brightness);
}

function setLedAlpha(index, mode, color, param, speed, brightness)
{
  local.sendTo(local.parameters.oscOutputs.alpha.remoteHost.get(), 
  local.parameters.oscOutputs.alpha.remotePort.get(),
  "/"+ALPHA+"/ledstrip/set", index, mode, parseFloat(color[0]), color[1], color[2], param, speed, brightness);

}


// relays & dout ALPHA
function setArc(value)
{
  setRelayAlpha(0, value);
}

function setPropane(value)
{
  setRelayAlpha(1, value);
} 

function setValves(one, two, three, four)
{
  setRelayAlpha(2, one);
  setRelayAlpha(3, two);
  setRelayAlpha(4, three);
  setRelayAlpha(5, four);
}

function setValve(index, value)
{
  setRelayAlpha(1+index, value);
}

function setPump1(value)
{
  setRelayAlpha(6, value);
}

function setPump2(value)
{
  setRelayAlpha(7, value);
}


function setIndicator(value)
{
  setRelayOmega(2, value);
}

// relays & dout OMEGA
function setUV(value)
{
  setRelayOmega(0, value);
}

function setFan1(value)
{
  setRelayOmega(1, value);
}

function setLight(red, yellow, green)
{
  setRelayOmega(3, green);
  setRelayOmega(4, yellow);
  setRelayOmega(5, red);
}

function setPlasma(value)
{
  setRelayOmega(6, value);
}

// motors
function setEyebrows(index, value)
{
  if (index == 2) // both
  {
    setServoOmega(0, value*0.5+0.5);
    setServoOmega(1, value*0.5+0.5);
  } else
    setServoOmega(index, value*0.5+0.5);
}

function setServos(one, two, three)
{
  setServoOmega(2, parseFloat(one));
  setServoOmega(3, parseFloat(two));
  setServoOmega(4, parseFloat(three));
}

// ALPHA
function setMosfetAlpha(index, val)
{
  local.sendTo(local.parameters.oscOutputs.alpha.remoteHost.get(), 
  local.parameters.oscOutputs.alpha.remotePort.get(), 
  "/"+ALPHA+"/gpio/output", index, val);

}

function setRelayAlpha(index, val)
{
  local.sendTo(local.parameters.oscOutputs.alpha.remoteHost.get(), 
  local.parameters.oscOutputs.alpha.remotePort.get(), 
  "/"+ALPHA+"/gpio/output", index, val);
}


// OMEGA
function setRelayOmega(index, val)
{
  local.sendTo(local.parameters.oscOutputs.omega.remoteHost.get(), 
  local.parameters.oscOutputs.omega.remotePort.get(), 
  "/"+OMEGA+"/gpio/output", index, val);
}

function setServoOmega(index, val)
{
  local.sendTo(local.parameters.oscOutputs.omega.remoteHost.get(), 
  local.parameters.oscOutputs.omega.remotePort.get(), 
  "/"+OMEGA+"/servo/set", index, val);
}
