#include "HX711.h"
#include "DHT.h"

// ===============================
// PIN CONFIGURATION
// ===============================
#define DOUT 23
#define CLK 19

#define DHTPIN 26
#define DHTTYPE DHT11

#define PIRPIN 14
#define BUZZER 33

// ===============================
// OBJECTS
// ===============================
HX711 scale;
DHT dht(DHTPIN, DHTTYPE);

// ===============================
// SETTINGS
// ===============================
const float WEIGHT_THRESHOLD = 2.5;   // kg
const int BUZZER_FREQUENCY = 3000;    // Hz

unsigned long previousBuzzerMillis = 0;
bool buzzerState = false;

// ===============================
// SETUP
// ===============================
void setup() {

  Serial.begin(115200);

  // Load cell
  scale.begin(DOUT, CLK);
  scale.set_scale(211000);
  scale.tare();

  // DHT11
  dht.begin();

  // PIR
  pinMode(PIRPIN, INPUT);

  // Passive buzzer
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);

  Serial.println();
  Serial.println("------------------------------------------");
  Serial.println("CHILD PRESENCE DETECTION SYSTEM");
  Serial.println("------------------------------------------");
}

// ===============================
// WARNING LEVEL 1
// Slow beep
// ===============================
void warningLevel1() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousBuzzerMillis >= 1000) {

    previousBuzzerMillis = currentMillis;
    buzzerState = !buzzerState;

    if (buzzerState) {
      tone(BUZZER, BUZZER_FREQUENCY);
    } else {
      noTone(BUZZER);
    }
  }
}

// ===============================
// WARNING LEVEL 2
// Fast beep
// ===============================
void warningLevel2() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousBuzzerMillis >= 200) {

    previousBuzzerMillis = currentMillis;
    buzzerState = !buzzerState;

    if (buzzerState) {
      tone(BUZZER, BUZZER_FREQUENCY);
    } else {
      noTone(BUZZER);
    }
  }
}

// ===============================
// BUZZER OFF
// ===============================
void buzzerOff() {

  noTone(BUZZER);
  buzzerState = false;
}

// ===============================
// LOOP
// ===============================
void loop() {

  // -------------------------------
  // READ PIR
  // -------------------------------
  int statusPIR = digitalRead(PIRPIN);

  // -------------------------------
  // READ DHT11
  // -------------------------------
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // -------------------------------
  // READ LOAD CELL
  // -------------------------------
  float weight = scale.get_units(5);

  if (weight < 0) {
    weight = 0;
  }

  // ===============================
  // DISPLAY CURRENT LOAD
  // ===============================
  Serial.print("Current Load: ");
  Serial.print(weight, 2);
  Serial.print(" kg");

  Serial.print(" | Threshold: ");
  Serial.print(WEIGHT_THRESHOLD, 2);
  Serial.print(" kg");

  // ===============================
  // DISPLAY PIR
  // ===============================
  Serial.print(" | PIR: ");

  if (statusPIR == HIGH) {
    Serial.print("MOVEMENT");
  } else {
    Serial.print("NO MOVEMENT");
  }

  // ===============================
  // DISPLAY TEMPERATURE
  // ===============================
  Serial.print(" | Temp: ");

  if (isnan(temperature)) {
    Serial.print("ERROR");
  } else {
    Serial.print(temperature, 1);
    Serial.print(" C");
  }

  // ===============================
  // DISPLAY HUMIDITY
  // ===============================
  Serial.print(" | Humidity: ");

  if (isnan(humidity)) {
    Serial.println("ERROR");
  } else {
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  // ===============================
  // WARNING LOGIC
  // ===============================

  // NORMAL
  if (weight <= WEIGHT_THRESHOLD) {

    buzzerOff();

    Serial.println("STATUS: NORMAL");
    Serial.println("No significant load detected.");
  }

  // WARNING LEVEL 1
  // Load > 2.5 kg, but PIR does not detect movement
  else if (weight > WEIGHT_THRESHOLD && statusPIR == LOW) {

    warningLevel1();

    Serial.println("STATUS: WARNING LEVEL 1");
    Serial.println("Possible child presence detected.");
    Serial.println("Load > 2.5 kg, but no movement detected.");
  }

  // WARNING LEVEL 2
  // Load > 2.5 kg AND PIR detects movement
  else if (weight > WEIGHT_THRESHOLD && statusPIR == HIGH) {

    warningLevel2();

    Serial.println("STATUS: WARNING LEVEL 2 - URGENT");
    Serial.println("Child presence and movement detected!");
  }

  Serial.println("------------------------------------------");

  delay(200);
}