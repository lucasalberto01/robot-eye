#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

#include "config.h"
#include "dl_lib.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

struct TMe_struct {
    char ssid[48];
    char password[48];
    short mode;
    char hostname[48];
    char serialNumber[6];
    int port;
};

typedef struct TMe_struct TMe;

TMe me;

// Configuração da rede WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// Set your Static IP address
IPAddress staticIP(192, 168, 4, 10);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

// Configuração do modelo de câmera (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define LED_GPIO_NUM 33

#define FLASH_GPIO_NUM 4

WebSocketsClient webSocket;
TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void* pvParameters);
void Task2code(void* pvParameters);

bool loop_stream = false;

void liveCam() {
#if DEBUG_TIME
    unsigned long t_start = millis();
#endif

    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t* _jpg_buf = NULL;
    camera_fb_t* fb = esp_camera_fb_get();

#if DEBUG_TIME
    unsigned long t_end_capture = millis();
    Serial.printf("Capture in %lu ms - ", t_end_capture - t_start);
#endif

    if (!fb) {
        Serial.println("Frame buffer could not be acquired");
        res = ESP_FAIL;
    } else {
        if (fb->format != PIXFORMAT_JPEG) {
            Serial.println("STREAM: Non-JPEG frame returned by camera module");
            res = ESP_FAIL;
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
    }
    if (res == ESP_OK) {
        // replace this with your own function
        if (webSocket.sendBIN(fb->buf, fb->len) == false) {
            Serial.println("Error sending frame");
        }
        Serial.println("Frame length: " + String(fb->len / 1024) + " Kbytes");

#if DEBUG_TIME
        unsigned long t_end_send = millis();
        Serial.printf("Send in %lu ms - ", t_end_send - t_end_capture);
        Serial.printf("Signal RSSI: %d dBm\n", WiFi.RSSI());
#endif
    }

    // return the frame buffer back to be reused
    if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
    } else if (_jpg_buf) {
        free(_jpg_buf);
        _jpg_buf = NULL;
    }
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[] Disconnected!\n");
            loop_stream = false;
            break;
        case WStype_CONNECTED:
            Serial.printf("[] Connected\n");
            break;
        case WStype_TEXT:
            Serial.printf("[] Text: %s\n", payload);
            if (strcmp((char*)payload, "start") == 0) {
                loop_stream = true;
                Serial.println("Starting loopStream");
            } else if (strcmp((char*)payload, "stop") == 0) {
                loop_stream = false;
                Serial.println("Stopping loopStream");
            }

            break;
        case WStype_BIN:
            Serial.printf("[] Binary: %u\n", length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            Serial.printf("[] Error\n");
            break;
    }
}

void setup() {
    // Iniciação da porta serial
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial2.begin(9600, SERIAL_8N1, 13, 14);

    // Iniciação do pino do flash
    pinMode(FLASH_GPIO_NUM, OUTPUT);

    // Configuração da câmera
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
    config.xclk_freq_hz = 20000000;  // 20MHz
    config.pixel_format = PIXFORMAT_JPEG;
    // Low(ish) default framesize and quality
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // Iniciação da câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    Serial.println("Waiting for message from ESP32-CAM");

    bool awaitMessage = true;

    while (awaitMessage) {
        while (Serial2.available() > 0) {
            // Receive TMe struct from ESP32-CAM
            Serial2.readBytes((byte*)&me, sizeof(me));
            awaitMessage = false;
        }
    }

    Serial.println("Setup started");
    Serial.println("Setting up WiFi");
    Serial.println("SSID: " + String(me.ssid));
    Serial.println("Password: " + String(me.password));
    Serial.println("Hostname: " + String(me.hostname));
    Serial.println("Serial Number: " + String(me.serialNumber));
    Serial.println("Port: " + String(me.port));

    // Configuração da rede WiFi
    WiFi.setSleep(WIFI_PS_NONE);
    // if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    //     Serial.println("Configuration failed.");
    // }

    // Conexão WiFi
    WiFi.begin(me.ssid, me.password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Configuração da câmera
    static sensor_t* s = esp_camera_sensor_get();
    // set initial flips
    s->set_hmirror(s, 1);    // flip it back
    s->set_vflip(s, 1);      // flip it back
    s->set_gain_ctrl(s, 1);  // enable gain control (auto gain)
    s->set_lenc(s, 1);       // enable lens correction

    // Iniciação do WebSocket
    webSocket.begin(me.hostname, me.port, "/ws", "ESPCAM");
    webSocket.setExtraHeaders("X-Auth-Token: 123\r\nX-Device-ID: 123\r\nX-Device-Type: CAM\r\n");
    webSocket.onEvent(webSocketEvent);

    // Conexão local
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        Task1code, /* Task function. */
        "Task1",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &Task1,    /* Task handle to keep track of created task */
        0);        /* pin task to core 0 */
    delay(500);

    // create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
    xTaskCreatePinnedToCore(
        Task2code, /* Task function. */
        "Task2",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &Task2,    /* Task handle to keep track of created task */
        1);        /* pin task to core 1 */
    delay(500);

    Serial.println("Setup finished");
}

void Task1code(void* pvParameters) {
    for (;;) {
        vTaskDelay(100);
        yield();
        webSocket.loop();
    }
}

void Task2code(void* pvParameters) {
    for (;;) {
        vTaskDelay(2000);
        yield();
        while (loop_stream) {
            liveCam();
        }
    }
}

void loop() {
    vTaskDelete(NULL);
}