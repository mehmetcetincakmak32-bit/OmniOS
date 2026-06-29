# OmniOS API Spesifikasyonu

> OmniOS, uygulama gelistiricilerin platformdan bagimsiz uygulamalar yazabilmesi icin bir API sunar.
> Bu dokuman, OmniOS API'sinin detayli spesifikasyonudur.

---

## 1. Genel API Yapisi

OmniOS API'si, hem iOS hem Android uygulamalarinin calismasini saglayan bir **soyutlama katmani**dir.

### 1.1 API Seviyeleri

| Seviye | Versiyon | Aciklama |
|--------|----------|----------|
| OmniOS Core API | v1.0 | Cift mod yonetimi, process, bellek |
| Android Runtime API | v4.0 (API 35) | Android uygulamalari icin ART emulasyonu |
| iOS Runtime API | v3.2 (API 18) | iOS uygulamalari icin UIKit emulasyonu |

---

## 2. Core API

### 2.1 System API

#### `system.getInfo()`
Sistem hakkinda genel bilgi dondurur.

**Response:**
```json
{
  "state": "IDLE | RUNNING | SLEEPING | SHUTDOWN",
  "mode": "normal | flow",
  "activeProcesses": 3,
  "totalApps": 18,
  "uptime": 3600,
  "androidRuntime": "ready",
  "iosRuntime": "ready"
}
```

#### `system.shutdown()`
OmniOS'u kapatir.

---

### 2.2 Mode API

#### `mode.get()`
Mevcut modu dondurur.
- **Response:** `"normal"` veya `"flow"`

#### `mode.set(mode: string)`
Modu belirtilen degere ayarlar.
- **Parametre:** `"normal"` | `"flow"`
- **Response:** `true` (basarili) / `false` (basarisiz)

#### `mode.toggle()`
Modu degistirir (normal -> flow, flow -> normal).
- **Response:** Yeni mod adi

#### `mode.getState()`
Moda ait durum bilgisi.
```json
{
  "mode": "flow",
  "lastActivity": "swipe_up",
  "history": ["swipe_up", "double_tap"]
}
```

---

### 2.3 Process API

#### `process.create(name: string, platform: string)`
Yeni bir process olusturur.
- **Parametreler:** `name`: Uygulama adi, `platform`: "Android" | "iOS" | "OmniOS"
- **Response:** Process bilgisi
```json
{
  "pid": "a1b2c3d4",
  "name": "Chrome",
  "platform": "Android",
  "state": "RUNNING",
  "uptime": 12.5
}
```

#### `process.kill(pid: string)`
Process'i sonlandirir.
- **Response:** `true` / `false`

#### `process.suspend(pid: string)`
Process'i duraklatir.
- **Response:** `true` / `false`

#### `process.list()`
Aktif process listesini dondurur.
- **Response:** Process dizisi

#### `process.getStats()`
Process istatistiklerini dondurur.
```json
{
  "totalCreated": 42,
  "active": 3,
  "terminated": 38,
  "suspended": 1,
  "crashed": 0
}
```

---

### 2.4 Gesture API

#### `gesture.recognize(name: string)`
Jest adini tanir.
- **Parametre:** Jest adi (`"swipe_up"`, `"double_tap"`, vb.)
- **Response:** Jest turu veya `null`

#### `gesture.execute(gesture: string, mode?: string)`
Jest'i calistirir.
- **Response:** Aksiyon adi
```json
{
  "gesture": "swipe_up",
  "action": "open_app_menu",
  "mode": "flow"
}
```

#### `gesture.getHints(mode?: string)`
Mevcut moddaki jest ipuclarini dondurur.
```json
[
  {"gesture": "swipe_up", "action": "open_app_menu", "description": "Uygulama menusu"},
  {"gesture": "swipe_right", "action": "open_recent_apps", "description": "Son uygulamalar"}
]
```

---

### 2.5 App API

