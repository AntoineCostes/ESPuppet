#include <Arduino.h>
#include "../lib/ESPuppet/src/ESPuppet.h"

ESPuppet puppet;

// check memory diff Preferences / LittleFS: LittleFS max size ?

void setup()
{
  Serial.begin(115200);
  delay(3000);
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
