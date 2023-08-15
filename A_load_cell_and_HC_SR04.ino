

#include "HX711.h"

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2

HX711 scale;

float calibration_factor = -1194.0; //-1194.0 worked for 697gram on my 1kg load cell scale setup
const int CH1_pingPin = 7; // CH1 Trigger Pin of Ultrasonic Sensor
const int CH1_echoPin = 6; // CH1 Echo Pin of Ultrasonic Sensor
const int CH2_pingPin = 5; // CH2 Trigger Pin of Ultrasonic Sensor
const int CH2_echoPin = 4; // CH2 Echo Pin of Ultrasonic Sensor

long duration, mm;

void setup() {
  Serial.begin(115200);
/*
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
*/  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  
  long zero_factor = scale.read_average(); //Get a baseline reading
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  pinMode(CH1_pingPin, OUTPUT);
  pinMode(CH1_echoPin, INPUT);
  pinMode(CH2_pingPin, OUTPUT);
  pinMode(CH2_echoPin, INPUT);
}


void loop() {

//  scale.set_scale(calibration_factor); //Adjust to this calibration factor
//  Serial.print("Reading: ");

  Serial.print("P=");
  Serial.print(scale.get_units(), 1);
  Serial.print(" g :"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person

//  Serial.print(" calibration_factor: ");
//  Serial.print(calibration_factor);
//  Serial.println();

 
  
  digitalWrite(CH1_pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(CH1_pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(CH1_pingPin, LOW);
 
  duration = pulseIn(CH1_echoPin, HIGH);
  mm = microsecondsToMilimeters(duration);
  Serial.print("V=");
  Serial.print(mm);
  Serial.print(" mm :");
//  Serial.println();
  delay(1);

  digitalWrite(CH2_pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(CH2_pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(CH2_pingPin, LOW);
 
  duration = pulseIn(CH2_echoPin, HIGH);
  mm = microsecondsToMilimeters(duration);
  Serial.print("D=");
  Serial.print(mm);
  Serial.print(" mm");

  Serial.println();
  delay(1);


/*
  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
*/

}

long microsecondsToMilimeters(long microseconds) {
   return 10 * microseconds / 29 / 2;
}
