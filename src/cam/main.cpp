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

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

struct TTelemetry_struct {
    int8_t battery;
    int8_t signal;
    int8_t temperature;
};

typedef struct TTelemetry_struct TTelemetry;

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
TTelemetry telemetry;

bool loop_stream = false;
bool flash_on = false;

void getTelemetry() {
    telemetry.battery = 0;  // TODO: Implement battery level
    telemetry.temperature = ((temprature_sens_read() - 32) / 1.8);
    telemetry.signal = WiFi.RSSI();

    // Exibe os dados de telemetria no monitor serial
    Serial.println("[] Telemetry: Battery: " + String(telemetry.battery) + "%\tSignal: " + String(telemetry.signal) + " dBm\tTemperature: " + String(telemetry.temperature) + "°C");

    // Aloque memória para o buffer dinamicamente
    size_t buffer_size = sizeof(uint8_t) + sizeof(TTelemetry);
    uint8_t* buffer = (uint8_t*)malloc(buffer_size);

    if (buffer == NULL) {
        // Lidar com erro de alocação de memória, se necessário
        Serial.println("Error allocating memory for buffer");
        return;
    }

    // Preencha o buffer diretamente
    buffer[0] = 0;

    // Use memcpy para copiar os dados da estrutura para o buffer
    memcpy(buffer + sizeof(uint8_t), &telemetry, sizeof(TTelemetry));

    // Use o buffer conforme necessário
    webSocket.sendBIN(buffer, sizeof(buffer));
    // Libere a memória alocada quando não for mais necessária
    free(buffer);
}
void toggleFlash() {
    if (flash_on) {
        digitalWrite(FLASH_GPIO_NUM, LOW);
        flash_on = false;
    } else {
        digitalWrite(FLASH_GPIO_NUM, HIGH);
        flash_on = true;
    }
}

void liveCam() {
#if DEBUG_TIME
    unsigned long t_start = millis();
#endif

    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
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
            // Send the image
            uint8_t type = 1;
            size_t message_len = 1 + fb->len;
            uint8_t* buffer = (uint8_t*)malloc(message_len);  // Alocar dinamicamente no heap

            if (buffer) {
                buffer[0] = type;  // Inclui o tipo no início da mensagem
                memcpy(&buffer[1], fb->buf, fb->len);

                // Verificar se o envio teve êxito
                if (webSocket.sendBIN(buffer, message_len) == false) {
                    Serial.println("Error sending frame");
                }

                // Liberar a memória alocada
                free(buffer);
            } else {
                Serial.println("Failed to allocate memory for buffer");
            }
        }
    }

#if DEBUG_TIME
    unsigned long t_end_send = millis();
    Serial.printf("Send in %lu ms - ", t_end_send - t_end_capture);
#endif

    // return the frame buffer back to be reused
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

void changeConfigCam(int quality, int framesize) {
    sensor_t* s = esp_camera_sensor_get();
    s->set_quality(s, quality);
    s->set_framesize(s, (framesize_t)framesize);
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
                Serial.println("[] Starting loopStream");
            } else if (strcmp((char*)payload, "stop") == 0) {
                loop_stream = false;
                Serial.println("[] Stopping loopStream");
            } else if (strcmp((char*)payload, "flash") == 0) {
                toggleFlash();
                Serial.println("[] Toggling flash: " + (flash_on ? String("ON") : String("OFF")));
            } else if (strcmp((char*)payload, "ping") == 0) {
                webSocket.sendTXT("pong");
                // webSocket.sendBIN(buffer, sizeof(buffer));
                Serial.println("[] Send: Pong");
            } else if (strstr((char*)payload, "frame#") != NULL) {
                strtok((char*)payload, "#");
                int framesize = atoi(strtok(NULL, "#"));
                int quality = atoi(strtok(NULL, "#"));
                changeConfigCam(quality, framesize);
                Serial.printf("[] Change config: %d %d\n", framesize, quality);
            }
            break;

        case WStype_BIN:
            Serial.printf("[] Binary: %u\n", length);
            break;

        case WStype_PING:  // Pong is automatically sent
            Serial.printf("[#] Ping\n");
            break;
        case WStype_PONG:  // In case you want to manually send a Pong
            Serial.printf("[#] Pong\n");
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
    config.jpeg_quality = 10;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // Iniciação da câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // Configuração da rede WiFi
    WiFi.setSleep(WIFI_PS_NONE);
    // if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    //     Serial.println("Configuration failed.");
    // }
    Serial.println("SSID: " + String(WIFI_SSID) + "\tPASS: " + String(WIFI_PASS));
    // Conexão WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    // Configuração da câmera
    static sensor_t* s = esp_camera_sensor_get();
    // set initial flips
    s->set_hmirror(s, 1);                     // flip it back
    s->set_vflip(s, 1);                       // flip it back
    s->set_brightness(s, 0);                  // -2 to 2
    s->set_contrast(s, 0);                    // -2 to 2
    s->set_saturation(s, 0);                  // -2 to 2
    s->set_special_effect(s, 0);              // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);                    // aka 'awb' in the UI; 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);                    // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);                     // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);               // 0 = disable , 1 = enable
    s->set_aec2(s, 0);                        // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);                    // -2 to 2
    s->set_aec_value(s, 300);                 // 0 to 1200
    s->set_gain_ctrl(s, 1);                   // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);                    // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);                         // 0 = disable , 1 = enable
    s->set_wpc(s, 1);                         // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);                     // 0 = disable , 1 = enable
    s->set_lenc(s, 1);                        // 0 = disable , 1 = enable
    s->set_dcw(s, 1);                         // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);                    // 0 = disable , 1 = enable

    // Get device ID
    uint32_t id = 0;
    for (int i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    String headers = "X-Device-ID: " + String(id) + "\r\nX-Device-Type: ESPCAM";
    Serial.printf("DeviceID = %d\n", id);

    // Iniciação do WebSocket
    webSocket.begin(HOST_ADDR, PORT_ADDR, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setExtraHeaders(headers.c_str());
    webSocket.setReconnectInterval(5000);

    // Conexão local
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Setup finished");
}

unsigned long last_time = 0;
unsigned long last_time_telemetry = 0;
unsigned long current_time;

void loop() {
    webSocket.loop();

    current_time = millis();

    if (current_time - last_time >= 50 && loop_stream) {
        last_time = current_time;
        liveCam();
    }

    current_time = millis();

    if (current_time - last_time_telemetry >= 1000) {
        last_time_telemetry = current_time;
        getTelemetry();
    }
}