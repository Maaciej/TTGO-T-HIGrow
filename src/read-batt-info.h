#include <Arduino.h>

// Read battery charging info
void read_batt_info() {
  readFile(SPIFFS, "/batinfo.conf");
  Serial.println("Here comes the charge date info info:");
  Serial.println(readString);

  batt_charge_epoc = getValue(readString, ':', 0);
  config.battery_charge_date = getValue(readString, ':', 1);

  readString = "";
}