#include <Arduino.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

#include "dl_lib.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

// Configuração da rede WiFi
const char* ssid = "ROBOT-1B:C0";
const char* password = "robo1234";

// Set your Static IP address
IPAddress staticIP(192, 168, 4, 10);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
#define PART_BOUNDARY "123456789000000000000987654321"

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

WebSocketsServer webSocket = WebSocketsServer(81);
uint8_t cam_num;
bool connected = false;

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

bool flash_led = false;

static esp_err_t parse_get(httpd_req_t* req, char** obuf) {
    char* buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

static esp_err_t streamToggleFlash(httpd_req_t* req) {
    flash_led = !flash_led;
    digitalWrite(FLASH_GPIO_NUM, flash_led);
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t streamChangeFrameSize(httpd_req_t* req) {
    char* buf = NULL;
    char variable[32];
    char value[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK || httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int val = atoi(value);
    sensor_t* s = esp_camera_sensor_get();
    int res = 0;

    if (!strcmp(variable, "framesize")) {
        if (s->pixformat == PIXFORMAT_JPEG) {
            Serial.print("Framesize : ");
            Serial.println(val);
            res = s->set_framesize(s, (framesize_t)val);
        }
    }

    if (res < 0) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t streamIndex(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    String html = "<html><head><title>ESP32-CAM</title></head><body>";
    httpd_resp_send(req, html.c_str(), html.length());
    return ESP_OK;
}

void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t toggle_uri = {
        .uri = "/toggle",
        .method = HTTP_GET,
        .handler = streamToggleFlash,
        .user_ctx = NULL};

    httpd_uri_t frame_size_uri = {
        .uri = "/framesize",
        .method = HTTP_GET,
        .handler = streamChangeFrameSize,
        .user_ctx = NULL};

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = streamIndex,
        .user_ctx = NULL};

    // Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &toggle_uri);
        httpd_register_uri_handler(stream_httpd, &frame_size_uri);
        httpd_register_uri_handler(stream_httpd, &index_uri);
    }
}

void liveCam(uint8_t num) {
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t* _jpg_buf = NULL;
    camera_fb_t* fb = esp_camera_fb_get();

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
        webSocket.sendBIN(num, fb->buf, fb->len);
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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            Serial.printf("[%u] Connected", num);
            cam_num = num;
            connected = true;
            break;
        case WStype_TEXT:
        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void setup() {
    Serial.begin(115200);

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
    config.xclk_freq_hz = 8000000;
    config.pixel_format = PIXFORMAT_JPEG;
    // Low(ish) default framesize and quality
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // Iniciação da câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    WiFi.setSleep(WIFI_PS_NONE);

    if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
        Serial.println("Configuration failed.");
    }

    // Conexão WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    static sensor_t* s = esp_camera_sensor_get();
    // set initial flips
    s->set_hmirror(s, 1);    // flip it back
    s->set_vflip(s, 1);      // flip it back
    s->set_gain_ctrl(s, 1);  // enable gain control (auto gain)
    s->set_lenc(s, 1);       // enable lens correction

    // Início da transmissão no servidor Web
    startCameraServer();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.print("Camera Stream Ready! Go to: http://");
    Serial.print(WiFi.localIP());

    pinMode(FLASH_GPIO_NUM, OUTPUT);
}

void loop() {
    webSocket.loop();
    if (connected == true) {
        liveCam(cam_num);
    }
}