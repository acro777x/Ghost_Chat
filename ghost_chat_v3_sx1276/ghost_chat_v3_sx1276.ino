/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║       GHOST CHAT V4 — Heltec WiFi LoRa 32 V2 (SX1276)       ║
 * ║          CORRECTED against official Heltec schematic         ║
 * ║   Source: resource.heltec.cn/download/WiFi_LoRa_32/V2/      ║
 * ║           WIFI_LoRa_32_V2(868-915).PDF                       ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  BUGS FIXED IN THIS VERSION:                                 ║
 * ║                                                              ║
 * ║  [FIX-1] SD: sdSPI.begin() must NOT receive CS as 4th arg   ║
 * ║           Passing CS there causes SPI driver to manage it,   ║
 * ║           which conflicts with SD library's own CS control.  ║
 * ║           Fix: sdSPI.begin(SCK, MISO, MOSI) — no CS arg.    ║
 * ║                                                              ║
 * ║  [FIX-2] Vext control is GPIO21, not GPIO17.                 ║
 * ║           Confirmed from schematic: GPIO21 → R13(1K) →       ║
 * ║           Q3(AO3401 P-MOSFET) → Vext rail.                   ║
 * ║           LOW = Vext ON, HIGH = Vext OFF (P-channel logic).  ║
 * ║           Must be driven LOW in setup() before SD init.      ║
 * ║                                                              ║
 * ║  [FIX-3] GPIO17 confirmed FREE — it is NOT Vext.             ║
 * ║           Original SD wiring 22/23/17/13 is correct and      ║
 * ║           does NOT conflict with LoRa (VSPI 5/19/27/18).     ║
 * ║                                                              ║
 * ║  [FIX-4] Vext must be enabled BEFORE SD init, OLED init,    ║
 * ║           and LoRa init — any peripheral on the Vext rail    ║
 * ║           needs the rail up first.                           ║
 * ║                                                              ║
 * ║  [FIX-5] LoRa.receive() called only once after init.        ║
 * ║           parsePacket() in loop() is the correct poll mode.  ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  PIN ASSIGNMENTS — verified from official schematic:         ║
 * ║                                                              ║
 * ║  LoRa SX1276 (VSPI bus — schematic-confirmed):              ║
 * ║    SCK   = GPIO5   (VSPICLK on schematic)                    ║
 * ║    MISO  = GPIO19  (VSPIQ on schematic)                      ║
 * ║    MOSI  = GPIO27  (routed to SX1276 MOSI on schematic)      ║
 * ║    SS/CS = GPIO18  (routed to SX1276 NSS on schematic)       ║
 * ║    RST   = GPIO14  (RST_1278 net on schematic)               ║
 * ║    DIO0  = GPIO26  (DIO0 net on schematic)                   ║
 * ║                                                              ║
 * ║  OLED SSD1306 (I2C — schematic-confirmed):                   ║
 * ║    SDA   = GPIO4   (SDA net on schematic)                    ║
 * ║    SCL   = GPIO15  (SCL net on schematic)                    ║
 * ║    RST   = GPIO16  (OLED_RST net on schematic)               ║
 * ║                                                              ║
 * ║  Board control (schematic-confirmed):                        ║
 * ║    Vext  = GPIO21  (Q3 gate via R13 — LOW=ON, HIGH=OFF)      ║
 * ║    LED   = GPIO25  (LED1 via R8/D4 on schematic)             ║
 * ║    BTN0  = GPIO0   (S2/BTN-0 on schematic)                   ║
 * ║                                                              ║
 * ║  SD Card (HSPI bus — free pins confirmed from schematic):    ║
 * ║    SCK   = GPIO22  (header pin, not claimed by any onboard)  ║
 * ║    MISO  = GPIO23  (header pin, not claimed by any onboard)  ║
 * ║    MOSI  = GPIO17  (header pin, not Vext — confirmed)        ║
 * ║    CS    = GPIO13  (header pin, not claimed by any onboard)  ║
 * ║                                                              ║
 * ║  PHYSICAL WIRING (unchanged from original — it was correct): ║
 * ║    SD VCC  → 3.3V pin on board                               ║
 * ║    SD GND  → GND                                             ║
 * ║    SD SCK  → GPIO22                                          ║
 * ║    SD MISO → GPIO23                                          ║
 * ║    SD MOSI → GPIO17                                          ║
 * ║    SD CS   → GPIO13                                          ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  LIBRARIES (Library Manager):                                ║
 * ║    - LoRa by Sandeep Mistry                                  ║
 * ║    - U8g2 by oliver                                          ║
 * ║    - SD (built-in Arduino/ESP32)                             ║
 * ║                                                              ║
 * ║  BOARD: Heltec WiFi LoRa 32(V2)                              ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <SD.h>
