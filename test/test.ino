#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <Zbar.h>

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

const char* ssid = "YOUR_SSID";         // Thay thế bằng SSID mạng của bạn
const char* password = "YOUR_PASSWORD"; // Thay thế bằng mật khẩu mạng của bạn

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối đến WiFi...");
  }
  Serial.println("Đã kết nối đến WiFi");

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

  // Đặt độ phân giải
  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA; // Thay đổi độ phân giải nếu cần
    config.jpeg_quality = 12; // Số nhỏ hơn nghĩa là chất lượng cao hơn
    config.fb_count = 2; // Sử dụng hai bộ đệm khung
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Khởi tạo camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Khởi tạo camera thất bại với lỗi 0x%x", err);
    return;
  }
}

void loop() {
  // Chụp hình ảnh
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Chụp camera thất bại");
    return;
  }

  // Xử lý hình ảnh với Zbar để tìm mã QR
  ZbarImage image(fb->width, fb->height, "Y800", fb->buf, fb->len);
  ZbarImageScanner scanner;
  scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
  
  int n = scanner.scan(image);
  if (n > 0) {
    for (ZbarSymbolIterator it = image.symbol_begin(); it != image.symbol_end(); ++it) {
      Serial.printf("Mã QR đã tìm thấy: %s\n", it->get_data().c_str());
    }
  } else {
    Serial.println("Không tìm thấy mã QR");
  }

  esp_camera_fb_return(fb);
  delay(2000); // Điều chỉnh độ trễ nếu cần
}
