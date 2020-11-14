#include <Arduino.h>
#include <WiFi.h>

#include "iot_hub_service.h"
#include "led.h"
#include "wifi_config.h"

// The time needs to be set, so set up details for an NTP connection
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

// The IoT Hub service
IoTHubService iot_service;

// Arduino setup function. This is run once by the OS when the
// device first starts up.
// This function intializes the WiFi connection, and starts the web server
void setup()
{
  // Start the serial port at the PlatformIO default speed for debugging
  // You can view the output of this code by connecting to the serial monitor
  Serial.begin(9600);
  Serial.println();

  // Connect to Wi-Fi using the SSID and password from the Config.h file
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // It can take a few seconds to connect, so loop whilst waiting for the WiFi
  // to connect, waiting a second each time before checking the connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Turn on the WiFI LED
  toggle_wifi_led(1);

  // Create and connect the IoT Hub service
  iot_service = IoTHubService();
}

int current_delay = 0;
const int delay_between_telemetry = 5000;
const int delay_between_work = 1000;

void loop()
{
  delay(delay_between_work);

  toggle_azure_led(1);

  iot_service.DoWork();

  current_delay += delay_between_work;
  if (current_delay >= delay_between_telemetry)
  {
    current_delay = 0;

    long temperature = random(100);
    char buff[50];
    sprintf(buff, "{\"Temperature\":%ld}", temperature);

    iot_service.SendTelemetry(buff);
  }

  toggle_azure_led(0);
}