#include <DNSServer.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "index_html.h"

// ============================================================
// CONFIGURATION
// ============================================================
#define USE_SD_CARD 1

const char* WIFI_SSID   = "Ghost_Net";
const char* WIFI_PASS   = "ghost1234";
const char* DURESS_PASS = "pink";
const char* ADMIN_PASS  = "ghostadmin99";
const char* DOMAIN_NAME = "ghost.chat";
const char* NODE_ID     = "NODE-A";

// ============================================================
// FILE SYSTEM SELECTION
// ============================================================
#if USE_SD_CARD
  #define FS_SYS SD
#else
  #define FS_SYS SPIFFS
#endif

// ============================================================
// PIN DEFINITIONS
// All verified from: WIFI_LoRa_32_V2(868-915).PDF schematic
// ============================================================

// LoRa SX1276 — VSPI bus
// Schematic nets: VSPICLK(5), VSPIQ(19), MOSI→27, NSS→18, RST_1278→14, DIO0→26
#define LORA_SCK    5
#define LORA_MISO   19
#define LORA_MOSI   27
#define LORA_SS     18
#define LORA_RST    14
#define LORA_DIO0   26

// OLED SSD1306 — I2C
// Schematic nets: SDA(4), SCL(15), OLED_RST(16)
#define OLED_SDA    4
#define OLED_SCL    15
#define OLED_RST    16

// Board control
// Schematic: GPIO21 → R13(1K) → Q3(AO3401) gate → Vext rail
// P-channel MOSFET: LOW on gate = transistor ON = Vext ON
// HIGH on gate = transistor OFF = Vext OFF
#define VEXT_PIN    21    // [FIX-2] Confirmed Vext control pin from schematic
#define LED_PIN     25    // Schematic: LED1 via R8(330R) + D4
#define PANIC_BTN   0     // Schematic: S2/BTN-0

// SD Card — HSPI bus
// All four pins are free header pins confirmed from schematic.
// GPIO17 is NOT Vext (Vext is GPIO21). [FIX-3]
#define SD_SCK      22
#define SD_MISO     23
#define SD_MOSI     17
#define SD_CS       13

// ============================================================
// RADIO SETTINGS
// ============================================================
#define LORA_FREQ   866E6   // 866 MHz — India IN865 band
#define LORA_BW     125E3
#define LORA_SF     7
#define LORA_POWER  17      // dBm, max 20 on V2

// ============================================================
// LIMITS
// ============================================================
#define RATE_MAX         10
#define RATE_WINDOW      60000UL
#define MAX_IPS          16
#define MAX_SESSIONS     8
#define MAX_USERS        10
#define MAX_ACTIVE_ROOMS 5
#define MAX_ROOM_BYTES   40000
#define MIN_FREE_HEAP    28000

// ============================================================
// OBJECTS
// ============================================================
const byte DNS_PORT = 53;
DNSServer  dnsServer;
WebServer  server(80);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
  U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE
);

// SD uses its own HSPI instance — does NOT share with LoRa VSPI
SPIClass sdSPI(HSPI);

// ============================================================
// DYNAMIC ROOM STORAGE
// ============================================================
struct ActiveRoom {
  String        hash;
  String        content;
  String        pinnedMsg;
  String        typingUser;
  unsigned long typingExpiry;
  unsigned long lastAccess;
};
ActiveRoom activeRooms[MAX_ACTIVE_ROOMS];

bool          loraOK      = false;
unsigned long lastLoraRx  = 0;
int           loraRSSI    = 0;

