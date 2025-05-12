#include "ota_manager.h"

static String githubToken;

void setGithubToken(const char* token) {
    githubToken = String(token);
}

bool performUpdate(const String& url) {
    HTTPClient http;
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    
    bool begun = http.begin(*client, url);
    if (!begun) {
        Serial.println("Failed to begin HTTP client");
        delete client;
        return false;
    }
    
    String authHeader = "Bearer " + githubToken;
    http.addHeader("Authorization", authHeader);
    http.addHeader("Accept", "application/octet-stream");
    
    int httpCode = http.GET();
    
    if (httpCode == 302) {
        String newUrl = http.getLocation();
        http.end();
        http.begin(*client, newUrl);
        httpCode = http.GET();
    }
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed, code: %d\n", httpCode);
        http.end();
        delete client;
        return false;
    }

    int contentLength = http.getSize();
    Serial.printf("Starting update (%d bytes)...\n", contentLength);

    if (!Update.begin(contentLength)) {
        Serial.printf("Not enough space: %s\n", Update.errorString());
        http.end();
        delete client;
        return false;
    }

    Update.onProgress([](size_t progress, size_t total) {
        static int lastPercent = 0;
        int percent = (progress * 100) / total;
        if (percent != lastPercent) {
            Serial.printf("Progress: %d%%\n", percent);
            lastPercent = percent;
        }
    });

    Stream& updateStream = http.getStream();
    size_t written = Update.writeStream(updateStream);
    
    if (written != contentLength) {
        Serial.println("Update failed: Size mismatch");
        http.end();
        delete client;
        return false;
    }

    if (!Update.end()) {
        Serial.printf("Update failed: %s\n", Update.errorString());
        http.end();
        delete client;
        return false;
    }

    http.end();
    delete client;
    Serial.println("Update successful! Rebooting...");
    delay(1000);
    ESP.restart();
    return true;
}

bool checkForUpdates() {
    Serial.println("Checking for updates...");
    
    HTTPClient http;
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    
    bool begun = http.begin(*client, GITHUB_API);
    if (!begun) {
        Serial.println("Failed to begin HTTP client");
        delete client;
        return false;
    }
    
    String authHeader = "Bearer " + githubToken;
    http.addHeader("Authorization", authHeader);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("Update check failed, HTTP code: %d\n", httpCode);
        http.end();
        delete client;
        return false;
    }

    String payload = http.getString();
    http.end();
    delete client;
    
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return false;
    }

    const char* latestVersion = doc["tag_name"];
    if (!latestVersion) {
        Serial.println("No version tag found in release");
        return false;
    }

    Serial.printf("Current version: %s, Latest version: %s\n", FIRMWARE_VERSION, latestVersion);
    
    if (strcmp(latestVersion, FIRMWARE_VERSION) <= 0) {
        Serial.println("Already on latest version");
        return false;
    }

    JsonArray assets = doc["assets"];
    if (assets.size() == 0) {
        Serial.println("No assets found in release");
        return false;
    }

    String firmwareUrl = assets[0]["url"].as<String>();
    Serial.printf("Using API asset URL: %s\n", firmwareUrl.c_str());
    return performUpdate(firmwareUrl);
} 