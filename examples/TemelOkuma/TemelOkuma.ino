// ============================================================
//  https://cayadev.com
// ============================================================
//  TemelOkuma.ino
//  ------------------------------------------------------------
//  CaYaLineFollowerSensor kütüphanesinin en temel kullanımı.
//  Tüm sensörleri okur, pozisyon ve hata değerlerini Seri Porta
//  basar. Robot devre dışıyken sensör dizisini elle çizginin
//  üzerinde gezdirip dönen değerleri gözlemleyebilirsiniz.
//
//  Bağlantı:
//    Sensör 0 -> A0   ...   Sensör 7 -> A7
//
//  Donanım: Arduino Nano / Uno (ATmega328P @ 16 MHz)
// ============================================================

#include <CaYaLineFollowerSensor.h>

CaYaLineFollowerSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  sensor.oku();   // 8 kanal + zemin tespiti + pozisyon + hata

  // Ham değerleri yazdır
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(sensor.deger(i));
    Serial.print('\t');
  }

  // Hesaplanan büyüklükler
  Serial.print(F("poz="));
  Serial.print(sensor.pozisyon());
  Serial.print(F("\thata="));
  Serial.print(sensor.hata());
  Serial.print(F("\tzemin="));
  Serial.println(sensor.zeminTipi() == ZEMIN_BEYAZ ? F("beyaz") : F("siyah"));

  delay(100);
}