// ============================================================
// ROOM HELPERS
// ============================================================
int getRoomIdx(const String& rh) {
  if (rh.length() == 0) return -1;

  int           oldestIdx  = 0;
  unsigned long oldestTime = 0xFFFFFFFF;

  for (int i = 0; i < MAX_ACTIVE_ROOMS; i++) {
    if (activeRooms[i].hash == rh) {
      activeRooms[i].lastAccess = millis();
      return i;
    }
    if (activeRooms[i].hash.length() == 0) {
      activeRooms[i].hash        = rh;
      activeRooms[i].lastAccess  = millis();
      activeRooms[i].pinnedMsg   = "";
      activeRooms[i].typingUser  = "";
      activeRooms[i].content     = "";
      String path = "/r_" + rh + ".txt";
      File f = FS_SYS.open(path.c_str(), FILE_READ);
      if (f) { activeRooms[i].content = f.readString(); f.close(); }
      return i;
    }
    if (activeRooms[i].lastAccess < oldestTime) {
      oldestTime = activeRooms[i].lastAccess;
      oldestIdx  = i;
    }
  }

  // Evict oldest — flush to disk first
  String evictPath = "/r_" + activeRooms[oldestIdx].hash + ".txt";
  File fw = FS_SYS.open(evictPath.c_str(), FILE_WRITE);
  if (fw) { fw.print(activeRooms[oldestIdx].content); fw.close(); }

  activeRooms[oldestIdx].hash        = rh;
  activeRooms[oldestIdx].lastAccess  = millis();
  activeRooms[oldestIdx].pinnedMsg   = "";
  activeRooms[oldestIdx].typingUser  = "";
  activeRooms[oldestIdx].content     = "";

  String loadPath = "/r_" + rh + ".txt";
  File fr = FS_SYS.open(loadPath.c_str(), FILE_READ);
  if (fr) { activeRooms[oldestIdx].content = fr.readString(); fr.close(); }
  return oldestIdx;
}

void trimRoomStr(String& chat) {
  if ((int)chat.length() > MAX_ROOM_BYTES) {
    chat = chat.substring(chat.length() - MAX_ROOM_BYTES);
    int first = chat.indexOf("|||");
    if (first > 0) chat = chat.substring(first + 3);
  }
}

void clearRoom(int idx) {
  if (idx < 0 || idx >= MAX_ACTIVE_ROOMS) return;
  String rh = activeRooms[idx].hash;
  if (rh.length() > 0) {
    activeRooms[idx].content    = "";
    activeRooms[idx].pinnedMsg  = "";
    activeRooms[idx].typingUser = "";
    String path = "/r_" + rh + ".txt";
    if (FS_SYS.exists(path.c_str())) FS_SYS.remove(path.c_str());
  }
}

void appendRoomMsg(const String& rh, const String& msg) {
  if (rh.length() == 0) return;

  for (int i = 0; i < MAX_ACTIVE_ROOMS; i++) {
    if (activeRooms[i].hash == rh) {
      activeRooms[i].content += msg;
      trimRoomStr(activeRooms[i].content);
      break;
    }
  }

  String path = "/r_" + rh + ".txt";
  File f = FS_SYS.open(path.c_str(), FILE_APPEND);
  if (f) {
    f.print(msg);
    if ((int)f.size() > MAX_ROOM_BYTES + 4000) {
      f.close();
      File fr = FS_SYS.open(path.c_str(), FILE_READ);
      String fc = fr ? fr.readString() : "";
      if (fr) fr.close();
      trimRoomStr(fc);
      File fw2 = FS_SYS.open(path.c_str(), FILE_WRITE);
      if (fw2) { fw2.print(fc); fw2.close(); }
      return;
    }
    f.close();
  } else {
    File fw2 = FS_SYS.open(path.c_str(), FILE_WRITE);
    if (fw2) { fw2.print(msg); fw2.close(); }
  }
}

// ============================================================
// RATE LIMITER
// ============================================================
struct RateEntry { uint32_t ip; unsigned long winStart; uint8_t count; };
RateEntry rateTable[MAX_IPS];
uint8_t   rateCount = 0;

bool checkRate(uint32_t ip) {
  unsigned long now = millis();
  for (int i = 0; i < rateCount; i++) {
    if (rateTable[i].ip == ip) {
      if (now - rateTable[i].winStart > RATE_WINDOW) {
        rateTable[i].winStart = now; rateTable[i].count = 1; return true;
      }
      if (rateTable[i].count >= RATE_MAX) return false;
      rateTable[i].count++;
      return true;
    }
  }
  if (rateCount < MAX_IPS) rateTable[rateCount++] = {ip, now, 1};
  return true;
}

