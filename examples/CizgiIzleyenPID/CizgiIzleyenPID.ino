// ============================================================
//  https://cayadev.com
// ============================================================
//  CizgiIzleyenPID.ino
//  ------------------------------------------------------------
//  CaYaLineFollowerSensor + fixed-point PID ile tam bir çizgi
//  izleyen örneği.
//  PID mantığının kütüphane ile sadeleştirilmiş hali.
//
//  Donanım:
//    Sensör  : 8 kanal -> A0..A7
//    Motor A : AIN1=8, AIN2=9, PWMA=10  (Sağ motor)
//    Motor B : BIN1=7, BIN2=6, PWMB=5   (Sol motor)
//
//  Not: Bu örnek için 31 kHz PWM kullanılmıyor (sade tutmak için
//  analogWrite). Yüksek performans isterseniz orjinal koddaki
//  Timer1/Timer0 register ayarlarını ekleyin.
// ============================================================

#include <CaYaLineFollowerSensor.h>

// ---- Sensör ----
CaYaLineFollowerSensor sensor;

// ---- Motor pinleri ----
#define AIN1 8
#define AIN2 9
#define PWMA 10
#define BIN1 7
#define BIN2 6
#define PWMB 5

// ---- PID katsayıları (Fixed-Point Q8.8) ----
// Kp = 0.06  ->  0.06 * 256 = 15  (yumuşak)
// Kd = 0.40  ->  0.40 * 256 = 102 (sönümleme)
const int16_t KP_FP = 15;
const int16_t KD_FP = 102;

const int16_t baseSpeed = 180;   // Taban hız (0-255)
const int16_t maxSpeed  = 255;

long lastError = 0;

void setMotor(int16_t sol, int16_t sag) {
  // Sağ motor (A)
  if (sag >= 0) { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); }
  else          { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); sag = -sag; }
  // Sol motor (B)
  if (sol >= 0) { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); }
  else          { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); sol = -sol; }

  if (sag > 255) sag = 255;
  if (sol > 255) sol = 255;
  analogWrite(PWMA, sag);
  analogWrite(PWMB, sol);
}

void setup() {
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);

  sensor.begin();
}

void loop() {
  // 1) Sensörleri oku (zemin/pozisyon/hata otomatik hesaplanır)
  sensor.oku();
  long error = sensor.hata();

  // 2) PID — fixed-point çarpım, sonuç 8 bit kaydırılır
  long motorSpeed = ((long)KP_FP * error + (long)KD_FP * (error - lastError)) >> 8;
  lastError = error;

  // 3) Tekerlek hızlarını hesapla ve sınırla
  int16_t sol = baseSpeed - (int16_t)motorSpeed;
  int16_t sag = baseSpeed + (int16_t)motorSpeed;

  if (sol < -50)       sol = -50;
  if (sol > maxSpeed)  sol = maxSpeed;
  if (sag < -50)       sag = -50;
  if (sag > maxSpeed)  sag = maxSpeed;

  setMotor(sol, sag);
}
