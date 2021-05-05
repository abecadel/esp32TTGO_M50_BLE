#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "Button2.h"
#include "esp_adc_cal.h"
#include "bmp.h"
#include <CanonBLE.h>

#define ADC_EN 14 //ADC_EN is the ADC detection enable port
#define ADC_PIN 34
#define BUTTON_1 0
#define BUTTON_2 35

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

String name_remote = "Canon BLE Remote";
CanonBLE canon_ble(name_remote);

#define DEFAULT_INTERVAL 5
#define DEFAULT_N_CAPTURES 10
#define DEFAULT_SHUTTER_SPEED 30

int interval = DEFAULT_INTERVAL;
int n_captures = DEFAULT_N_CAPTURES;
int shutter_speed = DEFAULT_SHUTTER_SPEED;

void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void draw_submenu_value(int val)
{
  tft.setTextColor(TFT_RED);
  tft.setTextSize(12);
  tft.drawNumber(val, 50, 90);
}

void draw_submenu_title(const char *menu_title)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println(menu_title);
}

void draw_submenu_options(const char *option1, const char *option2)
{
  tft.setTextSize(1);
  tft.setCursor(175, 0);
  tft.print(option1);

  tft.setCursor(175, 120);
  tft.print(option2);
}

// define screens
void capturing_screen();
void start_screen();
void set_shutter_speed();
void set_interval_screen();
void set_n_captures_screen();
void first_screen();

void capturing_screen()
{
  draw_submenu_title("Capturing...");
  draw_submenu_options("pause->", "back->");

  btn1.setPressedHandler([](Button2 &b) {
    start_screen();
  });

  btn2.setPressedHandler([](Button2 &b) {
    //i pykz
    canon_ble.trigger();
  });
}

void start_screen()
{
  draw_submenu_title("Start capture?");
  draw_submenu_options("start->", "back->");
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  tft.print("No of shots: ");
  tft.println(n_captures);
  tft.print("Interval(s): ");
  tft.println(interval);
  tft.print("Shutter speed(s): ");
  tft.println(shutter_speed);

  btn1.setPressedHandler([](Button2 &b) {
    first_screen();
  });

  btn1.setLongClickHandler([](Button2 &b) {
  });

  btn2.setPressedHandler([](Button2 &b) {
    capturing_screen();
  });
}

void set_shutter_speed()
{
  draw_submenu_title("Shutter(s):");
  draw_submenu_options("next->", "inc/rst->");
  draw_submenu_value(shutter_speed);

  btn1.setPressedHandler([](Button2 &b) {
    shutter_speed += 1;
    draw_submenu_title("Shutter(s):");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(shutter_speed);
  });

  btn1.setLongClickHandler([](Button2 &b) {
    shutter_speed = 0;
    draw_submenu_title("Shutter(s):");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(shutter_speed);
  });

  btn2.setPressedHandler([](Button2 &b) {
    start_screen();
  });
}

void set_interval_screen()
{
  draw_submenu_title("Interval(s):");
  draw_submenu_options("next->", "inc/rst->");
  draw_submenu_value(interval);

  btn1.setPressedHandler([](Button2 &b) {
    interval += 1;
    draw_submenu_title("Interval(s):");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(interval);
  });

  btn1.setLongClickHandler([](Button2 &b) {
    interval = 0;
    draw_submenu_title("Interval(s):");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(interval);
  });

  btn2.setPressedHandler([](Button2 &b) {
    set_shutter_speed();
  });
}

void set_n_captures_screen()
{
  draw_submenu_title("Shots:");
  draw_submenu_options("next->", "inc/rst->");
  draw_submenu_value(n_captures);

  btn1.setPressedHandler([](Button2 &b) {
    n_captures += 1;
    draw_submenu_title("Shots:");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(n_captures);
  });

  btn1.setLongClickHandler([](Button2 &b) {
    n_captures = 0;
    draw_submenu_title("Shots:");
    draw_submenu_options("next->", "inc/rst->");
    draw_submenu_value(n_captures);
  });

  btn2.setPressedHandler([](Button2 &b) {
    set_interval_screen();
  });
}

void first_screen()
{
  draw_submenu_title("Intervalometer setup wizard...");
  espDelay(3000);
  set_n_captures_screen();
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting Canon BLE Remote...");
  // BLEDevice::init("");

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

  while (!canon_ble.is_ready_to_connect())
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

  first_screen();
}

void loop()
{
  btn1.loop();
  btn2.loop();
}