// ============================================================
// SESSIONS
// ============================================================
struct Session {
  uint32_t      ip;
  char          tok[9];
  char          name[24];
  char          roomHash[17];
  unsigned long lastSeen;
};
Session sessions[MAX_SESSIONS];
uint8_t sessionCount = 0;

String genToken() {
  char t[9];
  snprintf(t, 9, "%08X", (uint32_t)esp_random());
  return String(t);
}

String getSessionRoom(uint32_t ip, const String& tok) {
  if (tok.length() == 0) return "";
  for (int i = 0; i < sessionCount; i++) {
    if (sessions[i].ip == ip && tok == String(sessions[i].tok)) {
      sessions[i].lastSeen = millis();
      return String(sessions[i].roomHash);
    }
  }
  return "";
}

String getSessionName(uint32_t ip, const String& tok) {
  for (int i = 0; i < sessionCount; i++) {
    if (sessions[i].ip == ip && tok == String(sessions[i].tok))
      return String(sessions[i].name);
  }
  return "";
}

String createSession(uint32_t ip, const String& name, const String& rhash) {
  String tok = genToken();
  for (int i = 0; i < sessionCount; i++) {
    if (sessions[i].ip == ip) {
      tok.toCharArray(sessions[i].tok, 9);
      name.substring(0, 23).toCharArray(sessions[i].name, 24);
      rhash.substring(0, 16).toCharArray(sessions[i].roomHash, 17);
      sessions[i].lastSeen = millis();
      return tok;
    }
  }
  if (sessionCount < MAX_SESSIONS) {
    Session s;
    s.ip = ip;
    tok.toCharArray(s.tok, 9);
    name.substring(0, 23).toCharArray(s.name, 24);
    rhash.substring(0, 16).toCharArray(s.roomHash, 17);
    s.lastSeen = millis();
    sessions[sessionCount++] = s;
  }
  return tok;
}

// ============================================================
// XSS SANITIZER
// ============================================================
String sanitize(const String& raw) {
  String out;
  out.reserve(raw.length() + 20);
  for (int i = 0; i < (int)raw.length() && i < 480; i++) {
    char c = raw[i];
    switch (c) {
      case '<':  out += "&lt;";   break;
      case '>':  out += "&gt;";   break;
      case '&':  out += "&amp;";  break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&#x27;"; break;
      default:   out += c;        break;
    }
  }
  return out;
}

// ============================================================
// ONLINE USER TRACKER
// ============================================================
struct UserEntry { char name[24]; unsigned long lastSeen; };
UserEntry onlineUsers[MAX_USERS];
int       userCount = 0;

void updateUser(const String& name) {
  for (int i = 0; i < userCount; i++) {
    if (String(onlineUsers[i].name) == name) {
      onlineUsers[i].lastSeen = millis(); return;
    }
  }
  if (userCount < MAX_USERS) {
    UserEntry u;
    name.substring(0, 23).toCharArray(u.name, 24);
    u.lastSeen = millis();
    onlineUsers[userCount++] = u;
  }
}

int onlineCount() {
  int c = 0;
  for (int i = 0; i < userCount; i++)
    if (millis() - onlineUsers[i].lastSeen < 30000) c++;
  return c;
}

// ============================================================
// WIPE
// ============================================================
void doWipe() {
  for (int i = 0; i < MAX_ACTIVE_ROOMS; i++) {
    activeRooms[i].hash        = "";
    activeRooms[i].content     = "";
    activeRooms[i].pinnedMsg   = "";
    activeRooms[i].typingUser  = "";
  }
  userCount    = 0;
  sessionCount = 0;
  rateCount    = 0;

#if USE_SD_CARD
  File root = SD.open("/");
  if (root) {
    File entry = root.openNextFile();
    while (entry) {
      String fname = "/" + String(entry.name());
      entry.close();
      if (fname.startsWith("/r_") || fname.endsWith(".jpg") || fname.endsWith(".jpeg")) {
        SD.remove(fname.c_str());
        Serial.println("[WIPE] Deleted: " + fname);
      }
      entry = root.openNextFile();
    }
    root.close();
  }
#else
  SPIFFS.format();
#endif

  Serial.println("[WIPE] Complete");
}

