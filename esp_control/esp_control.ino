#include <EEPROM.h>

#define RXD2 16
#define TXD2 17

#define stepPin 2
#define dirPin 3
#define conveyorBelt 4

#define line_1 5
#define line_2 6
#define line_3 7
#define position_cam 8


TaskHandle_t ReceiveUART_Task;
TaskHandle_t ControlClassify_Task;
TaskHandle_t ControlSample_Task;

SemaphoreHandle_t xSemaphoreUART;
SemaphoreHandle_t xSemaphoreClassify;

enum Product {
    PRODUCT_A,
    PRODUCT_B,
    PRODUCT_C,
    PRODUCT_UNKNOWN
};

int countA = 0;
int countB = 0;
int countC = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  xSemaphoreUART= xSemaphoreCreateBinary();
  xSemaphoreClassify = xSemaphoreCreateBinary();

  pinMode(conveyorBelt, OUTPUT);

  EEPROM.begin();
  xTaskCreatePinnedToCore(ReceiveUART, "ReceiveUART_Task", 256, NULL, 1, &ReceiveUART_Task, 0);
  xTaskCreatePinnedToCore(ControlSample, "ControlSample_Task", 128, NULL, 1, &ControlSample_Task, 0);
  xTaskCreatePinnedToCore(ControlClassify, "ControlClassify", 512, NULL, 1, &ControlClassify_Task, 0);
  
}

void ReceiveUART(void *pvParameters){
  if (xSemaphoreTake(xSemaphoreUART, portMAX_DELAY) == pdTRUE) {
      while (Serial2.available()) {
        String QRCodeResult = Serial2.readStringUntil('!');
          switch (classifyProduct(QRCodeResult)) {
            case PRODUCT_A:
              Serial.println("Processing Product A");
              countA++;
              break;
            case PRODUCT_B:
              Serial.println("Processing Product B");
              countB++
              break;
            case PRODUCT_C:
              Serial.println("Processing Product C");
              countC++
              break;
            default:
              Serial.println("Unknown Product");
              break;
            }
          saveCountsToEEPROM();
          xSemaphoreGive(xSemaphoreClassify); 
          vTaskResume(ControlClassify_Task);
          vTaskSuspend(ReceiveUART_Task);
      }
  }  
}

void ControlSample(void *pvParameters){
  while(1){
    moveStepper(true, 120);
    digitalWrite(conveyorBelt, HIGH);
    if(digitalRead(position_cam) == HIGH){
      digitalWrite(conveyorBelt, LOW);
      xSemaphoreGive(xSemaphoreUART); 
      vTaskSuspend(ControlSample);
    }
  }
}

void ControlClassify(void *pvParameters){
    if (xSemaphoreTake(xSemaphoreClassify, portMAX_DELAY) == pdTRUE) {
      while(1){
        digitalWrite(conveyorBelt, HIGH);
        switch (classifyProduct(QRCodeResult)){
            case PRODUCT_A:
              Serial.println("Processing Product A");
              if(digitalRead(line_1) == HIGH){
                controlServo();
              }
              break;
            case PRODUCT_B:
              Serial.println("Processing Product B");
              if(digitalRead(line_2) == HIGH){
                controlServo();
              }
              break;
            case PRODUCT_C:
              Serial.println("Processing Product C");
              if(digitalRead(line_3) == HIGH){
                controlServo();
              }
              break;
            default:
              Serial.println("Unknown Product");
              break;
 

            }
          
      }
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}

void moveStepper(bool direction, int steps){
    digitalWrite(dirPin, direction ? HIGH : LOW);
    for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000); 
  }
}

void controlServo(){
      for (int pos = 0; pos <= 180; pos++) {
        myServo.write(pos);
        delay(15);
    }
   delay(2000);
    for (int pos = 180; pos >= 0; pos--) {
        myServo.write(pos);
        delay(15);
    }
}
Product classifyProduct(const String& data) {
    if (data == "product_A") return PRODUCT_A;
    else if (data == "product_B") return PRODUCT_B;
    else if (data == "product_C") return PRODUCT_C;
    else return PRODUCT_UNKNOWN;
}
void saveCountsToEEPROM() {
    EEPROM.put(0, countA); 
    EEPROM.put(sizeof(int), countB);  
    EEPROM.put(2 * sizeof(int), countC); 
    EEPROM.commit(); 
}

void getERROM(){
    EEPROM.get(0, countA);
    EEPROM.get(sizeof(int), countB);
    EEPROM.get(2 * sizeof(int), countC);
}