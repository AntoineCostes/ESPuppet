#include <Arduino.h>
#include "../lib/ESPuppet/src/ESPuppet.h"

ESPuppet puppet;

bool usbSerial;
bool COMportOpened;
static void on_hwcdc_event(void*, esp_event_base_t base, int32_t id, void* data)
{
  if (base != ARDUINO_HW_CDC_EVENTS) return;
  if (id == ARDUINO_HW_CDC_BUS_RESET_EVENT) usbSerial = true;
}

void setup()
{
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#endif

  Serial.onEvent(on_hwcdc_event);
  Serial.begin(115200);
  if (usbSerial)  while(!Serial) delay(10); // wait for port COM to open
  Serial.println("====== ESPuppet ======");

  puppet.init();
}

void loop()
{
  puppet.update();
  delay(1);
}
