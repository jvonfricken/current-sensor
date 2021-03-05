#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct settings {
  char host[15];
  char client_id[15];
} Settings;

void setSettings(Settings settings);

Settings getSettings();

#endif