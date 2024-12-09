#include "EEPROM.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  

#define EEPROM_SIZE 200

#define RX2 16
#define TX2 17
#define positionCAM 15
#define positionA 4
#define positionB 5

#define ServoA 18
#define ServoB 19
#define ConveyorBelt 21
#define LED_Status 2

Servo myServoA, myServoB;



uint16_t CountA = 0 , CountB = 0 , CountC = 0; 

TaskHandle_t UART_Receive;
TaskHandle_t Control_TASK;
TaskHandle_t StatusCam;
SemaphoreHandle_t xSemaphoreControl;
SemaphoreHandle_t xSemaphoreUART;


void writeToFlash(const uint16_t value , int addr);
void controlServo(Servo &s);

void ReceiveUART(void *pvParameters);
void Status_CAM(void *pvParameters);
void Control(void *pvParameters);

void setup(){
    Serial.begin(115200);
    Serial2.begin(4800, SERIAL_8N1, RX2, TX2);
    EEPROM.begin(EEPROM_SIZE);
    lcd.init();
    lcd.backlight();
    if (EEPROM.read(0)  == 1) { 
       CountA = EEPROM.read(1);
       CountB = EEPROM.read(4);
       CountC = EEPROM.read(7);
    }  
    
     
    pinMode(LED_Status, OUTPUT);
    pinMode(ConveyorBelt, OUTPUT);
    pinMode(positionCAM, INPUT);
    pinMode(positionA, INPUT);
    pinMode(positionB, INPUT);

    myServoA.attach(ServoA);
    myServoB.attach(ServoB);

    xSemaphoreControl = xSemaphoreCreateBinary();
    xSemaphoreUART = xSemaphoreCreateBinary();
    digitalWrite(LED_Status, HIGH);
    xTaskCreatePinnedToCore(ReceiveUART, "task1", 2048, NULL, 1, &UART_Receive,1);
    xTaskCreatePinnedToCore(Control,"task2", 2048, NULL, 1, &Control_TASK, 1);
    xTaskCreatePinnedToCore(Status_CAM,"task3", 64, NULL, 1, &StatusCam, 1);
    Serial.println("setup done");
}
void loop(){}


void Status_CAM(void *pvParameters){
  while(Serial2.available()){
    String receivedStauts = Serial2.readStringUntil('\n');
      if(receivedStauts == "setup_done"){
        digitalWrite(LED_Status, LOW);
        receivedStauts = "";
        Serial.println("START");
        xSemaphoreGive(xSemaphoreControl);
        vTaskDelete(StatusCam);
      }
    }
}

void ReceiveUART(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreUART, portMAX_DELAY) == pdTRUE) {
      while(Serial2.available()){
        String receivedStauts = Serial2.readStringUntil('\n');
        Serial.println("đọc mã thành công");
        Serial.println(receivedStauts);
        digitalWrite(ConveyorBelt,HIGH);
         if(receivedStauts == "A"){
            receivedStauts = "";
            CountA++;
            writeToFlash(CountA, 1);
            if(digitalRead(positionA) == 1){
              Serial.println("sản phẩm A");
              controlServo(myServoA);
            }
          }
          else if(receivedStauts == "B"){
            receivedStauts = "";
            CountB++;
            writeToFlash(CountB, 4);
            if(digitalRead(positionB == 1)){
              Serial.println("sản phẩm B");
              controlServo(myServoA);
            }
          }
          else if(receivedStauts == "C"){
            Serial.println("sản phẩm C");
            receivedStauts = "";
            CountC++;
            writeToFlash(CountC, 7);
          }
          
          xSemaphoreGive(xSemaphoreControl);
          vTaskResume(Control_TASK);
          vTaskSuspend(UART_Receive);
      }
    }
}
void Control(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreControl, portMAX_DELAY) == pdTRUE) {
        while(1){
          Serial.println("bật băng tải");
          digitalWrite(ConveyorBelt,HIGH);
          if(digitalRead(positionCAM) == 1){
              Serial.println("đọc mã qr code");
              digitalWrite(ConveyorBelt,LOW);
              xSemaphoreGive(xSemaphoreUART);
              vTaskResume(UART_Receive);
              vTaskSuspend(Control_TASK);
            }
         }
      }
}

void writeToFlash(const uint16_t value , int addr){
    EEPROM.write(1,0);
    EEPROM.write(value, addr);   //  1 -> 3 count A  -- 4 -> 6 count B -- 7 -> 9 count C
    EEPROM.commit();
}

void controlServo(Servo &s){
    for (int pos = 0; pos <= 180; pos++) {
        s.write(pos);
        delay(15);
    }
   delay(2000);
    for (int pos = 180; pos >= 0; pos--) {
        s.write(pos);
        delay(15);
    }
}


