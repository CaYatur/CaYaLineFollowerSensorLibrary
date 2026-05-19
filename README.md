// https://cayadev.com

# CaYaLineFollowerSensor

[TÜRKÇE](#tr) | [ENGLISH](#en)

---

<a name="tr"></a>

## 🇹🇷 TÜRKÇE

### CaYaLineFollowerSensor

8 kanallı analog çizgi izleyen sensör dizileri için **hızlı**, **kalibrasyonsuz** ve **bağımlılıksız** Arduino kütüphanesi.

> ATmega328P (Arduino Nano / Uno) için saf ADC register erişimi kullanır. QTRSensors bağımlılığı **yoktur**.

---

## Neden bu kütüphane?

| Özellik | analogRead() | QTRSensors | **CaYaLineFollowerSensor** |
|---|---|---|---|
| 8 kanal okuma süresi | ~880 µs | ~400 µs | **~104 µs** |
| Kalibrasyon gerekir mi? | — | Evet | **Hayır** |
| Otomatik zemin tespiti | — | Hayır | **Evet** |
| Bağımlılık | — | QTRSensors.h | **Yok** |
| Donanım | Tümü | Tümü | ATmega328P |

Kalibrasyon yerine **gürültü eşiği (sanal kalibrasyon)** kullanır: eşik altındaki sensör çıktısı 0 sayılır, üstündeki net sinyal olarak işlenir. Bu sayede:

- Açtığınız anda hazır olur,
- Sıcaklık, aydınlatma değişikliklerinden çok daha az etkilenir,
- Sahada yarış başlamadan dakikalarca kalibrasyon yapılmaz.

---

## Donanım gereksinimleri

- ATmega328P tabanlı bir kart: **Arduino Nano** veya **Arduino Uno** (16 MHz).
- 8 kanallı analog çizgi sensörü (Pololu QTR-8A benzeri).
- Sensör çıkışları sırasıyla **A0 — A7** pinlerine bağlı.

> Kütüphane doğrudan ADC register'larına yazdığı için ATmega328P dışındaki kartlarda (ESP32, STM32, RP2040, Mega2560 vb.) **çalışmaz**.

---

## Kurulum

1. Bu klasörü olduğu gibi Arduino kütüphane klasörünüze kopyalayın:
   - Windows: `Belgeler\Arduino\libraries\CaYaLineFollowerSensor\`
   - macOS: `~/Documents/Arduino/libraries/CaYaLineFollowerSensor/`
2. Arduino IDE'yi yeniden başlatın.
3. `Dosya → Örnekler → CaYaLineFollowerSensor` altında örnekleri bulun.

---

## Hızlı başlangıç

```cpp
#include <CaYaLineFollowerSensor.h>

CaYaLineFollowerSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();          // ADC kurulumu
}

void loop() {
  sensor.oku();            // 8 kanal + zemin + pozisyon + hata
  long error = sensor.hata();   // -3500 .. +3500
  // motorları error'a göre sür ...
}
```

---

## API referansı

### Kurulum

| Fonksiyon | Açıklama |
|---|---|
| `begin()` | ADC prescaler ve AVcc referansını ayarlar. `setup()` içinde bir kez çağırılır. |

### Okuma

| Fonksiyon | Süre | Ne yapar? |
|---|---|---|
| `oku()` | ~110 µs | 8 kanal okur, zemini günceller, pozisyon ve hata hesaplar. **Ana kullanım.** |
| `okuHam()` | ~104 µs | Sadece 8 kanal okur, hesap yapmaz. |
| `okuKenar()` | ~26 µs | Sadece 0, 1, 6, 7 kanallarını okur. 90° dönüş döngülerinde idealdir. |
| `adcOku(kanal)` | ~13 µs | Tek bir kanalı ham okur. |

### Erişimciler

| Fonksiyon | Döndürür |
|---|---|
| `deger(kanal)` | İlgili kanalın son ham değeri (0..1023) |
| `dizi()` | İçteki `uint16_t[8]` dizisinin pointer'ı (eski koda uyumluluk) |
| `hata()` | Son hata değeri (-3500..+3500) — PID için ana giriş |
| `pozisyon()` | Son pozisyon (0..7000, merkez 3500) |
| `zeminTipi()` | `ZEMIN_BEYAZ` (1) veya `ZEMIN_SIYAH` (0) |

### Konfigürasyon

| Fonksiyon | Varsayılan | Açıklama |
|---|---|---|
| `zeminAyarla(tip)` | `ZEMIN_BEYAZ` | Zemini elle ayarla. |
| `otomatikZeminAyarla(aktif)` | `true` | Otomatik zemin tespiti aç/kapat. |
| `gurultuEsigiAyarla(esik)` | `250` | Bu değerin altındaki sensör çıktısı 0 sayılır. Düşürürseniz hassasiyet artar, gürültü artar; yükseltirseniz aksini yapar. |
| `zeminEsikleriAyarla(dusuk, yuksek)` | `200 / 800` | Otomatik zemin tespitinde dış sensörlerin (0 ve 7) baktığı eşikler. |

### 90° dönüş tespiti

| Fonksiyon | Açıklama |
|---|---|
| `sagaDonusVar(esikDusuk, esikYuksek)` | Sol taraf (0..3) boş, sağ taraf (6,7) çizgide ise `true`. Varsayılan eşikler: 400 / 600. |
| `solaDonusVar(esikDusuk, esikYuksek)` | Simetrik. |

---

## Hata değeri ne demek?

```
   sensör:   0    1    2    3    4    5    6    7
   pozisyon: 0  1000 2000 3000 4000 5000 6000 7000
   merkez = 3500
   hata   = pozisyon - 3500   ->   -3500 .. +3500
