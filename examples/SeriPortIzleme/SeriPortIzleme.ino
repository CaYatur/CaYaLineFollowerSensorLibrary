// ============================================================
//  https://cayadev.com
// ============================================================
//  SeriPortIzleme.ino
//  ------------------------------------------------------------
//  Seri Plotter ile sensör çıktısını canlı görmek için örnek.
//  Arduino IDE -> Tools -> Serial Plotter (115200 baud)
//
//  Eğri kümeleri:
//    s0..s7  : Tek tek sensör değerleri
//    poz     : Ağırlıklı ortalama pozisyon (0..7000)
//    hata    : Pozisyon - 3500
// ============================================================

#include <CaYaLineFollowerSensor.h>

CaYaLineFollowerSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  sensor.oku();

  // Serial Plotter etiket başlığı (yalnızca okunaklı log için)
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(F("s")); Serial.print(i); Serial.print(':');
    Serial.print(sensor.deger(i)); Serial.print(' ');
  }
  Serial.print(F("poz:"));  Serial.print(sensor.pozisyon()); Serial.print(' ');
  Serial.print(F("hata:")); Serial.println(sensor.hata());

  delay(20);
}
