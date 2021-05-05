#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "Button2.h"
#include "esp_adc_cal.h"
#include "bmp.h"
#include <CanonBLE.h>
#include <TimeLapse_Management.h>


#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            0
#define BUTTON_2            35

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

String name_remote = "Canon BLE Remote";
CanonBLE canon_ble(name_remote);

void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

void start_screen(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_DARKGREEN);
  tft.setCursor(0, 0);
  tft.println();

  btn1.setPressedHandler([](Button2 & b) {
      tft.println("button1");
  });

  btn2.setPressedHandler([](Button2 & b) {
      tft.println("button2");
  });
}


void setup() {
  Serial.begin(9600);
  Serial.println("Starting Canon BLE Remote...");
  BLEDevice::init("");

  /*
  ADC_EN is the ADC detection enable port
  If the USB port is used for power supply, it is turned on by default.
  If it is powered by battery, it needs to be set to high level
  */
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.setTextDatum(MC_DATUM);

  tft.fillScreen(TFT_YELLOW);
  tft.setCursor(1, 1);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  tft.println("Connecting...");

      
  while (! canon_ble.is_ready_to_connect())
  {
      canon_ble.scan(5);
  }

  Serial.print("Canon device found: ");
  Serial.println(canon_ble.get_device_address().toString().c_str());

  tft.print("Canon device found: ");
  tft.println(canon_ble.get_device_address().toString().c_str());
  espDelay(500);

  
  if (canon_ble.connect_to_device())
  {
    Serial.println("Connected successfully");
    tft.fillScreen(TFT_GREEN);
    tft.setCursor(1, 1);
    tft.println("Connected!");
    espDelay(1000);
  }

  start_screen();
}

void loop() {
  button_loop();
}