// ============================================================
// LORA
// ============================================================
void loraBroadcast(const String& rh, const String& msg) {
  if (!loraOK || rh.length() == 0) return;
  LoRa.idle();
  LoRa.beginPacket();
  LoRa.print("GC|" + String(NODE_ID) + "|" + rh + "|" + msg.substring(0, 200));
  LoRa.endPacket(false);
  LoRa.receive();   // [FIX-5] re-enter RX mode after TX
}

void loraReceive() {
  if (!loraOK) return;
  int ps = LoRa.parsePacket();   // [FIX-5] poll only, no LoRa.receive() here
  if (ps <= 0) return;

  String inc = "";
  while (LoRa.available()) inc += (char)LoRa.read();
  loraRSSI = LoRa.packetRssi();

  Serial.println("[LoRa RX] " + inc.substring(0, 60));

  if (!inc.startsWith("GC|")) return;
  int c1 = inc.indexOf('|', 3);   if (c1 < 0) return;
  int c2 = inc.indexOf('|', c1+1); if (c2 < 0) return;

  String srcNode = inc.substring(3, c1);
  String rh      = inc.substring(c1+1, c2);
  String payload = inc.substring(c2+1);

  if (srcNode == String(NODE_ID) || rh.length() == 0) return;

  appendRoomMsg(rh, payload + "|||");
  lastLoraRx = millis();
}

// ============================================================
// OLED
// ============================================================
unsigned long lastOled = 0;

void updateOLED() {
  if (millis() - lastOled < 3000) return;
  lastOled = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Ghost Chat V4");
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 26, ("IP:  " + WiFi.softAPIP().toString()).c_str());
  u8g2.drawStr(0, 38, ("Users: " + String(onlineCount()) + "/" + String(WiFi.softAPgetStationNum())).c_str());
  u8g2.drawStr(0, 50, ("RAM: " + String(ESP.getFreeHeap() / 1024) + "KB").c_str());
  String loraStr = loraOK ? ("LoRa OK " + String(loraRSSI) + "dBm") : "LoRa: FAILED";
  u8g2.drawStr(0, 62, loraStr.c_str());
  u8g2.sendBuffer();
}

