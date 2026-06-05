#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

// =====================================================
// MODOS
// =====================================================

enum Modo {
  PREWARM,
  MANUAL,
  BABY
};

Modo modoActual = PREWARM;

// =====================================================
// VARIABLES
// =====================================================

int potencia = 30;

float tempObjetivo = 36.0;

float tempBebe = 36.0;

float ultimaTemp = -1;

// =====================================================
// DEBOUNCE
// =====================================================

unsigned long ultimoBoton = 0;

const int debounceTiempo = 250;

// =====================================================
// PWM
// =====================================================

#define PWM_LED 4

// =====================================================
// BOTONES
// =====================================================

#define BTN_UP        26
#define BTN_DOWN      14
#define BTN_MODE      32
#define BTN_APGAR     13
#define BTN_TIMER     12
#define BTN_LOCK      27
#define BTN_CANCEL    25
#define BTN_SILENCE   33

// =====================================================
// BUZZER
// =====================================================

#define BUZZER 15

// =====================================================
// 74HC595
// =====================================================

#define DATA_PIN   23
#define CLOCK_PIN  18
#define LATCH_PIN   5

uint16_t estado595 = 0;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // UART

  Serial2.begin(
    115200,
    SERIAL_8N1,
    16,
    17
  );

  // I2C

  Wire.begin(21,22);

  // =================================================
  // SENSOR
  // =================================================

  if(lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {

    Serial.println("BH1750 OK");
  }

  else {

    Serial.println("ERROR BH1750");
  }

  // =================================================
  // BOTONES
  // =================================================

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);

  pinMode(BTN_APGAR, INPUT_PULLUP);
  pinMode(BTN_TIMER, INPUT_PULLUP);
  pinMode(BTN_LOCK, INPUT_PULLUP);
  pinMode(BTN_CANCEL, INPUT_PULLUP);
  pinMode(BTN_SILENCE, INPUT_PULLUP);

  // =================================================
  // BUZZER
  // =================================================

  pinMode(BUZZER, OUTPUT);

  // =================================================
  // 74HC595
  // =================================================

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // =================================================
  // PWM
  // =================================================

  ledcAttach(PWM_LED,5000,8);

  delay(1000);

  // =================================================
  // AUTOTEST
  // =================================================

  autotest();

  // =================================================
  // MODO INICIAL
  // =================================================

  actualizarModo();

  enviarModoUART();

  Serial.println("ESP2 READY");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  leerBotones();

  actualizarSensor();

  controlarPotencia();

  delay(10);
}

// =====================================================
// AUTOTEST
// =====================================================

void autotest() {

  estado595 = 0xFFFF;

  actualizar595();

  tone(BUZZER,2000);

  delay(2000);

  noTone(BUZZER);

  estado595 = 0x0000;

  actualizar595();

  delay(5000);
}

// =====================================================
// BOTONES
// =====================================================

void leerBotones() {

  static bool lastMode = HIGH;
  static bool lastUp = HIGH;
  static bool lastDown = HIGH;

  static bool lastApgar = HIGH;
  static bool lastTimer = HIGH;
  static bool lastLock = HIGH;
  static bool lastCancel = HIGH;
  static bool lastSilence = HIGH;

  // =================================================
  // MODE
  // =================================================

  bool estadoMode =
  digitalRead(BTN_MODE);

  if(
    estadoMode == LOW &&
    lastMode == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("MODE");

    beepCorto();

    cambiarModo();
  }

  lastMode = estadoMode;

  // =================================================
  // UP
  // =================================================

  bool estadoUp =
  digitalRead(BTN_UP);

  if(
    estadoUp == LOW &&
    lastUp == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("UP");

    beepCorto();

    subirValor();
  }

  lastUp = estadoUp;

  // =================================================
  // DOWN
  // =================================================

  bool estadoDown =
  digitalRead(BTN_DOWN);

  if(
    estadoDown == LOW &&
    lastDown == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("DOWN");

    beepCorto();

    bajarValor();
  }

  lastDown = estadoDown;

  // =================================================
  // APGAR
  // =================================================

  bool estadoApgar =
  digitalRead(BTN_APGAR);

  if(
    estadoApgar == LOW &&
    lastApgar == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("APGAR");

    beepCorto();

    Serial2.println("apgar");
  }

  lastApgar = estadoApgar;

  // =================================================
  // TIMER
  // =================================================

  bool estadoTimer =
  digitalRead(BTN_TIMER);

  if(
    estadoTimer == LOW &&
    lastTimer == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("TIMER");

    beepCorto();

    Serial2.println("timer");
  }

  lastTimer = estadoTimer;

  // =================================================
  // LOCK
  // =================================================

  bool estadoLock =
  digitalRead(BTN_LOCK);

  if(
    estadoLock == LOW &&
    lastLock == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("LOCK");

    beepCorto();

    Serial2.println("bloqueo");
  }

  lastLock = estadoLock;

  // =================================================
  // CANCEL
  // =================================================

  bool estadoCancel =
  digitalRead(BTN_CANCEL);

  if(
    estadoCancel == LOW &&
    lastCancel == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("CANCEL");

    beepCorto();

    Serial2.println("cancelar");
  }

  lastCancel = estadoCancel;

  // =================================================
  // SILENCE
  // =================================================

  bool estadoSilence =
  digitalRead(BTN_SILENCE);

  if(
    estadoSilence == LOW &&
    lastSilence == HIGH &&
    millis() - ultimoBoton > debounceTiempo
  ) {

    ultimoBoton = millis();

    Serial.println("SILENCE");

    beepCorto();

    Serial2.println("silencio");
  }

  lastSilence = estadoSilence;
}

