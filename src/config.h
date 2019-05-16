/*
 * This is a sample configuration file for the ESP01 MQTT plant watering system.
 *
 * Change the settings below and save the file as "config.h"
 * You can then upload the code using the Arduino IDE.
 */

// Pins
#define CONFIG_PIN_PUMP 0 // Output pin for water punp
#define CONFIG_PIN_FLOW_METER 1 // Input pin for water flow meter

// WiFi
#define CONFIG_WIFI_SSID "SSID" // WiFi SSID
#define CONFIG_WIFI_PASS "Password" // Corresponding WiFi password

// MQTT broker
#define CONFIG_MQTT_HOST "IP" // MQTT broker IP adress
#define CONFIG_MQTT_PORT 1883 // MQTT broker port (usually 1883)
#define CONFIG_MQTT_USER "Username" // MQTT broker username
#define CONFIG_MQTT_PASS "Password" // MQTT borker password
#define CONFIG_MQTT_CLIENT_ID "ESP_Watering" // MQTT broker client ID. Must be unique on the MQTT network
#define CONFIG_MQTT_UPDATE_FREQ 100 // MQTT status update delay in ms

// MQTT Topics
#define CONFIG_MQTT_TOPIC_STATE "home-assistant/watering" // MQTT topic for system status information
#define CONFIG_MQTT_TOPIC_SET "home-assistant/watering/set" // MQTT topic for set values
#define CONFIG_MQTT_TOPIC_AVAILABILITY "home-assistant/watering/availability" // MQTT topic for system avalability information

// MQTT Payloads
#define CONFIG_MQTT_PAYLOAD_ON "ON" // MQTT payload for indicating on-state
#define CONFIG_MQTT_PAYLOAD_OFF "OFF" // MQTT payload for indicating off-state
#define CONFIG_MQTT_PAYLOAD_ONLINE "online" // MQTT payload for indicating online-state
#define CONFIG_MQTT_PAYLOAD_OFFLINE "offline" // MQTT payload for indicating offline-state

// Flow Meter
#define CONFIG_FLOW_METER_PULSES 1925 * 3 / 2 // Flow Meter pulses per liter

// Enables Serial and print statements
#define CONFIG_DEBUG false
