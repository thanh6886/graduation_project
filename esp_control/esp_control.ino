#include "EEPROM.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  

#define EEPROM_SIZE 	200

#define RX2 			16
#define TX2				17
#define positionCAM 	15
#define positionA 		4
#define positionB 		5

#define ServoA 			18
#define ServoB 			19
#define ConveyorBelt 	2


#define button_Mode 	13
#define button_status 	23
#define button_L1 		12
#define button_L2 		14
#define button_L3 		27
// 21 sda 22 scl



Servo myServoA, myServoB;
String ClassValue = "";
char buffer_1[30], buffer_2[30], buffer_3[30] , buffer_L[30];

uint16_t CountA = 0 , CountB = 0 , CountC = 0, Sum = 0; 
bool L1 = 0, L2 = 0, L3 = 0, _mode = 0, _status = 0;

uint8_t sttOld_UART = 1, sttNew_UART = 1;
uint8_t sttNew_L2 = 1, sttNew_L3 = 1,sttOld_L2 = 1, sttOld_L3 = 1 ;
uint8_t sttOld_Mode = 1,  sttOld_status = 1,  sttNew_Mode = 1 , sttNew_status = 1 ;
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


void    Hbutton_Mode(void);
void    Hbutton_status(void);
void    Hbutton_UART(void);
void    Hbutton_L2(void);
void    Hbutton_L3(void);



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
    pinMode(ConveyorBelt, OUTPUT);
    pinMode(positionCAM, INPUT);
    pinMode(positionA, INPUT);
    pinMode(positionB, INPUT);

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
    Serial.println("loading");
    lcd.setCursor(5, 1);
    lcd.print("LOADING...");
    String receivedStauts = Serial.readStringUntil('\n');
      if(receivedStauts == "setup_done"){
        lcd.clear();
        displayLCD();
        receivedStauts = "";
        Serial.println("START");
        xSemaphoreGive(xSemaphoreControl);
        vTaskDelete(StatusCam);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }

}

void ReceiveUART(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreUART, portMAX_DELAY) == pdTRUE) {
      while(1){
        if(Serial2.available() > 0){
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
			  status = false;
              Serial2.print("start\n");
              xSemaphoreGive(xSemaphoreUART);
              vTaskResume(UART_Receive);
              vTaskSuspend(Control_TASK);
            }
          if(digitalRead(positionA) == 1 && ClassValue == "A"){
              Serial.println("sản phẩm A");
              controlServo(myServoA, L2);
            }
          if(digitalRead(positionB) == 1 && ClassValue == "B"){
              Serial.println("sản phẩm B");
              controlServo(myServoB, L3);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
         }
      }
}

void writeToFlash(const uint16_t value , int addr){
    EEPROM.write(1,0);
    EEPROM.write(value, addr);   //  1 -> 3 count A  -- 4 -> 6 count B -- 7 -> 9 count C
    EEPROM.commit();
}

void controlServo(Servo &s, bool &cpm){
  	cpm = true;
       memset(buffer_L, 0, sizeof(buffer_L));
    s.write(90);
    displayLCD(); 
    vTaskDelay(pdMS_TO_TICKS(2000));
    s.write(0);
    cpm = false;

        memset(buffer_L, 0, sizeof(buffer_L));
	  displayLCD();
}

void displayLCD(){
  sprintf(buffer_1, 	"<A:%u | B:%u | C:%u>", CountA, CountB, CountC);
  sprintf(buffer_2, 	"|CLASS:%s | TOTAL:%u|", ClassValue, Sum);
  sprintf(buffer_3, 	"-|%s| <|> |%s|-", 
          _mode   ?   	"AUTO":"HAND", 
          _status ? 	"START":"STOP ");  
  sprintf(buffer_L, 	"<L1:%sL2:%sL3:%s>", 
          L1 ? "on|" : "off", 
          L2 ? "on|" : "off", 
          L3 ? "on|" : "off" );
  lcd.setCursor(0, 0);  
  lcd.print(buffer_1);
  lcd.setCursor(0, 1); 
  lcd.print(buffer_2);
  lcd.setCursor(0, 2); 
  lcd.print(buffer_3);
  lcd.setCursor(0, 3);
  lcd.print(buffer_L);
}

// void Button_task(void *pvParameters){
//       while(1){
//         Hbutton_Mode();
//         if(_mode){
//           Hbutton_status();
// 		      Hbutton_UART();
// 		      Hbutton_L2();
// 		      Hbutton_L3();
//         }
//         displayLCD();
//         vTaskDelay(pdMS_TO_TICKS(100));
//       }
// }

void Hbutton_Mode(void){
    sttOld_Mode = sttNew_Mode;
    sttNew_Mode = digitalRead(button_Mode);
    if(sttOld_Mode == 0 && sttNew_Mode == 1){
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

void Hbutton_status(void){
    sttOld_status = sttNew_status;
    sttNew_status = digitalRead(button_status);
    if(sttOld_status == 0 && sttNew_status == 1){
        _status != _status;
        digitalWrite(ConveyorBelt,_status);
      }
       vTaskDelay(pdMS_TO_TICKS(5));

    }
    
void Hbutton_UART(void){
    sttOld_UART = sttNew_UART;
    sttNew_UART = digitalRead(button_L1);
    if(sttOld_UART == 0 && sttNew_UART == 1){
        L1 !=L1;
        Serial2.print("start\n");
      }
       vTaskDelay(pdMS_TO_TICKS(5));

    }


void Hbutton_L2(void){
    sttOld_L2 = sttNew_L2;
    sttNew_L2 = digitalRead(button_L2);
    if(sttOld_L2 == 0 && sttNew_L2 == 1){
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

void Hbutton_L3(void){
    sttOld_L3 = sttNew_L3;
    sttNew_L3 = digitalRead(button_L3);
    if(sttOld_L3 == 0 && sttNew_L3 == 1){
        counter_L3++;
        L3 != L3;
      }
    if(counter_L2%2==0){
        myServoB.write(90);
    }
    else{
      myServoB.write(0);
    }
     vTaskDelay(pdMS_TO_TICKS(5));

}

