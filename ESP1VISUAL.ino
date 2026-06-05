#include <WiFi.h>
#include <WebSocketsServer.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include <U8g2lib.h>

#include <SPI.h>
#include <Wire.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "SERVOCUNA";
const char* password = "12345678";

// =====================================================
// WEBSOCKET
// =====================================================

WebSocketsServer webSocket = WebSocketsServer(81);

// =====================================================
// COLOR DISPLAY
// =====================================================

#define COLOR_DISPLAY 0xFD20

// =====================================================
// TFT SPI
// =====================================================

#define TFT_SCL 18
#define TFT_SDA 23

// =====================================================
// TFT1
// =====================================================

#define TFT1_CS   19
#define TFT1_DC    4
#define TFT1_RST   5

// =====================================================
// TFT2
// =====================================================

#define TFT2_CS   13
#define TFT2_DC   32
#define TFT2_RST  25

Adafruit_ST7789 tft1(
  TFT1_CS,
  TFT1_DC,
  TFT1_RST
);

Adafruit_ST7789 tft2(
  TFT2_CS,
  TFT2_DC,
  TFT2_RST
);

// =====================================================
// OLED BARRAS
// =====================================================

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C oledBarra(
  U8G2_R0,
  U8X8_PIN_NONE,
  27,
  26
);

// =====================================================
// OLED SETPOINT
// =====================================================

U8G2_SSD1306_128X64_NONAME_F_SW_I2C oledSet(
  U8G2_R0,
  22,
  21,
  U8X8_PIN_NONE
);

// =====================================================
// VARIABLES
// =====================================================

String modo = "precalentamiento";

int potencia = 30;

float babyTemp = 36.0;

float tempObjetivo = 36.0;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  Serial2.begin(
    115200,
    SERIAL_8N1,
    16,
    17
  );

  // WIFI

  WiFi.softAP(ssid,password);

  // WEBSOCKET

  webSocket.begin();

  webSocket.onEvent(webSocketEvent);

  // SPI

  SPI.begin(TFT_SCL,-1,TFT_SDA);

  // TFT1

  tft1.init(240,320);

  tft1.setRotation(3);

  tft1.fillScreen(ST77XX_BLACK);

  // TFT2

  tft2.init(240,320);

  tft2.setRotation(1);

  tft2.fillScreen(ST77XX_BLACK);

  // OLEDS

  oledBarra.begin();

  oledSet.begin();

  delay(1000);

  // AUTOTEST

  mostrarAutotest1();

  delay(2000);

  mostrarAutotest2();

  delay(5000);

  mostrarPrewarm();

  Serial.println(WiFi.softAPIP());
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  webSocket.loop();

  recibirUART();

  delay(5);
}

// =====================================================
// WEBSOCKET
// =====================================================

void webSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t * payload,
  size_t length
){

  if(type == WStype_TEXT){

    String msg =
    String((char*)payload);

    msg.trim();

    Serial2.println(msg);
  }
}

// =====================================================
// UART
// =====================================================

void recibirUART() {

  while(Serial2.available()) {

    String msg =
    Serial2.readStringUntil('\n');

    msg.trim();

    Serial.println(msg);

    webSocket.broadcastTXT(msg);

    // MODOS

    if(msg == "precalentamiento") {

      modo = "precalentamiento";

      mostrarPrewarm();
    }

    if(msg == "manual") {

      modo = "manual";

      mostrarManual();
    }

    if(msg == "bebe") {

      modo = "bebe";

      mostrarBaby();
    }

    // POWER

    if(msg.startsWith("POWER:")) {

      potencia =
      msg.substring(6).toInt();

      if(modo == "manual") {

        actualizarBarra(potencia);
      }
    }

    // TEMP

    if(msg.startsWith("BABYTEMP:")) {

      babyTemp =
      msg.substring(9).toFloat();

      if(modo == "bebe") {

        mostrarTempBebe();
      }
    }

    // SET TEMP

    if(msg.startsWith("SETTEMP:")) {

      tempObjetivo =
      msg.substring(8).toFloat();

      if(modo == "bebe") {

        mostrarSetTemp();
      }
    }
  }
}

// =====================================================
// AUTOTEST 1
// =====================================================

