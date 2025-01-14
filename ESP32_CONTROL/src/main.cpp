#include "EEPROM.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  

#define EEPROM_SIZE 	200

#define RX2 			          18
#define TX2				          19


#define positionCAM 	      13 
#define positionA 		      12
#define positionB 		      4

#define ServoA 			        16
#define ServoB 			        17
#define ConveyorBelt 	      2


#define button_Mode 	      5
#define button_status       23
#define button_CAM 		      15
#define button_L1 		      14
#define button_L2 		      27
// 21 sda 22 scl



Servo myServoA, myServoB;
String ClassValue = "";
char buffer_1[30], buffer_2[30], buffer_3[30] , buffer_L[30];

uint16_t CountA = 0 , CountB = 0 , CountC = 0, Sum = 0; 
bool L1 = 0, L2 = 0, _Cam = 0, _Mode = 0, _Status = 0;
bool UART_QR = 0;
// 0 là tắt 1 là chạy _mode    ==  0 là auto  1 là hand

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
void Button_task(void *pvParameters);
void displayLCD(void);

void    Hbutton_Mode(void);
void    Hbutton_Status(void);
void    Hbutton_Cam(void);
void    Hbutton_L1(void);
void    Hbutton_L2(void);



void setup(){
    Serial.begin(115200);
    Serial2.begin(4800, SERIAL_8N1, RX2, TX2);
    EEPROM.begin(EEPROM_SIZE);
    lcd.init();
    lcd.backlight();
    lcd.clear();


    Serial.println("loading...");
    lcd.setCursor(5, 1);
    lcd.print("LOADING...");


    if (EEPROM.read(0)  == 1) { 
       CountA = EEPROM.read(1);
       CountB = EEPROM.read(4);
       CountC = EEPROM.read(7);
    }  
    pinMode(ConveyorBelt, OUTPUT);
    pinMode(positionCAM, INPUT_PULLDOWN);
    pinMode(positionA, INPUT_PULLDOWN);
    pinMode(positionB, INPUT_PULLDOWN);

    myServoA.attach(ServoA);
    myServoB.attach(ServoB);

    xSemaphoreControl = xSemaphoreCreateBinary();
    xSemaphoreUART = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(ReceiveUART, "task1", 2048, NULL, 2, &UART_Receive,1);
    xTaskCreatePinnedToCore(Control,"task2", 2048, NULL, 2, &Control_TASK, 1);
    xTaskCreatePinnedToCore(Status_CAM,"task3", 2048, NULL, 1, &StatusCam, 1);
    // xTaskCreatePinnedToCore(Button_task,"task4", 2048, NULL, 1, NULL, 0);
    Serial.println("setup done");
}
void loop(){}


