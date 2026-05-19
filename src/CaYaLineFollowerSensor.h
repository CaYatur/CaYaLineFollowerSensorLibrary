// ============================================================
//  https://cayadev.com
// ============================================================
//  CaYaLineFollowerSensor.h
//  ------------------------------------------------------------
//  8 kanallı analog çizgi izleyen sensör kütüphanesi.
//  ATmega328P (Arduino Nano / Uno) için saf ADC register erişimi.
//
//  Özellikler:
//   - QTRSensors bağımlılığı YOK
//   - 8 kanal okuma ~104 us (analogRead'den ~7x hızlı)
//   - Otomatik zemin tespiti (siyah çizgi / beyaz çizgi)
//   - Ağırlıklı ortalama pozisyon (0 — 7000)
//   - Gürültü filtresi (sanal kalibrasyon — eşik altı = 0)
//   - Kenar sensörü okuma (90 derece dönüş tespiti için ~26 us)
//   - Hata (error) hesaplama (-3500 .. +3500)
//
//  Donanım varsayımı:
//   - Sensör 0 .. Sensör 7 sırasıyla A0 .. A7 pinlerine bağlı
//   - Sensörler "yansıtıcı" türde (analog çıkış; karanlık -> yüksek)
//
//  Yazar : CaYaDev
//  Versiyon: 1.0.0
//  Lisans: MIT
// ============================================================

#ifndef CAYA_LINE_FOLLOWER_SENSOR_H
#define CAYA_LINE_FOLLOWER_SENSOR_H

#include <Arduino.h>

// Zemin tipi sabitleri ----------------------------------------
// ZEMIN_BEYAZ : Beyaz zemin üzerinde siyah çizgi yok — beyaz zemin
//               üzerinde SİYAH çizgi takip ediliyor (klasik durum).
//               Sensörlerde "yüksek değer = çizgi".
// ZEMIN_SIYAH : Siyah zemin üzerinde BEYAZ çizgi takip ediliyor.
//               Sensörlerde "düşük değer = çizgi" — kütüphane
//               değerleri ters çevirir.
//
// Not: Mevcut çizgi izleyen kodlarındaki "zemin" değişkeniyle
// uyumlu olması için:
//   zemin == 1 -> beyaz zemin (siyah çizgi)  -> ZEMIN_BEYAZ
//   zemin == 0 -> siyah zemin (beyaz çizgi)  -> ZEMIN_SIYAH
#define ZEMIN_BEYAZ 1
#define ZEMIN_SIYAH 0

class CaYaLineFollowerSensor {
public:
  // -- Yapıcı ------------------------------------------------
  // Varsayılan eşiklerle kütüphaneyi oluşturur. Pin atamaları
  // sabittir: A0..A7.
  CaYaLineFollowerSensor();

  // -- Donanım kurulumu --------------------------------------
  // ADC prescaler = 16 (16 MHz / 16 = 1 MHz ADC clock)
  // Bir dönüşüm 13 ADC cycle = 13 us, 8 kanal ~ 104 us.
  // setup() içinde BİR KEZ çağrılmalı.
  void begin();

  // -- Tek kanal okuma (ham) ---------------------------------
  // 0-7 kanal numarası. 10-bit ham ADC değeri döner (0..1023).
  uint16_t adcOku(uint8_t kanal);

  // -- Tüm sensörleri oku + pozisyon hesapla -----------------
  // İç sensors[8] dizisini doldurur, zemini otomatik günceller,
  // gürültü filtresi uygular, ağırlıklı ortalama pozisyon
  // hesaplar ve error değerini (-3500..+3500) günceller.
  // Hiç çizgi görmüyorsa son error değeri korunur.
  void oku();

  // -- Sadece ham oku (pozisyon hesaplamadan) ----------------
  // Hızlı; pid() çağrılmadan sensor dizisinin tazelenmesi için.
  void okuHam();

  // -- Sadece kenar sensörlerini oku -------------------------
  // 0, 1, 6, 7 numaralı sensörleri okur (~26 us). 90 derece
  // dönüş tespiti gibi yüksek hızlı senaryolar için.
  void okuKenar();

  // -- Erişimciler -------------------------------------------
  // Son okunan ham sensör değerini döner (0..1023).
  uint16_t deger(uint8_t kanal) const;

  // Tüm dizinin pointer'ını döner (mevcut kodla uyumluluk için).
  // 8 elemanlı uint16_t dizisi.
  uint16_t* dizi();

  // Son hesaplanan hata değeri (-3500 .. +3500).
  // 0 = çizgi merkezde. Pozitif = robot çizginin solunda.
  long hata() const;

  // Son hesaplanan pozisyon (0..7000). Merkez = 3500.
  long pozisyon() const;

  // Otomatik tespit edilen zemin tipi (ZEMIN_BEYAZ / ZEMIN_SIYAH).
  uint8_t zeminTipi() const;

  // -- Konfigürasyon -----------------------------------------
  // Zemin tipini elle ayarla (otomatik tespiti devre dışı
  // bırakmadan da bir başlangıç değeri vermek için kullanılır).
  void zeminAyarla(uint8_t tip);

  // Otomatik zemin tespitini aç/kapat (varsayılan: açık).
  // Kapalıyken oku() çağrısı zemin değişkenine dokunmaz.
  void otomatikZeminAyarla(bool aktif);

  // Gürültü filtresi eşiği. Bu değerin altındaki sensör çıktısı
  // 0 kabul edilir (arka plan bastırma). Varsayılan: 250.
  void gurultuEsigiAyarla(uint16_t esik);

  // Otomatik zemin tespitinde kullanılan eşikler.
  // Tüm sensörler dusuk'un altındaysa -> beyaz zemin.
  // Tüm sensörler yuksek'in üstündeyse -> siyah zemin.
  // Varsayılan: 200 / 800.
  void zeminEsikleriAyarla(uint16_t dusuk, uint16_t yuksek);

  // -- 90 derece dönüş yardımcıları ---------------------------
  // Aşağıdaki iki fonksiyon SADECE sensör durumuna bakar; motor
  // sürmez. Dönüş manevrasını siz kontrol edersiniz.
  //
  // Sağa 90 derece dönüş koşulu:
  //   Sol taraf (0..3) eşik altında, sağ taraf (6,7) eşik üstünde.
  bool sagaDonusVar(uint16_t esikDusuk = 400, uint16_t esikYuksek = 600) const;

  // Sola 90 derece dönüş koşulu:
  //   Sağ taraf (4..7) eşik altında, sol taraf (0,1) eşik üstünde.
  bool solaDonusVar(uint16_t esikDusuk = 400, uint16_t esikYuksek = 600) const;

  // -- Kalibrasyon yok ---------------------------------------
  // Bu kütüphane TASARIM GEREĞİ kalibrasyon kullanmaz.
  // Gürültü eşiği (sanal kalibrasyon) tüm farkı kapatır.
  // Bu sayede çalıştığınız anda hazır olur ve sıcaklığa/aydınlığa
  // göre yeniden kalibre etmenize gerek kalmaz.

private:
  // Son okunan 8 kanal (0..1023)
  uint16_t _sensor[8];

  // Hesaplama sonuçları
  long _hata;
  long _pozisyon;

  // Zemin tipi (ZEMIN_BEYAZ / ZEMIN_SIYAH)
  uint8_t _zemin;

  // Konfigürasyon
  bool     _otomatikZemin;
  uint16_t _gurultuEsigi;
  uint16_t _zeminEsikDusuk;
  uint16_t _zeminEsikYuksek;
};

#endif // CAYA_LINE_FOLLOWER_SENSOR_H
