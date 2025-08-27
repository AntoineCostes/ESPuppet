
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
    local.sendTo("frigo.local", 12345, "/yo");
    local.send("/yo");
  }
  if (param.name == "setPort") local.send("/targetPort", local.parameters.oscInput.localPort.get());
}

// VALUES
function moduleValueChanged(value) {
}

// OSC
function oscEvent(address, args)
{
  // script.log("OSC Message received "+address+", "+args.length+" arguments");
  if (address.matches("/frigo/ip") && args.length == 1) local.parameters.oscOutputs.oscOutput.remoteHost.set(args[0]);
}

// COMMANDS
function setLeds(mode, color, param, brightness)
{
  local.send("/ledstrip/set", 0, mode, parseFloat(color[0]), parseFloat(color[1]), parseFloat(color[2]), param, brightness);
}

function setGyrophare(value)
{
  setRelay(0, value);
}

function set5V(value)
{
  setRelay(1, value);
}

function setSonnette(value)
{
  setRelay(2, value);
}

function setRelay2(value)
{
  setRelay(3, value);
}

function setRelay3(value)
{
  setRelay(4, value);
}

function setRelay4(value)
{
  setRelay(5, value);
}

function setRelay(index, val)
{
  local.send("/gpio/output", index, val);
}