void Status_CAM(void *pvParameters){
  while(1){   
    if(Serial.available() > 0){
    String receivedStauts = Serial.readStringUntil('\n');
      if(receivedStauts == "setup_done"){
        lcd.clear();
        receivedStauts = "";
        Serial.println("START");
        displayLCD();
        xSemaphoreGive(xSemaphoreControl);
        vTaskDelete(StatusCam);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

void ReceiveUART(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreUART, portMAX_DELAY) == pdTRUE) {
      while(1){
        if(Serial.available() > 0){
        String receivedStauts = Serial.readStringUntil('\n');
        Serial.println("Read qr code successfully");
        Serial.println(receivedStauts);
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
          Serial.println(ClassValue);
          Serial.println(Sum);
          UART_QR = 0;
          vTaskDelay(pdMS_TO_TICKS(1000));
          vTaskResume(Control_TASK);
          vTaskSuspend(UART_Receive);

        }
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
}
void Control(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreControl, portMAX_DELAY) == pdTRUE) {
		    Serial.println("Turn on the conveyor belt");
        Serial.print("L:BT:1\n"); // bật băng tải 
        digitalWrite(ConveyorBelt, LOW);
        while(1){
          if(digitalRead(positionCAM) == 1 && UART_QR == 0){
            Serial.println(digitalRead(positionCAM));
            vTaskDelay(pdMS_TO_TICKS(100));
            if(digitalRead(positionCAM) == 1 && UART_QR == 0){
              Serial.println("read qr code");
              digitalWrite(ConveyorBelt,HIGH);
              Serial2.print("L:BT:0\n");
              Serial2.print("start\n");

              xSemaphoreGive(xSemaphoreUART);
              vTaskResume(UART_Receive);
              vTaskSuspend(Control_TASK);
            }
          }
          if(digitalRead(positionA) == 1 && ClassValue == "A" && UART_QR == 1){
              Serial.println("sản phẩm A");
              controlServo(myServoA);
              UART_QR == 0;
            }
          if(digitalRead(positionB) == 1 && ClassValue == "B"  && UART_QR == 1){
              Serial.println("sản phẩm B");
              controlServo(myServoB);
               UART_QR == 0;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
         }
      }
}


// void Button_task(void *pvParameters){
//     while(1){
//         // Hbutton_Mode();
//         // if(_Mode){
//         //   Hbutton_Status();
// 		    //   Hbutton_Cam();
// 		    //   Hbutton_L1();
// 		    //   Hbutton_L2();
//         // }
//         // displayLCD();
//         vTaskDelay(pdMS_TO_TICKS(50));
//       }
// }

void Hbutton_Mode(){
    static int sttOld_Mode = 0;     
    static int counter_Mode = 0;   
    int sttNew_Mode;
    sttNew_Mode = digitalRead(button_Mode);
    if (sttOld_Mode == 0 && sttNew_Mode == 1) {
        vTaskDelay(pdMS_TO_TICKS(20)); 
        if (digitalRead(button_Mode) == 1) { 
            counter_Mode++;
            if (counter_Mode % 2 == 0) {
              // chế độ hand
                xSemaphoreGive(xSemaphoreUART);
                vTaskResume(UART_Receive);
                vTaskSuspend(Control_TASK);
                _Mode = 1; 
            } else {
              // chế độ auto
                xSemaphoreGive(xSemaphoreControl);
                vTaskResume(Control_TASK);
                vTaskSuspend(UART_Receive);
                _Mode = 0; 
            }
        }
    }
    sttOld_Mode = sttNew_Mode;
    vTaskDelay(pdMS_TO_TICKS(5));
}

void Hbutton_Status(void){
    static int sttOld_Status = 0;     
    static int counter_Status = 0;   
    int sttNew_Status;
    sttNew_Status = digitalRead(button_status);
    if (sttOld_Status == 0 && sttNew_Status == 1) {
        vTaskDelay(pdMS_TO_TICKS(20)); 
        if (digitalRead(button_status) == 1) { 
            counter_Status++;
            if (counter_Status % 2 == 0) {

             Serial.println("bật băng tải");
             Serial2.print("L:BT:1\n"); // bật băng tải 
             digitalWrite(ConveyorBelt, LOW);
   
            } else {
              Serial.println("TẮT BĂNG TẢI");
              digitalWrite(ConveyorBelt,HIGH);
              Serial2.print("L:BT:0\n");
              
            }
        }
    }
    sttOld_Status = sttNew_Status;
    vTaskDelay(pdMS_TO_TICKS(5));
}


void Hbutton_Cam(void){

}
void Hbutton_L1(void){

}
void Hbutton_L2(void){

}


void writeToFlash(const uint16_t value , int addr){
    EEPROM.write(1,0);
    EEPROM.write(value, addr);   //  1 -> 3 count A  -- 4 -> 6 count B -- 7 -> 9 count C
    EEPROM.commit();
}

void controlServo(Servo &s){
    s.write(90); 
    vTaskDelay(pdMS_TO_TICKS(3000));
    s.write(0);
}

void displayLCD(){
  sprintf(buffer_1, 	"<A:%u | B:%u | C:%u>", CountA, CountB, CountC);
  sprintf(buffer_2, 	"|CLASS:%s | TOTAL:%u|", ClassValue, Sum);
  sprintf(buffer_3, 	"-|%s| <|> |%s|-", 
          _Mode   ?   	"AUTO":"HAND", 
          _Status ? 	"START":"STOP ");  
  sprintf(buffer_L, 	"<L1:%sL2:%sL3:%s>", 
          L1 ? "on|" : "off", 
          L2 ? "on|" : "off", 
          _Cam ? "on|" : "off");
  lcd.setCursor(2, 0);  
  lcd.print(buffer_1);
  lcd.setCursor(0, 1); 
  lcd.print(buffer_2);
  lcd.setCursor(0, 2); 
  lcd.print(buffer_3);
  lcd.setCursor(0, 3);
  lcd.print(buffer_L);
}