// ============================================================
// SERVER ROUTES
// ============================================================
void setupRoutes() {

  auto serveMain = []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma",        "no-cache");
    server.sendHeader("Expires",       "0");
    server.send(200, "text/html", INDEX_HTML);
  };

  server.on("/", serveMain);

  // Captive portal — Android, iOS, Windows
  server.on("/generate_204",              []() { server.send(204, "text/plain", ""); });
  server.on("/gen_204",                   []() { server.send(204, "text/plain", ""); });
  server.on("/connecttest.txt",           []() { server.send(200, "text/plain", "Microsoft Connect Test"); });
  server.on("/hotspot-detect.html",       []() { server.send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>"); });
  server.on("/library/test/success.html", []() { server.send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>"); });
  server.on("/favicon.ico",               []() { server.send(204, "text/plain", ""); });

  // LOGIN
  server.on("/login", []() {
    String rhash = server.arg("room");
    String ptext = server.arg("pass");
    String name  = server.arg("name");

    if (ptext == DURESS_PASS) {
      doWipe();
      server.send(200, "text/plain", "WIPED");
      return;
    }
    if (ptext == ADMIN_PASS) {
      String safeName = name.length() > 0 ? name.substring(0, 20) : "ADMIN";
      String tok = createSession((uint32_t)server.client().remoteIP(), safeName, "ADMIN");
      server.send(200, "text/plain", "TOK:" + tok);
      return;
    }
    if (rhash.length() == 0) { server.send(400, "text/plain", "DENIED"); return; }

    String safeName = name.substring(0, 20);
    safeName.replace("|", ""); safeName.replace("&", ""); safeName.trim();
    if (safeName.length() == 0) safeName = "Ghost";

    updateUser(safeName);
    String tok = createSession((uint32_t)server.client().remoteIP(), safeName, rhash);
    server.send(200, "text/plain", "TOK:" + tok);
  });

  // SEND
  server.on("/send", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    String tok  = server.arg("tok");
    String rh   = getSessionRoom(ip, tok);

    if (rh == "")      { server.send(401, "text/plain", "AUTH");  return; }
    if (!checkRate(ip)) { server.send(429, "text/plain", "RATE"); return; }

    if (server.hasArg("msg")) {
      String m = server.arg("msg");
      if ((int)m.length() > 1500) m = m.substring(0, 1500);
      appendRoomMsg(rh, m + "|||");
      loraBroadcast(rh, m);
    }
    server.send(200, "text/plain", "OK");
  });

  // READ
  server.on("/read", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma",        "no-cache");
    server.sendHeader("Expires",       "0");

    uint32_t ip = (uint32_t)server.client().remoteIP();
    String tok  = server.arg("tok");
    String rh   = getSessionRoom(ip, tok);
    if (rh == "") { server.send(401, "text/plain", "AUTH"); return; }

    int idx = getRoomIdx(rh);
    if (idx < 0) { server.send(400, "text/plain", "ERR"); return; }

    String out = activeRooms[idx].content;
    if (activeRooms[idx].typingUser != "" && millis() < activeRooms[idx].typingExpiry)
      out += "###TYPE:" + activeRooms[idx].typingUser;
    out += "###ONLINE:" + String(onlineCount());
    server.send(200, "text/plain", out);
  });

  // TYPING
  server.on("/typing", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    String rh   = getSessionRoom(ip, server.arg("tok"));
    if (rh == "") { server.send(401, "text/plain", "AUTH"); return; }

    int idx = getRoomIdx(rh);
    if (idx >= 0 && server.hasArg("name")) {
      activeRooms[idx].typingUser   = server.arg("name") + "|" + rh;
      activeRooms[idx].typingExpiry = millis() + 3000;
      updateUser(server.arg("name"));
    }
    server.send(200, "text/plain", "OK");
  });

  // ONLINE
  server.on("/online", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    if (getSessionRoom(ip, server.arg("tok")) == "") { server.send(401, "text/plain", "AUTH"); return; }
    server.send(200, "text/plain", String(onlineCount()));
  });

  // STATUS
  server.on("/status", []() {
    String s  = "RAM:"    + String(ESP.getFreeHeap()/1024)       + "KB";
    s += " Up:"    + String(millis()/60000)                      + "min";
    s += " WiFi:"  + String(WiFi.softAPgetStationNum())          + "AP";
    s += " LoRa:"  + String(loraOK ? "OK" : "OFF");
    s += " Node:"  + String(NODE_ID);
    s += " FS:"    + String(USE_SD_CARD ? "SD" : "SPIFFS");
    server.send(200, "text/plain", s);
  });

  // RAM
  server.on("/ram", []() {
    server.send(200, "text/plain", String(ESP.getFreeHeap()/1024));
  });

  // LORA STATUS
  server.on("/lorastatus", []() {
    if (!loraOK) { server.send(200, "text/plain", "OFF"); return; }
    unsigned long ago = lastLoraRx > 0 ? (millis() - lastLoraRx)/1000 : 9999;
    server.send(200, "text/plain", "OK:" + String(loraRSSI) + ":" + String(ago));
  });

  // PIN
  server.on("/pin", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    String rh   = getSessionRoom(ip, server.arg("tok"));
    if (rh == "") { server.send(401, "text/plain", "AUTH"); return; }

    int idx = getRoomIdx(rh);
    if (idx >= 0 && server.hasArg("msg")) {
      activeRooms[idx].pinnedMsg = server.arg("msg");
      if (activeRooms[idx].pinnedMsg == "CLEAR") activeRooms[idx].pinnedMsg = "";
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/getpin", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    String rh   = getSessionRoom(ip, server.arg("tok"));
    if (rh == "") { server.send(401, "text/plain", "AUTH"); return; }

    int idx = getRoomIdx(rh);
    String pin = (idx >= 0 && activeRooms[idx].pinnedMsg.length() > 0)
                   ? activeRooms[idx].pinnedMsg : "NONE";
    server.send(200, "text/plain", pin);
  });

  // CLEAR
  server.on("/clear", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    String rh   = getSessionRoom(ip, server.arg("tok"));
    if (rh == "") { server.send(401, "text/plain", "AUTH"); return; }
    int idx = getRoomIdx(rh);
    if (idx >= 0) clearRoom(idx);
    server.send(200, "text/plain", "OK");
  });

  // ADMIN
  server.on("/admin", []() {
    if (server.arg("pass") != ADMIN_PASS) { server.send(403, "text/plain", "DENIED"); return; }
    String resp = "{\"node\":\"" + String(NODE_ID) + "\",";
    resp += "\"ram_kb\":"       + String(ESP.getFreeHeap()/1024) + ",";
    resp += "\"uptime_min\":"   + String(millis()/60000)         + ",";
    resp += "\"sessions\":"     + String(sessionCount)           + ",";
    resp += "\"online\":"       + String(onlineCount())          + ",";
    resp += "\"lora_ok\":"      + String(loraOK ? "true" : "false") + "}";
    server.send(200, "application/json", resp);
  });

  server.on("/admin_wipe", HTTP_POST, []() {
    if (server.arg("pass") != ADMIN_PASS) { server.send(403, "text/plain", "DENIED"); return; }
    doWipe();
    server.send(200, "text/plain", "WIPED");
  });

  // DELETE FILE
  server.on("/delete_file", []() {
    uint32_t ip = (uint32_t)server.client().remoteIP();
    if (getSessionRoom(ip, server.arg("tok")) == "") { server.send(401, "text/plain", "AUTH"); return; }
    if (server.hasArg("name")) {
      String fn = server.arg("name");
      fn.replace("..", ""); fn.replace("/", ""); fn.trim();
      String p = "/" + fn;
      if (FS_SYS.exists(p.c_str())) FS_SYS.remove(p.c_str());
    }
    server.send(200, "text/plain", "OK");
  });

  // UPLOAD — correct open/write/close lifecycle
  server.on("/upload", HTTP_POST,
    []() {
      uint32_t ip = (uint32_t)server.client().remoteIP();
      if (getSessionRoom(ip, server.arg("tok")) == "") {
        server.send(401, "text/plain", "AUTH"); return;
      }
      server.send(200, "text/plain", "OK");
    },
    []() {
      HTTPUpload& u = server.upload();
      static File uf;

      if (u.status == UPLOAD_FILE_START) {
        String fn = u.filename;
        fn.replace("/", ""); fn.replace("..", ""); fn.trim();
        String p = "/" + fn;
        if (FS_SYS.exists(p.c_str())) FS_SYS.remove(p.c_str());
        uf = FS_SYS.open(p.c_str(), FILE_WRITE);   // open once
      } else if (u.status == UPLOAD_FILE_WRITE) {
        if (uf) uf.write(u.buf, u.currentSize);     // append without reopening
      } else if (u.status == UPLOAD_FILE_END) {
        if (uf) uf.close();                          // close only when done
      }
    }
  );

  // DOWNLOAD
  server.on("/download", []() {
    if (server.hasArg("name")) {
      String fn = server.arg("name");
      fn.replace("/", ""); fn.replace("..", ""); fn.trim();
      String p = "/" + fn;
      if (FS_SYS.exists(p.c_str())) {
        File f = FS_SYS.open(p.c_str(), FILE_READ);
        server.streamFile(f, "image/jpeg");
        f.close();
      } else {
        server.send(404, "text/plain", "Not found");
      }
    }
  });

  // 404 — redirect to chat
  server.onNotFound([]() {
    server.sendHeader("Location", String("http://") + DOMAIN_NAME, true);
    server.send(302, "text/plain", "");
  });
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[Ghost Chat V4] Starting...");

  // ── [FIX-2] Enable Vext rail FIRST ──────────────────────────
  // Schematic: GPIO21 → R13(1K) → Q3(AO3401 P-MOSFET) → Vext
  // P-channel MOSFET: gate LOW = ON = Vext powered
  // Must be done before SD init, OLED init, or any Vext-powered peripheral
  pinMode(VEXT_PIN, OUTPUT);
  digitalWrite(VEXT_PIN, LOW);   // LOW = Vext ON (schematic-confirmed)
  delay(100);                    // Wait for Vext rail to stabilize
  Serial.println("[Vext] GPIO21 LOW — Vext rail ON");

  // ── LED ─────────────────────────────────────────────────────
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ── Panic button ────────────────────────────────────────────
  pinMode(PANIC_BTN, INPUT_PULLUP);

  // ── OLED — GPIO16 RST must toggle LOW → HIGH ────────────────
  // Schematic: GPIO16 = OLED_RST net
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);  delay(20);
  digitalWrite(OLED_RST, HIGH); delay(20);

  Wire.begin(OLED_SDA, OLED_SCL);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 22, "Ghost Chat V4");
  u8g2.drawStr(20, 38, "Starting...");
  u8g2.sendBuffer();

  // ── LoRa SX1276 on VSPI (GPIO 5/19/27/18) ───────────────────
  // Schematic-confirmed: SCK=5, MISO=19, MOSI=27, NSS=18, RST=14, DIO0=26
  Serial.print("[LoRa] Initializing on VSPI (5/19/27/18)... ");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (LoRa.begin(LORA_FREQ)) {
    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(LORA_POWER);
    LoRa.setSyncWord(0xAB);
    LoRa.receive();   // [FIX-5] enter RX mode once after init
    loraOK = true;
    Serial.println("OK");
  } else {
    Serial.println("FAILED — check antenna and wiring");
  }

  // ── SD Card on HSPI (GPIO 22/23/17/13) ──────────────────────
  // Schematic: all four pins are free header pins.
  // GPIO17 is NOT Vext (Vext = GPIO21). [FIX-3]
  //
  // [FIX-1]: sdSPI.begin() must NOT receive CS as 4th argument.
  // The 4th arg tells the SPI driver to manage SS automatically,
  // which conflicts with the SD library doing the same thing.
  // Solution: call sdSPI.begin() with only SCK/MISO/MOSI,
  // then pass CS only to SD.begin().
