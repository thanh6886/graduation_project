#include "EEPROM.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  

#define EEPROM_SIZE 	200

#define RX2 			16
#define TX2 			17
#define positionCAM 	15
#define positionA 		4
#define positionB 		5

#define ServoA 			18
#define ServoB 			19
#define ConveyorBelt 	21
#define LED_Status 		2

#define button_Mode 	22
#define button_status 	23
#define button_L1 		12
#define button_L2 		13
#define button_L3 		14




Servo myServoA, myServoB;
uint8_t ClassValue = "";
uint8_t buffer_1[22], buffer_2[20], buffer_3[20] , buffer_1[20];

uint16_t CountA = 0 , CountB = 0 , CountC = 0, Sum = 0; 
bool L1 = 0, L2 = 0, L3 = 0, _mode = 0, _status = 0;
uint8_t sttOld_Mode = 1,  sttOld_status = 1,  sttNew_Mode = 1 , sttNew_status = 1 ;
uint8_t sttOld_UART = 1, sttNew_UART = 1;
uint8_t sttNew_L2 = 1, sttNew_L3 = 1,sttOld_L2 = 1, sttOld_L3 = 1 ;
uint16_t counter_Mode = 0, counter_L2 = 0, counter_L3 = 0;

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

void displayLCD(void);


void setup(){
    Serial.begin(115200);
    Serial2.begin(4800, SERIAL_8N1, RX2, TX2);
    EEPROM.begin(EEPROM_SIZE);
    lcd.init();
    lcd.backlight();
    lcd.clear();
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
    xTaskCreatePinnedToCore(Status_CAM,"task3", 32, NULL, 1, &StatusCam, 1);
    xTaskCreatePinnedToCore(Button_task,"task4", 255, NULL, 1, NULL, 1);
    Serial.println("setup done");
}
void loop(){}


void Status_CAM(void *pvParameters){
  while(Serial2.available()){
    lcd.setCursor(5, 1);
    lcd.print("LOADING...");
    String receivedStauts = Serial2.readStringUntil('\n');
      if(receivedStauts == "setup_done"){
        lcd.clear();
        displayLCD();
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
            CountA++;
            writeToFlash(CountA, 1);
          }
          else if(receivedStauts == "B"){
            CountB++;
            writeToFlash(CountB, 4);
          }
          else if(receivedStauts == "C"){
            Serial.println("sản phẩm C");
            CountC++;
            writeToFlash(CountC, 7);
          }
          ClassValue = receivedStauts;
          Sum = CountA + CountB + CountC;
          displayLCD();
          if(!_mode){
           xSemaphoreGive(xSemaphoreControl);
           vTaskResume(Control_TASK);
           vTaskSuspend(UART_Receive);
          }
      }
    }
}
void Control(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreControl, portMAX_DELAY) == pdTRUE) {
		Serial.println("bật băng tải");
        digitalWrite(ConveyorBelt,HIGH);
		_status = true;
        while(1){
          if(digitalRead(positionCAM) == 1){
              Serial.println("đọc mã qr code");
              digitalWrite(ConveyorBelt,LOW);
			  _status = false;
              Serial2.print("start\n");
              xSemaphoreGive(xSemaphoreUART);
              vTaskResume(UART_Receive);
              vTaskSuspend(Control_TASK);
            }
          if(digitalRead(positionA) == 1 && ClassValue == "A"){
              Serial.println("sản phẩm A");
              controlServo(myServoA, L2);
            }
          if(digitalRead(positionB) == 1 && ClassValue == "B"{
              Serial.println("sản phẩm B");
              controlServo(myServoB, L3);
            }
         }
      }
}

void writeToFlash(const uint16_t value , int addr){
    EEPROM.write(1,0);
    EEPROM.write(value, addr);   //  1 -> 3 count A  -- 4 -> 6 count B -- 7 -> 9 count C
    EEPROM.commit();
}

void controlServo(Servo &s, bool &cpm){
    
    s.write(90);
	  cpm = true;
    displayLCD();
    vTaskDelay(pdMS_TO_TICKS(2000));
	  cpm = false;
    s.write(0);
	  displayLCD();
}
controlServo(myServoB, L3);

void displayLCD(){
  sprintf(buffer_1, 	"<A:%u | B:%u | C:%u>", valueA, valueB, valueC);
  sprintf(buffer_2, 	"|CLASS:%u | TOTAL:%u|", ClassValue, Sum);
  sprintf(buffer_3, 	"-|%s| <|> |%s|-", 
          _mode   ?   	"AUTO:HAND", 
          _status ? 	"START":"STOP ";  
  sprintf(buffer_4, 	"<L1:%sL2:%sL3:%s>", 
          L1 ? "on|" : "off", 
          L2 ? "on|" : "off", 
          L3 ? "on|" : "off", 
  lcd.setCursor(0, 0);  
  lcd.print(buffer_1);
  lcd.setCursor(0, 1); 
  lcd.print(buffer_2);
  lcd.setCursor(0, 2); 
  lcd.print(buffer_3);
  lcd.setCursor(0, 3);
  lcd.print(buffer_4);
}

void Button_task(void *pvParameters){
      while(1){
        currentMillis = millis();
        button_Mode();
        if(_mode){
          button_status();
		  button_UART();
		  button_L2();
		  button_L3();
        }
        displayLCD();
        vTaskDelay(pdMS_TO_TICKS(50));
      }
}

void button_Mode(void){
    sttOld_Mode = sttNew_Mode;
    sttNew_Mode = digitalRead(button_Mode);
    if(sttOld == 1 && sttNew == 0){
        counter_Mode++;
      }
    if(counter_Mode%2==0){
        xSemaphoreGive(xSemaphoreUART);
        vTaskResume(UART_Receive);
        vTaskSuspend(Control_TASK);
        _mode = 1;
    }
    else{
        xSemaphoreGive(xSemaphoreControl);
        vTaskResume(Control_TASK);
        vTaskSuspend(UART_Receive);
        _mode = 0;
    }
     vTaskDelay(pdMS_TO_TICKS(5));
}

void button_status(void){
    sttOld_status = sttNew_status;
    sttNew_status = digitalRead(button_status);
    if(sttOld_status == 1 && sttNew_status == 0){
        _status != _status
        digitalWrite(ConveyorBelt,_status);
      }
    }
     vTaskDelay(pdMS_TO_TICKS(5));
}
void button_UART(void){
    sttOld_UART = sttNew_UART;
    sttNew_UART = digitalRead(button_L1);
    if(sttOld_UART == 1 && sttNew_UART == 0){
        L1 !=L1;
        Serial2.print("start\n");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
}

void button_L2(void){
    sttOld_L2 = sttNew_L2;
    sttNew_L2 = digitalRead(button_L2);
    if(sttOld_L2 == 1 && sttNew_L2 == 0){
        counter_L2++;
        L2 !=L2;
      }
    if(counter_L2%2==0){
         myServoA.write(90);
    }
    else{
       myServoA.write(0);
    }
     vTaskDelay(pdMS_TO_TICKS(5));
}

void button_L3(void){
    sttOld_L3 = sttNew_L3;
    sttNew_L3 = digitalRead(button_L3);
    if(sttOld_L3 == 1 && sttNew_L3 == 0){
        counter_L3++;
        L3 != L3
      }
    if(counter_L2%2==0){
        myServoB.write(90);
    }
    else{
      myServoB.write(0);
    }
     vTaskDelay(pdMS_TO_TICKS(5));

}

