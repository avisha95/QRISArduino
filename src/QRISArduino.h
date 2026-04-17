#ifndef QRISARDUINO_H
#define QRISARDUINO_H

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PNGdec.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

#define MAYAR_BASE_URL "https://api.mayar.id/hl/v1"

struct QRISResponse {
  int     status_code;
  String  message;
  bool    success;
  String  url;
  long    amount;
  String  reference_id;
  long    balance;
  long    balance_pending;
  long    balance_active;
  String  rawJson;
};

class Mayar {
public:
  Mayar(const String& apiKey);
  void setTimeout(int seconds);
  void setInsecure(bool insecure = true);
  void setAutoReconnect(bool enable);
  void debug(bool enable);
  bool begin(const char* ssid, const char* password, int timeoutSec = 30);
  bool isConnected();
  bool reconnect();
  QRISResponse balance();
  QRISResponse createQris(long amount);
  bool waitForPayment(long initialBalance, unsigned long pollIntervalMs, unsigned long timeoutMs,
                      std::function<void(long currentBalance, unsigned long elapsed)> callback = nullptr);
  static String resizeImageUrl(const String& url, int newSize);

private:
  String _apiKey;
  int    _timeout;
  bool   _insecure;
  bool   _autoReconnect;
  bool   _debug;
  String _ssid;
  String _password;
  bool _ensureWiFi();
  QRISResponse _request(const String& method, const String& endpoint, const String& payload = "");
  QRISResponse _parseResponse(int httpCode, const String& body);
};

class TFTKeypad {
public:
  TFTKeypad(Adafruit_ILI9341& display, Adafruit_FT6206& touch);
  void begin();
  void draw();
  void setValue(const String& val);
  String getValue();
  void reset();
  void updateDisplay();
  void onDigit(std::function<void(char digit)> callback);
  void onDel(std::function<void()> callback);
  void onOk(std::function<void()> callback);
  void process();

private:
  Adafruit_ILI9341* _tft;
  Adafruit_FT6206*  _ctp;
  String _value;
  unsigned long _lastTouch;
  bool _wasReleased;
  std::function<void(char)> _digitCb;
  std::function<void()> _delCb;
  std::function<void()> _okCb;
  static const int SCREEN_W, SCREEN_H, DISP_H, KEY_PAD, KEY_GAP, KEY_W, KEY_H, KEY_START_X, KEY_START_Y, DEBOUNCE_MS;
  static const char KEY_LABEL[4][3];
  bool hit(int px, int py, int row, int col);
  void drawKey(int row, int col, bool pressed);
};

class TFTImageDisplay {
public:
  TFTImageDisplay(Adafruit_GFX& display);

  // Show QR image. Offset is top-left (auto-centered horizontally, y=0 by default).
  // Call setOffset() before show() to override position.
  bool show(const String& url, int targetWidth = 0, int targetHeight = 0, int timeoutSec = 10,
            std::function<void(int percent)> progressCb = nullptr);

  // Override render position. Call resetOffset() to restore auto-center.
  void setOffset(int x, int y);
  void resetOffset();

private:
  Adafruit_GFX* _display;
  int  _targetW  = 0;
  int  _targetH  = 0;
  int  _origW    = 0;
  int  _origH    = 0;
  int  _offsetX  = 0;
  int  _offsetY  = 0;
  bool _offsetOverride = false;
  PNG  _png;
  static TFTImageDisplay* _instance;
  static int _pngDrawCallback(PNGDRAW* pDraw);
  int _internalDraw(PNGDRAW* pDraw);
  bool downloadAndDecode(const String& url, int timeoutSec, std::function<void(int percent)> progressCb);
};

#endif