#if USE_SD_CARD
  Serial.print("[SD] Initializing on HSPI (SCK=22,MISO=23,MOSI=17,CS=13)... ");
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI);  // [FIX-1] NO CS arg here
  delay(10);

  if (!SD.begin(SD_CS, sdSPI)) {           // CS only here, given to SD lib
    Serial.println("FAILED");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 22, "SD FAILED!");
    u8g2.drawStr(0, 38, "Check SD module");
    u8g2.sendBuffer();
    delay(3000);
  } else {
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.println("OK — " + String((uint32_t)cardSize) + " MB");
  }
#else
  Serial.print("[SPIFFS] Mounting... ");
  if (!SPIFFS.begin(true)) { Serial.println("FAILED"); }
  else                       { Serial.println("OK");    }
#endif

  // ── WiFi AP ─────────────────────────────────────────────────
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  delay(100);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  Serial.println("[WiFi] SSID: " + String(WIFI_SSID) + " | IP: " + WiFi.softAPIP().toString());

  // ── Web Server ───────────────────────────────────────────────
  setupRoutes();
  server.begin();
  Serial.println("[HTTP] Server started — http://ghost.chat");

  // ── Ready screen ─────────────────────────────────────────────
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(5,  14, "Ghost Chat V4");
  u8g2.drawLine(0, 17, 128, 17);
  u8g2.drawStr(0,  30, ("IP: " + WiFi.softAPIP().toString()).c_str());
  u8g2.drawStr(0,  42, ("WiFi: " + String(WIFI_SSID)).c_str());
  u8g2.drawStr(0,  54, ("LoRa: " + String(loraOK ? "READY" : "FAILED")).c_str());
  u8g2.drawStr(0,  62, ("FS: " + String(USE_SD_CARD ? "SD Card" : "SPIFFS")).c_str());
  u8g2.sendBuffer();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  updateOLED();
  loraReceive();   // non-blocking poll — parsePacket() only

  // RAM protection — trim oldest room if heap is low
  if (ESP.getFreeHeap() < MIN_FREE_HEAP) {
    Serial.println("[WARN] Low RAM — trimming rooms");
    for (int i = 0; i < MAX_ACTIVE_ROOMS; i++) {
      if (activeRooms[i].hash.length() > 0)
        trimRoomStr(activeRooms[i].content);
    }
  }

  // Panic button — hold 3 seconds to wipe everything
  if (digitalRead(PANIC_BTN) == LOW) {
    delay(50);
    if (digitalRead(PANIC_BTN) == LOW) {
      unsigned long t = millis();
      while (digitalRead(PANIC_BTN) == LOW) {
        int s   = (millis() - t) / 1000;
        int rem = max(0, 3 - s);
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(15, 22, "HOLD TO WIPE");
        u8g2.drawStr(30, 40, (String(rem) + " sec...").c_str());
        u8g2.sendBuffer();

        if (millis() - t > 3000) {
          doWipe();
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_6x10_tf);
          u8g2.drawStr(20, 32, "WIPED CLEAN");
          u8g2.sendBuffer();
          delay(2000);
          break;
        }
      }
    }
  }

  delay(2);
}
