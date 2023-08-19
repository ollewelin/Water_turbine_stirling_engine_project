#include "HX711.h"

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN   2

HX711 scale;
float calibration_factor = -1194.0;

const int CH1_pingPin = 7;
const int CH1_echoPin = 6;
const int CH2_pingPin = 5;
const int CH2_echoPin = 4;

const int NTC_pin = A0; // Analog pin for NTC temperature sensor
/*
#Temperature (°C) | Resistance (kΩ)
#---------------------------------
#24               | 260
#30               | 200
#34               | 161
#55               | 72
#90               | 20.2
#100              | 14.5
#120              | 7.9
#150              | 3.47
#160              | 2.65
#170              | 2.08
#180              | 1.66
#191              | 1.31
#195              | 1.17
*/

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();
  long zero_factor = scale.read_average();
  scale.set_scale(calibration_factor);
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
  pinMode(CH1_pingPin, OUTPUT);
  pinMode(CH1_echoPin, INPUT);
  pinMode(CH2_pingPin, OUTPUT);
  pinMode(CH2_echoPin, INPUT);
}

void loop() {
  Serial.print("P=");
  Serial.print(scale.get_units(), 1);
  Serial.print(" g :");

  digitalWrite(CH2_pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(CH2_pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(CH2_pingPin, LOW);
  long duration = pulseIn(CH2_echoPin, HIGH);
  long mm = microsecondsToMillimeters(duration);
  Serial.print("V=");
  Serial.print(mm);
  Serial.print(" mm :");

  delay(1);

  digitalWrite(CH1_pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(CH1_pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(CH1_pingPin, LOW);
  duration = pulseIn(CH1_echoPin, HIGH);
  mm = microsecondsToMillimeters(duration);
  Serial.print("D=");
  Serial.print(mm);
  Serial.print(" mm :");

  // Read NTC temperature sensor value
  int ntcRawValue = analogRead(NTC_pin);

  // Convert NTC value to resistance
  float ntcResistance = calculateNTCResistance(ntcRawValue);

/*
  // Convert resistance to temperature in Celsius
  float ntcTemperature = calculateNTCTemperature(ntcResistance);

  Serial.print("T=");
  Serial.print(ntcTemperature, 2);
  Serial.println(" °C");
*/
  Serial.print("R=");
  Serial.print(ntcResistance, 2);
  Serial.println(" Ω");

  delay(1);
}

long microsecondsToMillimeters(long microseconds) {
  return 10 * microseconds / 29 / 2;
}

// Calculate NTC resistance based on analog value
float calculateNTCResistance(int rawValue) {
  float voltage = (float)rawValue * 5.0 / 1023.0;
  float ntcResistance = 4700.0 * voltage / (5.0 - voltage);
  return ntcResistance;
}

// Convert NTC resistance to temperature using Steinhart-Hart equation
float calculateNTCTemperature(float resistance) {
  const float A = 0.001125308852122;   // Steinhart-Hart coefficients
  const float B = 0.000234711863267;
  const float C = 0.000000085663516;
  float steinhart = resistance / 4700.0; // Normalize resistance
  steinhart = log(steinhart);            // ln(R/R0)
  steinhart /= A;
  steinhart += B;
  steinhart += C;
  steinhart = 1.0 / steinhart;           // Invert
  steinhart -= 273.15;                   // Convert to Celsius
  return steinhart;
}

 
