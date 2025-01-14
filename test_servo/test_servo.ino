#include <ESP32Servo.h>
#define ServoA 			        16
#define ServoB 			        17
Servo myServoA, myServoB;
void controlServo(Servo &s);
void setup() {
  // put your setup code here, to run once:
    myServoA.attach(ServoA);
    myServoB.attach(ServoB);
    Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
    controlServo(myServoA);
    delay(2000);
    controlServo(myServoB);
    delay(2000);
}
void controlServo(Servo &s){
    s.write(90); 
    delay(3000);
    s.write(0);
}
