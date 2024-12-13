void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(task2, "task2", 2048, NULL, 1, NULL,0);
  Serial.println("setup done");
}

void loop() {
 
}





