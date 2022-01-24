#include <ESP8266WiFi.h>
#include <DHTesp.h>
#include "AdafruitIO_WiFi.h"
#include "device.h"
#include "wifi_id.h"       // #define WIFI_SSID and WIFI_PASSWORD
#include "AdafruitIO_id.h" // #define IO_USERNAME and IO_KEY

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASSWORD);
AdafruitIO_Feed *feedHumidity = io.feed("humidity");
AdafruitIO_Feed *feedLedRed = io.feed("ledRed");
AdafruitIO_Feed *feedLedYellow = io.feed("ledYellow");
AdafruitIO_Feed *feedLedGreen = io.feed("ledGreen");
DHTesp dht;
long int nPrevMillis = 0;

void WifiConnect();

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  Serial.println("Booting...");
  Serial.print("Connecting to Adafruit IO...");
  io.connect();
  // set message handler to read feed from dashboard
  feedLedRed->onMessage([](AdafruitIO_Data *data)
                        { digitalWrite(LED_RED, data->toPinLevel()); });
  feedLedGreen->onMessage([](AdafruitIO_Data *data)
                          { digitalWrite(LED_GREEN, data->toPinLevel()); });
  feedLedYellow->onMessage([](AdafruitIO_Data *data)
                           { digitalWrite(LED_YELLOW, data->toPinLevel()); });
  // wait for a connection
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  digitalWrite(LED_BUILTIN, HIGH);
  nPrevMillis = millis(); // initialize
}

void loop()
{
  io.run();
  if (millis() - nPrevMillis >= 5000)
  {
    //send data
    nPrevMillis = millis();
    digitalWrite(LED_BUILTIN, LOW);

    float h = dht.getHumidity();
    float t = dht.getTemperature();
    if (dht.getStatus() == DHTesp::ERROR_NONE)
    {
      Serial.printf("Temp: %.2f, Humidity: %.2f\n", t, h);
      feedHumidity->save(h);
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
