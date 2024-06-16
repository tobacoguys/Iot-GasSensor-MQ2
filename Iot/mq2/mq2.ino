#define BLYNK_TEMPLATE_ID "TMPL6CxhRlaDS"
#define BLYNK_TEMPLATE_NAME "GAS"
#define BLYNK_AUTH_TOKEN "4UjqaIpoQRSf96bbOPxXm89hHxCZkDwH"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Thêm thư viện LiquidCrystal_I2C

BlynkTimer timer;
int timerID1, timerID2;

// D1.D2 - SCL.SDA
int mucCanhbao = 250;
int mq2_value;
int coi = 13;        // D7
int ledMode = 12;    // D6 led hiển thị chế độ hoạt động
int rl = 14;         // D5
int warningLed = 2;  // D4 LED trên ESP8266 để hiển thị cảnh báo
int button1 = 0;     // D3 Bật tắt chế độ cảnh báo
boolean button1State = HIGH;
boolean runMode = 1;  // Bật/tắt chế độ cảnh báo
boolean canhbaoState = 0;

WidgetLED led(V0);
#define KHIGAS V1
#define MUCCANHBAO V2
#define RUNMODE V3
#define TRANGTHAICB V4

// Khai báo địa chỉ I2C cho LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C 0x27 với LCD 16x2

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(button1, INPUT_PULLUP);
  pinMode(coi, OUTPUT);
  digitalWrite(coi, LOW);  // Tắt coi
  pinMode(rl, OUTPUT);
  digitalWrite(rl, LOW);  // Tắt rl
  pinMode(ledMode, OUTPUT);
  digitalWrite(ledMode, LOW);  // Tắt led mode
  pinMode(warningLed, OUTPUT);
  digitalWrite(warningLed, LOW);                               // Tắt LED cảnh báo
  lcd.begin(16, 2);                                            // Khởi tạo LCD với 16 cột và 2 hàng
  lcd.init();                                                  // Khởi tạo LCD I2C
  lcd.backlight();                                             // Bật đèn nền LCD
  Blynk.begin(BLYNK_AUTH_TOKEN, "Quan_Dz", "12345678");       // Thay thế bằng SSID và Password của bạn
  timerID1 = timer.setInterval(1000L, handleTimerID1);
}

void loop() {
  Blynk.run();
  timer.run();
  if (digitalRead(button1) == LOW) {
    if (button1State == HIGH) {
      button1State = LOW;
      runMode = !runMode;
      digitalWrite(ledMode, runMode);
      Serial.println("Run mode: " + String(runMode));
      Blynk.virtualWrite(RUNMODE, runMode);
      delay(200);
    }
  } else {
    button1State = HIGH;
  }
}

void handleTimerID1() {
  mq2_value = analogRead(A0);
  Blynk.virtualWrite(KHIGAS, mq2_value);
  Serial.println("Nồng độ khí gas: " + String(mq2_value));  // Hiển thị nồng độ khí gas trên Serial Monitor
  if (led.getValue()) {
    led.off();
  } else {
    led.on();
  }
  if (runMode == 1) {
    if (mq2_value > mucCanhbao) {
      if (canhbaoState == 0) {
        canhbaoState = 1;
        Blynk.logEvent("canhbao", String("Cảnh báo! Khí gas=" + String(mq2_value) + " vượt quá mức cho phép!"));
        digitalWrite(coi, HIGH);
        digitalWrite(rl, HIGH);
        digitalWrite(warningLed, HIGH);  // Bật LED cảnh báo
        Blynk.virtualWrite(TRANGTHAICB, HIGH);
        Serial.println("Đã bật cảnh báo!");
        timerID2 = timer.setTimeout(50000L, handleTimerID2);
      }
      // Hiển thị thông báo cảnh báo trên LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CANH BAO: " + String(mq2_value));
      lcd.setCursor(0, 1);
      lcd.print("RO RI KHI GAS!");
    }
  } else {
    digitalWrite(coi, LOW);
    digitalWrite(rl, LOW);
    digitalWrite(warningLed, LOW);  // Tắt LED cảnh báo
    Blynk.virtualWrite(TRANGTHAICB, LOW);
    Serial.println("Đã tắt cảnh báo!");
    canhbaoState = 0;
    // Hiển thị nồng độ khí gas trên LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nong Do Gas: " + String(mq2_value));
    lcd.setCursor(0, 1);
    lcd.print("OKE!");
  }
}

void handleTimerID2() {
  canhbaoState = 0;
  if (mq2_value < mucCanhbao) {
    digitalWrite(coi, LOW);
    digitalWrite(rl, LOW);
    digitalWrite(warningLed, LOW);  // Tắt LED cảnh báo
    Blynk.virtualWrite(TRANGTHAICB, LOW);
    Serial.println("Đã tắt cảnh báo!");
    // Hiển thị nồng độ khí gas trên LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nong Do Gas: " + String(mq2_value));
    lcd.setCursor(0, 1);
    lcd.print("OKE!");
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(RUNMODE, MUCCANHBAO);
}

BLYNK_WRITE(MUCCANHBAO) {
  mucCanhbao = param.asInt();
  Serial.println("Mức cảnh báo: " + String(mucCanhbao));
}

BLYNK_WRITE(RUNMODE) {
  runMode = param.asInt();
  digitalWrite(ledMode, runMode);
}
