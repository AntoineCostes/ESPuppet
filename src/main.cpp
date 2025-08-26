#include <Arduino.h>
#include "../lib/ESPuppet/src/ESPuppet.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"

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

  uint32_t efuse_value = 0;
  esp_efuse_read_field_blob(ESP_EFUSE_UART_PRINT_CONTROL, &efuse_value, 2);
  // 0: Default → send logs to UART0 (GPIO20/21).
  // 1: Send logs to USB CDC (if enabled).
  // 2: Disable logs completely.
  // 3: (Reserved / future use).
  if (efuse_value == 0)
  {
  // WARNING bits cannot be flipped back to 0
    efuse_value = 1;
    esp_err_t err = esp_efuse_write_field_blob(ESP_EFUSE_UART_PRINT_CONTROL, &efuse_value, 2); // value is on 2 bits
 
    if (err == ESP_OK) {
      Serial.println("Efuse burned: UART_PRINT_CONTROL = 1 (boot messages on USB CDC)");
    } else {
      Serial.printf(">>> ERROR burning efuse: %s\n", esp_err_to_name(err));
    }
  }

  puppet.init();
}

void loop()
{
  puppet.update();
  delay(1);
}
