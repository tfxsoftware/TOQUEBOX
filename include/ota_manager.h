#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Update.h>

// Function declarations
void setGithubToken(const char* token);
bool checkForUpdates();
bool performUpdate(const String& url);

// Constants
#define FIRMWARE_VERSION "1.0.0"  // Change this to match your version
#define GITHUB_API "https://api.github.com/repos/tfxsoftware/TOQUEBOX/releases/latest"  // Change this to your repo

#endif // OTA_MANAGER_H 