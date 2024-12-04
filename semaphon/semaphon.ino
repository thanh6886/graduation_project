#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "quirc.h"

#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


#define RX1 14
#define TX1 15


HardwareSerial mySerial(1);

struct quirc *q = NULL;
camera_fb_t * fb = NULL;
uint8_t *image = NULL;  
struct quirc_code code;
struct quirc_data data;
String QRCodeResult = "";

SemaphoreHandle_t xSemaphoreQRScan;


TaskHandle_t QRCodeReader_Task; 
TaskHandle_t ReceiveUART_Task;



void setup(){
  Serial.begin(115200);
  mySerial.begin(4800, SERIAL_8N1, RX1, TX1); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  setup_esp32_cam();
  xSemaphoreQRScan = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(QRCodeReader, "QRCodeReader_Task", 5000, NULL, 1, &QRCodeReader_Task,1);
  xTaskCreatePinnedToCore(ReceiveUART,"ReceiveUART_Task", 2048, NULL, 1, &ReceiveUART_Task, 1);
  Serial.println("setup done");
}

void loop(){
}

void setup_esp32_cam() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;  
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 15;
  config.fb_count = 1;  


  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
}



void ReceiveUART(void *pvParameters) {
  while(1){
  if(mySerial.available() > 0){
    String receivedStauts = Serial.readStringUntil('\n');
      if(receivedStauts == "start"){
        receivedStauts = "";
        Serial.println("TASK QRCODE Render");
        xSemaphoreGive(xSemaphoreQRScan);
        vTaskResume(QRCodeReader_Task);
        vTaskSuspend(ReceiveUART_Task);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void QRCodeReader( void * pvParameters ){
  if (xSemaphoreTake(xSemaphoreQRScan, portMAX_DELAY) == pdTRUE) {
    while(1){
      fb = esp_camera_fb_get();
      q = quirc_new();
      if (!fb){
        esp_camera_fb_return(fb);
        continue;
      }   
      quirc_resize(q, fb->width, fb->height);
      image = quirc_begin(q, NULL, NULL);
      memcpy(image, fb->buf, fb->len);
      quirc_end(q);
      int count = quirc_count(q);
      if (count > 0) {
        quirc_extract(q, 0, &code);
        if (!quirc_decode(&code, &data)){
          dumpData(&data);
          vTaskResume(ReceiveUART_Task);
          vTaskSuspend(QRCodeReader_Task);
        } 
      } 
      esp_camera_fb_return(fb);
      fb = NULL;
      image = NULL;  
      quirc_destroy(q); 
    }      
  }
}

void dumpData(const struct quirc_data *data){ 
  Serial.println("QRCODE OK");
  QRCodeResult = (const char *)data->payload;
  Serial.println(QRCodeResult);
  mySerial.print(QRCodeResult);
  mySerial.print("\n");
}