#### `app.list(platform?: string)`
Uygulama listesini dondurur.
- **Parametre:** `"all" | "android" | "ios"` (default: "all")
- **Response:**
```json
[
  {"name": "Chrome", "icon": "🌐", "category": "internet", "platform": "android"},
  {"name": "Safari", "icon": "🌐", "category": "internet", "platform": "ios"}
]
```

#### `app.launch(name: string)`
Uygulama baslatir.
- **Response:** `true` / `false`

#### `app.kill(name: string)`
Uygulamayi durdurur.
- **Response:** `true` / `false`

---

### 2.6 Memory API

#### `memory.allocate(pid: string, size: number, label?: string)`
Bellek ayirir.
- **Response:** `true` / `false`

#### `memory.free(pid: string)`
Bellegi serbest birakir.
- **Response:** `true` / `false`

#### `memory.getReport()`
Bellek raporu dondurur.
```json
{
  "totalMb": 4096,
  "usedMb": 1024,
  "availableMb": 3072,
  "usagePercent": 25.0,
  "status": "normal"
}
```

---

## 3. Uyumluluk API'si

### 3.1 Android Runtime API

#### `android.canRun(app)`
Uygulamanin Android Runtime'da calisip calisamayacagini kontrol eder.

#### `android.launch(app)`
Uygulamayi ART emulasyonu ile baslatir.

#### `android.getInfo()`
```json
{
  "name": "Android Runtime (ART) Emulator",
  "apiLevel": 35,
  "status": "ready"
}
```

### 3.2 iOS Runtime API

#### `ios.canRun(app)`
Uygulamanin iOS Runtime'da calisip calisamayacagini kontrol eder.

#### `ios.launch(app)`
Uygulamayi UIKit emulasyonu ile baslatir.

#### `ios.getInfo()`
```json
{
  "name": "iOS Runtime (UIKit) Emulator",
  "apiLevel": 18,
  "status": "ready"
}
```

---

## 4. Event Sistemi

OmniOS, olay tabanli bir mimari kullanir.

### 4.1 Event List

| Event | Aciklama | Data |
|-------|----------|------|
| `boot` | Sistem baslangici | Durum mesaji |
| `launch` | Uygulama baslatma | Uygulama adi |
| `kill` | Uygulama sonlandirma | Uygulama adi |
| `mode` | Mod degisimi | Yeni mod adi |
| `gesture` | Jest algilama | Jest adi |
| `error` | Hata durumu | Hata mesaji |
| `shutdown` | Kapanis | Kapanis mesaji |

### 4.2 Kullanim

```python
engine.on("launch", lambda data: print(data))
engine.on("mode", on_mode_change)
engine.emit("gesture", "swipe_up")
```

---

## 5. Plugin API

### 5.1 Plugin Arayuzu

```python
class OmniOSPlugin:
    name = "plugin_name"
    version = "1.0.0"
    description = ""
    author = ""

    def on_load(self): pass
    def on_enable(self): pass
    def on_disable(self): pass
    def on_unload(self): pass
```

### 5.2 Plugin Yonetimi

| Metod | Aciklama |
|-------|----------|
| `pluginManager.discover(directory)` | Pluginleri kesfeder |
| `pluginManager.enable(name)` | Plugin'i aktif eder |
| `pluginManager.disable(name)` | Plugin'i devre disi birakir |
| `pluginManager.getAll()` | Tum pluginleri listeler |

---

## 6. Hata Kodlari

| Kod | Aciklama |
|-----|----------|
| `E_SUCCESS` | Basarili |
| `E_NOT_FOUND` | Kaynak bulunamadi |
| `E_ALREADY_RUNNING` | Uygulama zaten calisiyor |
| `E_MAX_PROCESSES` | Maksimum process sinirina ulasildi |
| `E_MEMORY_FULL` | Bellek dolu |
| `E_PLATFORM_UNSUPPORTED` | Platform desteklenmiyor |
| `E_INVALID_GESTURE` | Gecersiz jest |
| `E_PLUGIN_ERROR` | Plugin hatasi |

---

*Bu spesifikasyon OmniOS v1.0 icindir ve proje gelistikce guncellenecektir.*
