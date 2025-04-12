#include <Arduino.h>
#include "../lib/ESPuppet/src/ESPuppet.h"

ESPuppet puppet;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#endif

  puppet.init();
}

void loop()
{
  puppet.update();
  delay(1);
}
