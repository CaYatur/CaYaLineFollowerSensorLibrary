// ============================================================
//  https://cayadev.com
// ============================================================
//  CaYaLineFollowerSensor.cpp
//  Bkz. CaYaLineFollowerSensor.h
// ============================================================

#include "CaYaLineFollowerSensor.h"

CaYaLineFollowerSensor::CaYaLineFollowerSensor()
  : _hata(0),
    _pozisyon(3500),
    _zemin(ZEMIN_BEYAZ),       // varsayılan: beyaz zemin / siyah çizgi
    _otomatikZemin(true),
    _gurultuEsigi(250),         // sanal kalibrasyon eşiği
    _zeminEsikDusuk(200),
    _zeminEsikYuksek(800)
{
  for (uint8_t i = 0; i < 8; i++) _sensor[i] = 0;
}

// ---- Donanım kurulumu --------------------------------------
void CaYaLineFollowerSensor::begin() {
  // ADC prescaler = 16 -> 16 MHz / 16 = 1 MHz ADC clock
  // Bir dönüşüm = 13 ADC cycle = 13 us
  // 8 kanal = ~104 us
  ADCSRA = (1 << ADEN)        // ADC Enable
         | (1 << ADPS2)       // Prescaler bit 2 = 1
         | (0 << ADPS1)       // Prescaler bit 1 = 0
         | (0 << ADPS0);      // Prescaler bit 0 = 0  -> Prescaler = 16
  ADMUX  = (1 << REFS0);      // AVcc referans voltajı
}

// ---- Tek kanal ADC okuma (saf register) --------------------
uint16_t CaYaLineFollowerSensor::adcOku(uint8_t kanal) {
  ADMUX = (1 << REFS0) | (kanal & 0x07);   // AVcc, kanal seç
  ADCSRA |= (1 << ADSC);                    // Dönüşüm başlat
  while (ADCSRA & (1 << ADSC)) { }           // Bitene kadar bekle
  return ADC;                                // 10-bit sonuç
}

// ---- Tüm sensörleri oku + pozisyon hesapla ------------------
void CaYaLineFollowerSensor::oku() {
  // 1) Ham ADC değerleri (A0..A7)
  for (uint8_t i = 0; i < 8; i++) {
    _sensor[i] = adcOku(i);
  }

  // 2) Otomatik zemin tespiti
  if (_otomatikZemin) {
    // Tüm dış sensörler düşükse -> beyaz zemin (siyah çizgi)
    if (_sensor[0] < _zeminEsikDusuk && _sensor[7] < _zeminEsikDusuk) {
      _zemin = ZEMIN_BEYAZ;
    }
    // Tüm dış sensörler yüksekse -> siyah zemin (beyaz çizgi)
    if (_sensor[0] > _zeminEsikYuksek && _sensor[7] > _zeminEsikYuksek) {
      _zemin = ZEMIN_SIYAH;
    }
  }

  // 3) Ağırlıklı ortalama pozisyon
  //    pozisyon = Σ(deger[i] * i * 1000) / Σ(deger[i])
  //    Beyaz zemin (siyah çizgi): yüksek değer = çizgi
  //    Siyah zemin (beyaz çizgi): değerleri ters çevir
  uint32_t agirlikliToplam = 0;
  uint32_t toplam          = 0;

  for (uint8_t i = 0; i < 8; i++) {
    uint16_t d;
    if (_zemin == ZEMIN_SIYAH) {
      d = 1023 - _sensor[i];                // beyaz çizgi
    } else {
      d = _sensor[i];                       // siyah çizgi
    }

    // Gürültü filtresi (sanal kalibrasyon)
    // Eşik altı = 0, üstü = (değer - eşik)
    if (d < _gurultuEsigi) {
      d = 0;
    } else {
      d = d - _gurultuEsigi;
    }

    agirlikliToplam += (uint32_t)d * i * 1000UL;
    toplam          += d;
  }

  // Çizgi görüyorsa pozisyon ve hata güncellenir.
  // Görmüyorsa son değerler korunur -> robot çizgiden çıkmaz,
  // önceki yönüne devam eder.
  if (toplam > 0) {
    _pozisyon = (long)(agirlikliToplam / toplam);
    _hata     = _pozisyon - 3500L;
  }
}

// ---- Tüm sensörleri sadece ham olarak oku -------------------
void CaYaLineFollowerSensor::okuHam() {
  for (uint8_t i = 0; i < 8; i++) {
    _sensor[i] = adcOku(i);
  }
}

// ---- Kenar sensörlerini oku (0,1,6,7) ----------------------
void CaYaLineFollowerSensor::okuKenar() {
  _sensor[0] = adcOku(0);
  _sensor[1] = adcOku(1);
  _sensor[6] = adcOku(6);
  _sensor[7] = adcOku(7);
}

// ---- Erişimciler -------------------------------------------
uint16_t CaYaLineFollowerSensor::deger(uint8_t kanal) const {
  if (kanal > 7) return 0;
  return _sensor[kanal];
}

uint16_t* CaYaLineFollowerSensor::dizi() {
  return _sensor;
}

long    CaYaLineFollowerSensor::hata()       const { return _hata; }
long    CaYaLineFollowerSensor::pozisyon()   const { return _pozisyon; }
uint8_t CaYaLineFollowerSensor::zeminTipi()  const { return _zemin; }

// ---- Konfigürasyon -----------------------------------------
void CaYaLineFollowerSensor::zeminAyarla(uint8_t tip) {
  _zemin = (tip == ZEMIN_SIYAH) ? ZEMIN_SIYAH : ZEMIN_BEYAZ;
}

void CaYaLineFollowerSensor::otomatikZeminAyarla(bool aktif) {
  _otomatikZemin = aktif;
}

void CaYaLineFollowerSensor::gurultuEsigiAyarla(uint16_t esik) {
  _gurultuEsigi = esik;
}

void CaYaLineFollowerSensor::zeminEsikleriAyarla(uint16_t dusuk, uint16_t yuksek) {
  _zeminEsikDusuk  = dusuk;
  _zeminEsikYuksek = yuksek;
}

// ---- 90 derece dönüş tespiti --------------------------------
bool CaYaLineFollowerSensor::sagaDonusVar(uint16_t esikDusuk,
                                          uint16_t esikYuksek) const {
  // Sol taraf (0..3) eşik altı, sağ taraf (6,7) eşik üstü
  return  _sensor[0] < esikDusuk  && _sensor[1] < esikDusuk &&
          _sensor[2] < esikDusuk  && _sensor[3] < esikDusuk &&
          _sensor[6] > esikYuksek && _sensor[7] > esikYuksek;
}

bool CaYaLineFollowerSensor::solaDonusVar(uint16_t esikDusuk,
                                          uint16_t esikYuksek) const {
  // Sağ taraf (4..7) eşik altı, sol taraf (0,1) eşik üstü
  return  _sensor[7] < esikDusuk  && _sensor[6] < esikDusuk &&
          _sensor[5] < esikDusuk  && _sensor[4] < esikDusuk &&
          _sensor[1] > esikYuksek && _sensor[0] > esikYuksek;
}
