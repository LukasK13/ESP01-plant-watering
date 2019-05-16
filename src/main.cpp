/*
 * ESP8266 MQTT Plant watering system
 *
 * DIY plant watering system for Home Assistant using MQTT and JSON.
 *
 * This software connects to a given WiFI network and a given MQTT server.
 * JSON formatted messages are used for exchanging information with the commanding unit.
 *
 * Copy the included `config-sample.h` file to `config.h` and update
 * accordingly for your setup.
 *
 * Inspired by https://github.com/corbanmailloux/esp-mqtt-rgb-led
 */

#include "config.h" // Set configuration options for pins, WiFi, and MQTT in this file
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // http://pubsubclient.knolleary.net/
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

const int JSON_DOCUMENT_SIZE = JSON_OBJECT_SIZE(3); // JSON buffer is used for handling JSON objects
bool state = false; // state refers to the state of the pump: on / off
unsigned long millis_time; // Time for status update delay
float volumeTotal = 0.0; // Total commanded volume for plant watering in ml
float volumeCurrent = -1.0; // Currently flown volume

WiFiClient wifi; // Create WiFiClient object
PubSubClient mqtt(wifi); // Create PubSubClient object

/*
 * Set up WiFi
 * 
 * This function allows to connect to a given WiFi Access Point
 * using a given passwort. Debug information will be printed
 * to the serial interface.
 */
void setup_wifi() {
	delay(10);
	Serial.println(); // Print debug info
	Serial.print("Connecting to "); // Print debug info
	Serial.println(CONFIG_WIFI_SSID); // Print debug info

	WiFi.mode(WIFI_STA); // Disable the built-in WiFi access point.
	WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASS); // Connect to given network

	while (WiFi.status() != WL_CONNECTED) { // Loop until connected to WiFi network
		delay(500); // Wait 500 ms
		Serial.print("."); // Print debug info
	}

	Serial.println(""); // Print debug info
	Serial.println("WiFi connected"); // Print debug info
	Serial.println("IP address: "); // Print debug info
	Serial.println(WiFi.localIP()); // Print debug info
}

/*
 * Process incoming JSON formatted message
 * 
 * This function processes an incoming JSON formatted
 * message from the MQTT broker. The message is deparsed
 * and the new values assigned to the corresponding variables.
 */
