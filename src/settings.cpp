#include "settings.h"

#include <EEPROM.h>

void setSettings(Settings settings) {
  EEPROM.put(0, settings);
  EEPROM.commit();
}

Settings getSettings() {
  Serial.println("Getting");
  Settings settings;
  EEPROM.get(0, settings);
  return settings;
};