#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Bounce2.h>

#include "config.h"

extern "C" {
#include "user_interface.h"
}

#define HX711_DOUT D5
#define HX711_CLK  D6

#define MODE_SELECT_PERSON 1
#define MODE_USE_SCALE 2

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
HX711 scale;
Bounce button_mode = Bounce();
Bounce button_select = Bounce();

float calibration_factor = 20.71320754716981132075;
unsigned short action_mode = MODE_SELECT_PERSON;
unsigned short action_person = 1;

void display_person_select() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("select person:");
  for (short i=1; i<5; i++) {
    if (action_person == i) {
      display.setCursor(0, i * 10);
      display.print(">>");
    }
    display.setCursor(17, i * 10);
    display.println(String("person " + String(i)));
  }
  display.display();
}

void display_weight(float weight, bool send) {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println(String("person " + String(action_person) + ":"));

  display.setTextSize(2);
  display.setCursor(2, 20);
  display.print(String(weight / 1000));
  display.println(" kg");

  if (send == true) {
    display.setCursor(0, 40);
    display.println("... send");
  }
  display.display();
}

void weight_person() {
  float weight_summery = 0.0;

  for (short i=0; i<5; i++) {
    float weight = scale.get_units(10);
    weight_summery += weight;
    display_weight(weight, false);
    delay(300);
  }

  weight_summery = weight_summery / 5;

  char mqtt_queue_name[22] = "";
  sprintf(mqtt_queue_name, "/scale/weight/person%d", action_person);

  char mqtt_value[10] = "";
  sprintf(mqtt_value, "%.2f", weight_summery / 1000);

  client.publish(mqtt_queue_name, mqtt_value);
  display_weight(weight_summery, true);

  delay(5000);
}

void mqtt_check_connection() {
  if (!client.connected()) {
    client.connect("waage", MQTT_USERNAME, MQTT_PASSWORD);
  }
}

void wificonnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  mqtt_check_connection();
}

void setup() {
  WiFi.hostname("wifi-waage");
  Serial.begin(115200);
  Wire.begin(D1, D2);
  /**
  * D1 = SDA
  * D2 = SCL
  **/
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  display.setTextColor(WHITE);

  scale.begin(HX711_DOUT, HX711_CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Starting...");
  display.display();

  wificonnect();
  client.setServer(MQTT_SERVER, 1883);

  button_mode.attach(D3, INPUT_PULLUP);
  button_mode.interval(50);

  button_select.attach(D7, INPUT_PULLUP);
  button_select.interval(50);
}

void loop() {
  button_mode.update();
  button_select.update();

  if (WiFi.status() != WL_CONNECTED) {
    wificonnect();
  }
  mqtt_check_connection();
  client.loop();

  if (action_mode == MODE_SELECT_PERSON) {

    if(button_mode.fell()) {
      action_person++;
      if (action_person == 5) {
        action_person = 1;
      }
    }

    display_person_select();

    if (button_select.fell()) {
      action_mode = MODE_USE_SCALE;
    }
  }

  if (action_mode == MODE_USE_SCALE) {
    weight_person();
    action_mode = MODE_SELECT_PERSON;
  }
}

