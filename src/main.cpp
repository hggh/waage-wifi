#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Bounce2.h>

#define HX711_DOUT D5
#define HX711_CLK  D6

Adafruit_SSD1306 display(128, 64, &Wire, -1);
HX711 scale;

float calibration_factor = 20.71320754716981132075;

void display_scale() {
  float weight = scale.get_units(10);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(2, 0);
  display.print(String(weight / 1000));
  display.println(" kg");
  display.display();
}

void setup() {
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
  display.println("Starting..");
  display.display();
}

void loop() {
  display_scale();
  delay(800);
}