bool processJson(char* message) {
	StaticJsonDocument<JSON_DOCUMENT_SIZE> jsonDocument; // Initialize new JSON document

	auto error = deserializeJson(jsonDocument, message); // parse message to JSON object

	if (error) { // parsing message failed
		Serial.println("parseObject() failed"); // Print debug info
    Serial.print(F("deserializeJson() failed: ")); // Print debug info
    Serial.println(error.c_str()); // Print debug info
		return false; // return with failure status
	}

	if (jsonDocument.containsKey("state")) { // JSON object contains state key
		if (strcmp(jsonDocument["state"], CONFIG_MQTT_PAYLOAD_ON) == 0) { // state on is requested
			state = true; // set state to on
		}
		else if (strcmp(jsonDocument["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0) { // state off is requested
			state = false;// set state to off
		}
	}

	if (jsonDocument.containsKey("volume")) { // JSON object contains volume key
		volumeTotal = (float) jsonDocument["volume"]; // set total volume
	}

	return true; // return with success status
}

/*
 * Publish JSON formatted state to MQTT broker
 * 
 * This function sends the current state of the
 * system to the MQTT broker as JSON formatted message.
 *
 * Sample Payload:
 * {
 *   "volumeTarget": 120,
 *   "volumeCurrent": 110,
 *   "state": "ON"
 * }
 */
void sendState() {
	StaticJsonDocument<JSON_DOCUMENT_SIZE> jsonDocument; // Initialize new JSON document

	jsonDocument["state"] = (state) ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF; // Create and assign state key
	jsonDocument["volumeTarget"] = volumeTotal; // Create and assign total volume key
	jsonDocument["volumeCurrent"] = (volumeCurrent == -1.0) ? 0.0 : volumeCurrent; // Create and assign current volume key

	char buffer[measureJson(jsonDocument) + 1]; // Define buffer for JSON message
	serializeJson(jsonDocument, buffer, sizeof(buffer)); // Encode JSON object as string
	mqtt.publish(CONFIG_MQTT_TOPIC_STATE, buffer, true); // Publish JSON message to MQTT server
}

/*
 * Callback function for MQTT client
 * 
 * This function is called every time the set-topic
 * is changed. It processes the incoming new message
 * and sends the new system status to the MQTT broker.
 *
 * Sample Payload:
 * {
 *   "volumeTarget": 120,
 *   "state": "ON"
 * }
 */
void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("New meessage arrived: ["); // Print debug info
	Serial.print(topic); // Print debug info
	Serial.print("] "); // Print debug info

	char message[length + 1]; // Create new empty character array
	for (unsigned int i = 0; i < length; i++) { // Copy message to character array
		message[i] = (char)payload[i]; // Copy byte to character array
	}
	message[length] = '\0'; // Terminate message
	Serial.println(message); // Print debug info

	if (processJson(message)) { // processing JSON successful
		sendState(); // Update MQTT system status
	}
}

/*
 * Connect to MQTT broker
 * 
 * This function connects to the given MQTT broker using
 * the given parameters. The last will for the MQTT connection
 * is setting the availability topic to offline.
 * Status information will be printed to the serial interface
 * for debugging purposes.
 */
void MQTTconnect() {
	while (!mqtt.connected()) { // Loop until connected
		Serial.print("Attempting MQTT connection..."); // Print debug info
		if (mqtt.connect(CONFIG_MQTT_CLIENT_ID, CONFIG_MQTT_USER, CONFIG_MQTT_PASS, CONFIG_MQTT_TOPIC_AVAILABILITY, 0, 1, CONFIG_MQTT_PAYLOAD_OFFLINE)) { // Connect was successful
			Serial.println("connected"); // Print debug info
			mqtt.publish(CONFIG_MQTT_TOPIC_AVAILABILITY, CONFIG_MQTT_PAYLOAD_ONLINE, true); // Set system availability to online
			sendState(); // Update MQTT system status
			mqtt.subscribe(CONFIG_MQTT_TOPIC_SET); // Subscripe to set value topic
		} else { // Connect failed
			Serial.print("failed, rc="); // Print debug info
			Serial.print(mqtt.state()); // Print debug info
			Serial.println(" try again in 5 seconds"); // Print debug info
			delay(5000); // Wait 5 seconds before retrying
		}
	}
}

/*
 * Interrupt handler for flow meter
 * 
 * This function is called on every falling edge of
 * the flow meter. It increments the currently flown
 * water volume.
 */
void ICACHE_RAM_ATTR pulseCounter() { // link interrupt handler to RAM
	volumeCurrent += 1000.0 / CONFIG_FLOW_METER_PULSES; // Increment flown water volume
}

/*
 * Set up all necessary services at startup
 * 
 * This function is automatically called once during
 * the system startup. It sets up all necessary services.
 * Afterwarts, the loop funciton will be executed repeatedly.
 */
void setup() {
	// Set up pin modes
	pinMode(CONFIG_PIN_PUMP, OUTPUT); // Set pump pin mode to output
  if (!CONFIG_DEBUG) { // Debug mode is disabled
	  pinMode(CONFIG_PIN_FLOW_METER, INPUT); // Set flow meter pin mode to input
  }

	// Set up the serial interface
	if (CONFIG_DEBUG) { // Only set up serial interface if debug mode is enabled
		Serial.begin(115200); // Set serial baudrate to 115200 baud/s
	}

	// Set up WiFi and MQTT
	setup_wifi(); // Execute WiFi setup
	mqtt.setServer(CONFIG_MQTT_HOST, CONFIG_MQTT_PORT); // Set MQTT server
	mqtt.setCallback(callback); // Register MQTT callback function
}

/*
 * Infinite loop
 */
void loop() {
	if (!mqtt.connected()) { // No longer connected to MQTT server
		MQTTconnect(); // Attempt to reconnect to the MQTT server
	}

	if (!mqtt.loop()) { // Maintaining connection to MQTT server failed
		MQTTconnect(); // Attempt to reconnect to the MQTT server
	}

	mqtt.loop(); // Maintain connection to MQTT server
	if (state) { // Plant watering is activated
		if (volumeCurrent == -1.0) { // Pump is not activated yet
			volumeCurrent = 0.0; // Reset currently flown volume
			attachInterrupt(digitalPinToInterrupt(CONFIG_PIN_FLOW_METER), pulseCounter, FALLING); // Attach interrupt for flow meter
			digitalWrite(CONFIG_PIN_PUMP, HIGH); // Activate pump
      millis_time = millis(); // Save current system time for status update delay
			Serial.println("Watering plants."); // Print debug message
		} else if (volumeCurrent >= volumeTotal) { // Volume limit reached
      digitalWrite(CONFIG_PIN_PUMP, LOW); // Deactivate pump
			detachInterrupt(digitalPinToInterrupt(CONFIG_PIN_FLOW_METER)); // Detach interrupt for flow meter
			volumeCurrent = -1.0; // Set current volume to -1.0 to indicate pump deactivation
			state = false; // set pump state variable to off
      sendState(); // Update MQTT system status
			Serial.println("Finished watering plants."); // Print debug message
		} else if (millis() - millis_time >= CONFIG_MQTT_UPDATE_FREQ){ // Plant Watering is ongoing and status update is due
      millis_time = millis(); // Save current system time for status update delay
      //volumeCurrent = volumeCurrent + 1.0; // Dummy increment current volume for testing purposes without flow meter
      sendState(); // Update MQTT system status
		}
	}
}