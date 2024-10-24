

TaskHandle_t ReceiveUART_Task;
TaskHandle_t ControlMain_Task;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  xTaskCreatePinnedToCore(ReceiveUART, "ReceiveUART_Task", 256, NULL, 1, &ReceiveUART_Task, 0);
  xTaskCreatePinnedToCore(ControlMain, "ControlMain_task", 512, NULL, 1, &ControlMain, 0);
}

void ReceiveUART(void *pvParameters){

}

void ControlMain(void *pvParameters){

}

void loop() {
  // put your main code here, to run repeatedly:

}