void mostrarAutotest1() {

  // TFT1

  tft1.fillScreen(ST77XX_BLACK);

  tft1.setTextColor(COLOR_DISPLAY);

  tft1.setTextSize(8);

  tft1.setCursor(10,95);

  tft1.print("88:88");

  // TFT2

  tft2.fillScreen(ST77XX_BLACK);

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(25,95);

  tft2.print("8.8.8");

  // OLED BARRAS

  actualizarBarra(100);

  // OLED SET

  oledSet.clearBuffer();

  oledSet.setFont(
    u8g2_font_logisoso32_tn
  );

  oledSet.setCursor(5,45);

  oledSet.print("8.8.8");

  oledSet.sendBuffer();
}

// =====================================================
// AUTOTEST 2
// =====================================================

void mostrarAutotest2() {

  tft1.fillScreen(ST77XX_BLACK);

  tft1.setTextColor(COLOR_DISPLAY);

  tft1.setTextSize(8);

  tft1.setCursor(30,95);

  tft1.print("----");

  tft2.fillScreen(ST77XX_BLACK);

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(40,95);

  tft2.print("---");

  actualizarBarra(0);

  oledSet.clearBuffer();

  oledSet.setFont(
    u8g2_font_logisoso32_tn
  );

  oledSet.setCursor(5,45);

  oledSet.print("---");

  oledSet.sendBuffer();
}

// =====================================================
// PREWARM
// =====================================================

void mostrarPrewarm() {

  tft1.fillScreen(ST77XX_BLACK);

  tft1.setTextColor(COLOR_DISPLAY);

  tft1.setTextSize(8);

  tft1.setCursor(20,95);

  tft1.print("11:42");

  tft2.fillScreen(ST77XX_BLACK);

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(25,95);

  tft2.print("--.-");

  actualizarBarra(30);

  oledSet.clearBuffer();

  oledSet.setFont(
    u8g2_font_logisoso32_tn
  );

  oledSet.setCursor(5,45);

  oledSet.print("--.-");

  oledSet.sendBuffer();
}

// =====================================================
// MANUAL
// =====================================================

void mostrarManual() {

  tft1.fillScreen(ST77XX_BLACK);

  tft1.setTextColor(COLOR_DISPLAY);

  tft1.setTextSize(8);

  tft1.setCursor(20,95);

  tft1.print("11:42");

  tft2.fillScreen(ST77XX_BLACK);

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(25,95);

  tft2.print("--.-");

  actualizarBarra(potencia);

  oledSet.clearBuffer();

  oledSet.setFont(
    u8g2_font_logisoso32_tn
  );

  oledSet.setCursor(5,45);

  oledSet.print("--.-");

  oledSet.sendBuffer();
}

// =====================================================
// BABY
// =====================================================

void mostrarBaby() {

  // TFT1

  tft1.fillScreen(ST77XX_BLACK);

  tft1.setTextColor(COLOR_DISPLAY);

  tft1.setTextSize(8);

  tft1.setCursor(20,95);

  tft1.print("11:42");

  // TFT2 SIN TEMP TODAVIA

  tft2.fillScreen(ST77XX_BLACK);

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(25,95);

  tft2.print("--.-");

  // OLED BARRAS OFF

  actualizarBarra(0);

  // OLED SETPOINT

  mostrarSetTemp();
}

// =====================================================
// TFT2 TEMP
// =====================================================

void mostrarTempBebe() {

  tft2.fillRect(
    0,
    80,
    240,
    120,
    ST77XX_BLACK
  );

  tft2.setTextColor(COLOR_DISPLAY);

  tft2.setTextSize(8);

  tft2.setCursor(25,95);

  tft2.print(babyTemp,1);
}

// =====================================================
// OLED SET TEMP
// =====================================================

void mostrarSetTemp() {

  oledSet.clearBuffer();

  oledSet.setFont(
    u8g2_font_logisoso32_tn
  );

  oledSet.setCursor(5,45);

  oledSet.print(tempObjetivo,1);

  oledSet.sendBuffer();
}

// =====================================================
// OLED BARRAS
// =====================================================

void actualizarBarra(int porcentaje) {

  oledBarra.clearBuffer();

  int barras =
  map(porcentaje,0,100,0,10);

  for(int i=0;i<10;i++) {

    int x = 6 + i*12;

    if(i < barras) {

      oledBarra.drawBox(x,6,8,20);
    }

    else {

      oledBarra.drawFrame(x,6,8,20);
    }
  }

  oledBarra.sendBuffer();
}