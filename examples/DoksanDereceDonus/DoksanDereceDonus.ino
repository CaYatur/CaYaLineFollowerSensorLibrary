// ============================================================
//  https://cayadev.com
// ============================================================
//  DoksanDereceDonus.ino
//  ------------------------------------------------------------
//  Çizgi izleme yaparken 90 derece köşeleri tespit etmek için
//  kenar sensör okuma + dönüş tespiti örneği.
//
//  ORIGINAL doksan.ino mantığıyla aynı:
//   - sagaDonusVar() doğru -> sert fren, sağa dön, çizgi sola
//     gelene kadar PID, sonra sola hafif düzeltme.
//   - solaDonusVar() simetrik.
//
//  Bu örnekte sadece tespit + manevra iskeleti yer alır;
//  motor sürücü detayları sade tutulmuştur.
// ============================================================

#include <CaYaLineFollowerSensor.h>

CaYaLineFollowerSensor sensor;

// ---- Motor pinleri (CizgiIzleyenPID örneğindekiyle aynı) ----
#define AIN1 8
#define AIN2 9
#define PWMA 10
#define BIN1 7
#define BIN2 6
#define PWMB 5

void setMotor(int16_t sol, int16_t sag) {
  if (sag >= 0) { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); }
  else          { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); sag = -sag; }
  if (sol >= 0) { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); }
  else          { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); sol = -sol; }
  if (sag > 255) sag = 255;
  if (sol > 255) sol = 255;
  analogWrite(PWMA, sag);
  analogWrite(PWMB, sol);
}

// ---- Sağa 90 derece dönüş manevrası -------------------------
bool sagaDon() {
  if (!sensor.sagaDonusVar(400, 600)) return false;

  // 1) Sert fren
  setMotor(-150, -120);
  delay(30);

  // 2) Çizgi sol kenara gelene kadar yerinde dön
  while (true) {
    sensor.okuKenar();
    setMotor(150, -120);   // sağa dön
    if (sensor.deger(0) > 600 || sensor.deger(1) > 600) break;
  }

  // 3) Overshoot düzeltmesi
  setMotor(-100, 100);
  delay(30);
  setMotor(0, 0);
  return true;
}

// ---- Sola 90 derece dönüş manevrası -------------------------
bool solaDon() {
  if (!sensor.solaDonusVar(400, 600)) return false;

  setMotor(-120, -150);
  delay(30);

  while (true) {
    sensor.okuKenar();
    setMotor(-120, 150);
    if (sensor.deger(6) > 600 || sensor.deger(7) > 600) break;
  }

  setMotor(100, -100);
  delay(30);
  setMotor(0, 0);
  return true;
}

void setup() {
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
  sensor.begin();
}

void loop() {
  sensor.oku();

  if (sagaDon())  return;
  if (solaDon())  return;

  // Aksi halde basit P-kontrol ile düz git
  long error = sensor.hata();
  int16_t duzeltme = (int16_t)(error / 20);
  setMotor(150 - duzeltme, 150 + duzeltme);
}