// =====================================================
// SUBIR
// =====================================================

void subirValor() {

  if(modoActual == MANUAL) {

    potencia += 10;

    if(potencia > 100) {

      potencia = 100;
    }

    Serial2.print("POWER:");
    Serial2.println(potencia);

    Serial2.println("aumento");
  }

  if(modoActual == BABY) {

    tempObjetivo += 0.5;

    if(tempObjetivo > 40.0) {

      tempObjetivo = 40.0;
    }

    Serial2.print("SETTEMP:");
    Serial2.println(tempObjetivo);

    Serial2.println("aumento");
  }
}

// =====================================================
// BAJAR
// =====================================================

void bajarValor() {

  if(modoActual == MANUAL) {

    potencia -= 10;

    if(potencia < 0) {

      potencia = 0;
    }

    Serial2.print("POWER:");
    Serial2.println(potencia);

    Serial2.println("disminucion");
  }

  if(modoActual == BABY) {

    tempObjetivo -= 0.5;

    if(tempObjetivo < 32.0) {

      tempObjetivo = 32.0;
    }

    Serial2.print("SETTEMP:");
    Serial2.println(tempObjetivo);

    Serial2.println("disminucion");
  }
}

// =====================================================
// CAMBIAR MODOS
// =====================================================

void cambiarModo() {

  if(modoActual == PREWARM) {

    modoActual = MANUAL;
  }

  else if(modoActual == MANUAL) {

    modoActual = BABY;
  }

  else {

    modoActual = PREWARM;
  }

  actualizarModo();

  enviarModoUART();
}

// =====================================================
// UART
// =====================================================

void enviarModoUART() {

  if(modoActual == PREWARM) {

    Serial2.println("precalentamiento");
  }

  if(modoActual == MANUAL) {

    Serial2.println("manual");
  }

  if(modoActual == BABY) {

    Serial2.println("bebe");
  }
}

// =====================================================
// LEDS MODOS
// =====================================================

void actualizarModo() {

  estado595 = 0x0000;

  // PREWARM

  if(modoActual == PREWARM) {

    estado595 = 0x0040;

    potencia = 30;
  }

  // MANUAL

  if(modoActual == MANUAL) {

    estado595 = 0x0080;
  }

  // BABY

  if(modoActual == BABY) {

    estado595 = 0x0100;
  }

  actualizar595();
}

// =====================================================
// 74HC595
// =====================================================

void actualizar595() {

  digitalWrite(LATCH_PIN, LOW);

  shiftOut(
    DATA_PIN,
    CLOCK_PIN,
    MSBFIRST,
    highByte(estado595)
  );

  shiftOut(
    DATA_PIN,
    CLOCK_PIN,
    MSBFIRST,
    lowByte(estado595)
  );

  digitalWrite(LATCH_PIN, HIGH);
}

// =====================================================
// SENSOR
// =====================================================

void actualizarSensor() {

  // SOLO EN BABY

  if(modoActual != BABY) {

    return;
  }

  // =============================================
  // TEMPERATURA SIMULADA
  // =============================================

  float variacion =
  random(-2,3) * 0.1;

  tempBebe += variacion;

  // ACERCARSE AL SETPOINT

  if(tempBebe < tempObjetivo) {

    tempBebe += 0.1;
  }

  if(tempBebe > tempObjetivo) {

    tempBebe -= 0.1;
  }

  // LIMITES

  if(tempBebe > 39.5) {

    tempBebe = 39.5;
  }

  if(tempBebe < 32.0) {

    tempBebe = 32.0;
  }

  // EVITAR SPAM

  if(abs(tempBebe - ultimaTemp) < 0.2) {

    return;
  }

  ultimaTemp = tempBebe;

  Serial2.print("BABYTEMP:");
  Serial2.println(tempBebe);
}

// =====================================================
// CONTROL PWM
// =====================================================

void controlarPotencia() {

  if(modoActual == BABY) {

    if(tempBebe < tempObjetivo) {

      potencia++;
    }

    if(tempBebe > tempObjetivo) {

      potencia--;
    }

    if(potencia > 100) {

      potencia = 100;
    }

    if(potencia < 0) {

      potencia = 0;
    }
  }

  int duty =
  map(potencia,0,100,0,255);

  ledcWrite(PWM_LED,duty);
}

// =====================================================
// BUZZER
// =====================================================

void beepCorto() {

  tone(BUZZER,2000);

  delay(60);

  noTone(BUZZER);
}