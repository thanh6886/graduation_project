#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "quirc.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <stdio.h>





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


// const char* ssid = "TP-Link_1E09";
// const char* password = "63718140";

const char* ssid = "bethu";
const char* password = "88888888";

const char* mqtt_server = "5becba34c368460ba7657c804a6e4eed.s2.eu.hivemq.cloud";
const char* mqtt_username = "bé_thu";
const char* mqtt_password = "Thanh2412";
const int mqtt_port = 8883;

WiFiClientSecure esp_client;
PubSubClient client(esp_client);


static const char* root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


void MQTT_Connected();
void dumpData(const struct quirc_data *data);
void setup_esp32_cam();
void ReceiveUART(void *pvParameters);
void QRCodeReader( void * pvParameters );

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
  Serial.println("Initializing...");
  WiFi.begin(ssid, password);
  while (WiFi.status()!= WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Kết nối với mạng Wi-Fi...");
  }
  Serial.println("Đã kết nối với mạng Wi-Fi.");
  esp_client.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while(!client.connected()){
    MQTT_Connected();
    delay(1000);
  }

  setup_esp32_cam();
  xSemaphoreQRScan = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(QRCodeReader, "QRCodeReader_Task", 2048, NULL, 1, &QRCodeReader_Task,1);
  xTaskCreatePinnedToCore(ReceiveUART,"ReceiveUART_Task", 2048, NULL, 1, &ReceiveUART_Task, 1);
  Serial.println("setup done");
}

void loop(){
}




void ReceiveUART(void *pvParameters) {
  while(mySerial.available()){
    String receivedStauts = mySerial.readStringUntil('\n');
      if(receivedStauts == "start"){
        receivedStauts = "";
        Serial.println("TASK QRCODE Render");
        vTaskDelay();
        xSemaphoreGive(xSemaphoreQRScan);
        vTaskResume(QRCodeReader_Task);
        vTaskSuspend(ReceiveUART_Task);
      }
      if(receivedStauts == "L1"){
          Serial.println("L1");
      }
      if(receivedStauts == "L2"){
          Serial.println("L2");
      }
      if(receivedStauts == "L3"){
          Serial.println("L3");
      }
      if(receivedStauts == "L4"){
          Serial.println("L4");
      }
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
  config.xclk_freq_hz = 10000000;
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
  Serial.println("setup cam done");
}

void dumpData(const struct quirc_data *data){ 
  Serial.println("QRCODE OK");
  QRCodeResult = (const char *)data->payload;
  client.publish("data", QRCodeResult.c_str());
  Serial.println(QRCodeResult);
  mySerial.print(QRCodeResult);
  mySerial.print("\n");
}

void MQTT_Connected(){
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESPCient", mqtt_username, mqtt_password)) {
      Serial.println("Connected MQTT.");
    } else {
      Serial.println("Try again.");
    }
}
void callback(char* topic, byte* payload, unsigned int length){}