```

- `hata == 0`  → çizgi tam merkezde
- `hata < 0`   → çizgi sola kaydı, robotu **sola** çevir
- `hata > 0`   → çizgi sağa kaydı, robotu **sağa** çevir

PID:

```cpp
long motorSpeed = ((long)KP_FP * hata + (long)KD_FP * (hata - sonHata)) >> 8;
sonHata = hata;
```

---

## Zemin tipi

| Sabit | Değer | Anlamı |
|---|---|---|
| `ZEMIN_BEYAZ` | 1 | Beyaz zemin üzerinde **siyah** çizgi (klasik) |
| `ZEMIN_SIYAH` | 0 | Siyah zemin üzerinde **beyaz** çizgi |

Kütüphane, dış sensörlerin (`s0` ve `s7`) değerlerine bakarak hangi durumda olduğunu otomatik tespit eder. İsterseniz `otomatikZeminAyarla(false)` ile kapatabilirsiniz.

---

## Örnekler

| Örnek | Ne gösterir? |
|---|---|
| **TemelOkuma** | En küçük örnek — sensör değerlerini Seri Porta basar. |
| **CizgiIzleyenPID** | Tam çalışan fixed-point PID ile çizgi izleyici. |
| **DoksanDereceDonus** | 90° köşe tespiti ve manevra iskeleti. |
| **SeriPortIzleme** | Serial Plotter ile sensörleri canlı izlemek için. |

---

## Performans notları

- **ADC clock**: 1 MHz (prescaler 16). Bir dönüşüm 13 µs, 8 kanal ~104 µs.
- **Pozisyon hesabı**: `uint32_t` ağırlıklı toplam, division dahil ~50 µs.
- **Tam döngü**: ~150 µs → teorik **6500+ Hz** kontrol döngüsü.

Eğer ana `loop()` içinde de 31 kHz PWM, Timer2 customMillis vb. ayarları yaparsanız, pratikte 9000+ Hz'e ulaşmak mümkündür.

---

## Lisans

MIT — özgürce kullanın, değiştirin, dağıtın.

---

<a name="en"></a>

## 🇬🇧 ENGLISH

### CaYaLineFollowerSensor

**Fast**, **calibration-free**, and **dependency-free** Arduino library for 8-channel analog line-following sensor arrays.

> Uses raw ADC register access for ATmega328P (Arduino Nano / Uno). **No QTRSensors dependency**.

---

### Why this library?

| Feature | analogRead() | QTRSensors | **CaYaLineFollowerSensor** |
|---|---|---|---|
| 8-channel read time | ~880 µs | ~400 µs | **~104 µs** |
| Calibration needed? | — | Yes | **No** |
| Auto ground detection | — | No | **Yes** |
| Dependency | — | QTRSensors.h | **None** |
| Hardware | All | All | ATmega328P |

Instead of calibration, it uses a **noise threshold (virtual calibration)**: sensor output below the threshold is counted as 0, above it as clean signal. This means:

- Ready to use the moment you power it on,
- Much less affected by temperature and lighting changes,
- No lengthy calibration before the race starts.

---

### Hardware Requirements

- ATmega328P-based board: **Arduino Nano** or **Arduino Uno** (16 MHz).
- 8-channel analog line sensor (like Pololu QTR-8A).
- Sensor outputs connected to **A0 — A7** pins in order.

> Since the library writes directly to ADC registers, it does **not work** on non-ATmega328P boards (ESP32, STM32, RP2040, Mega2560, etc.).

---

### Installation

1. Copy this folder to your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\CaYaLineFollowerSensor\`
   - macOS: `~/Documents/Arduino/libraries/CaYaLineFollowerSensor/`
2. Restart the Arduino IDE.
3. Find examples under `File → Examples → CaYaLineFollowerSensor`.

---

### Quick Start

```cpp
#include <CaYaLineFollowerSensor.h>

CaYaLineFollowerSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();          // ADC initialization
}

void loop() {
  sensor.oku();            // Read 8 channels + ground + position + error
  long error = sensor.hata();   // -3500 .. +3500
  // Drive motors based on error ...
}
```

---

### API Reference

#### Setup

| Function | Description |
|---|---|
| `begin()` | Sets ADC prescaler and AVcc reference. Call once in `setup()`. |

#### Reading

| Function | Time | What it does |
|---|---|---|
| `oku()` | ~110 µs | Reads 8 channels, updates ground, calculates position and error. **Main usage.** |
| `okuHam()` | ~104 µs | Reads only 8 channels, no calculations. |
| `okuKenar()` | ~26 µs | Reads only channels 0, 1, 6, 7. Ideal for 90° turn loops. |
| `adcOku(kanal)` | ~13 µs | Reads a single channel raw. |

#### Accessors

| Function | Returns |
|---|---|
| `deger(kanal)` | Last raw value of the channel (0..1023) |
| `dizi()` | Pointer to internal `uint16_t[8]` array (for legacy code compatibility) |
| `hata()` | Last error value (-3500..+3500) — main PID input |
| `pozisyon()` | Last position (0..7000, center 3500) |
| `zeminTipi()` | `ZEMIN_BEYAZ` (1) or `ZEMIN_SIYAH` (0) |

#### Configuration

| Function | Default | Description |
|---|---|---|
| `zeminAyarla(tip)` | `ZEMIN_BEYAZ` | Set ground manually. |
| `otomatikZeminAyarla(aktif)` | `true` | Toggle auto ground detection. |
| `gurultuEsigiAyarla(esik)` | `250` | Sensor output below this is counted as 0. Lower = more sensitive but noisier; higher = opposite. |
| `zeminEsikleriAyarla(dusuk, yuksek)` | `200 / 800` | Thresholds for outer sensors (0 and 7) in auto ground detection. |

#### 90° Turn Detection

| Function | Description |
|---|---|
| `sagaDonusVar(esikDusuk, esikYuksek)` | Returns `true` if left side (0..3) is empty and right side (6,7) is on line. Default thresholds: 400 / 600. |
| `solaDonusVar(esikDusuk, esikYuksek)` | Symmetric. |

---

### What does the error value mean?

```
   sensor:   0    1    2    3    4    5    6    7
   position: 0  1000 2000 3000 4000 5000 6000 7000
   center = 3500
   error   = position - 3500   ->   -3500 .. +3500
```

- `error == 0`  → line is exactly centered
- `error < 0`   → line shifted left, turn robot **left**
- `error > 0`   → line shifted right, turn robot **right**

PID example:

```cpp
long motorSpeed = ((long)KP_FP * hata + (long)KD_FP * (hata - sonHata)) >> 8;
sonHata = hata;
```

---

### Ground Type

| Constant | Value | Meaning |
|---|---|---|
| `ZEMIN_BEYAZ` | 1 | **Black** line on white ground (classic) |
| `ZEMIN_SIYAH` | 0 | **White** line on black ground |

The library auto-detects which case by looking at outer sensors (s0 and s7). You can disable this with `otomatikZeminAyarla(false)`.

---

### Examples

| Example | What it shows |
|---|---|
| **TemelOkuma** | Minimal example — prints sensor values to Serial. |
| **CizgiIzleyenPID** | Full working line follower with fixed-point PID. |
| **DoksanDereceDonus** | 90° corner detection and maneuver skeleton. |
| **SeriPortIzleme** | Live sensor monitoring with Serial Plotter. |

---

### Performance Notes

- **ADC clock**: 1 MHz (prescaler 16). One conversion is 13 µs, 8 channels ~104 µs.
- **Position calculation**: `uint32_t` weighted sum with division included, ~50 µs.
- **Full loop**: ~150 µs → theoretical **6500+ Hz** control loop.

If you also set up 31 kHz PWM, Timer2 customMillis, etc. in your main `loop()`, you can reach 9000+ Hz in practice.

---

### License

MIT — use, modify, and distribute freely.
