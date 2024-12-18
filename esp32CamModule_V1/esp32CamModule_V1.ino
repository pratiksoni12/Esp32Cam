#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// Replace with your Wi-Fi credentials
const char* ssid = "xxxx";
const char* password = "xxxx";


// Static IP configuration
IPAddress local_IP(192, 168, 1, 101);  // Replace with your desired static IP
IPAddress gateway(192, 168, 1, 1);     // Replace with your network gateway
IPAddress subnet(255, 255, 255, 0);    // Subnet mask



// Server URL
const String serverURL = "http://192.168.1.9:5000/upload"; // Replace <IP> with the actual IP of your Server

// Assign a unique ID to each ESP32-CAM
String deviceID = "Esp1"; // Change to "ESP32-CAM-2" for the second module

// Camera pin configuration for AI-Thinker ESP32-CAM module
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

void setup() {
  Serial.begin(115200);

  //  // Connect to Wi-Fi
  //  WiFi.begin(ssid, password);
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.println("Connecting to WiFi...");
  //  }
  //  Serial.println("WiFi connected");


  // Connect to Wi-Fi with static IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP configuration failed!");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the camera
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
  config.pixel_format = PIXFORMAT_JPEG;

  // Adjust settings based on PSRAM availability
  if (psramFound()) {
    // Lower resolution for smaller image size
    config.frame_size = FRAMESIZE_VGA;   // VGA (640x480)
    config.jpeg_quality = 15;  // Higher number for lower quality (more compression)
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // QVGA (320x240)
    config.jpeg_quality = 20;  // Higher number for lower quality
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  Serial.println("Camera initialized successfully");
}

void loop() {
  // Capture Image
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.printf("Captured image size: %u bytes\n", fb->len);

  // Send Image to RPi
  HTTPClient http;
  http.setTimeout(20000); // Set timeout to 20 seconds
  http.begin(serverURL);

  // Add headers
  http.addHeader("Content-Type", "application/octet-stream");
  http.addHeader("Device-ID", deviceID);

  // Debugging: Sending image
  Serial.println("Sending image to server...");

  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  esp_camera_fb_return(fb); // Release frame buffer

  // Wait for 3 minutes (180000 ms)
  delay(60000); // 3 minutes
}
