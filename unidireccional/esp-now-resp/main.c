#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/Task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "led_strip.h"

#define ESP_CHANNEL 1
//#define LED_STRIP 2  // For esp32WROOM
#define LED_STRIP 48 // For esp32 S3
#define LED_STRIP_MAX_LEDS 1

//static uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
//static esp_now_peer_info_t esp_now_peer_info = {};

led_strip_handle_t led_strip;

static const char * TAG = "esp_now_resp";
static esp_err_t init_wifi(void)
{

    // Initialize NVS flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi init complete");
    return ESP_OK;
}

void recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    ESP_LOGI(TAG, "Data received: " MACSTR " %s",MAC2STR(esp_now_info->src_addr), data);

    uint8_t r_value = data[0];
    uint8_t g_value = data[1];
    uint8_t b_value = data[2];

    r_value = atoi(strtok((char *)data,"|")); //255|0|0
    g_value = atoi(strtok(NULL,"|")); //0|255|0
    b_value = atoi(strtok(NULL,"|")); //0|0|255

    led_strip_set_pixel(led_strip, 0, r_value, g_value, b_value);
    led_strip_refresh(led_strip);

}

void send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    if(status == ESP_NOW_SEND_SUCCESS)
    {
        ESP_LOGI(TAG, "ESP_NOW_SEND_SUCCESS");
    }
    else
    {
        ESP_LOGW(TAG, "ESP_NOW_SEND_FAIL");
    }
}

static esp_err_t init_esp_now(void)
{
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);

    esp_now_register_send_cb(send_cb);

    ESP_LOGI(TAG, "esp now init completed");
    return ESP_OK;
}

/*static esp_err_t register_peer(uint8_t *peer_addr)
{
    memcpy(esp_now_peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = 0;  // Use the current channel that station is on
    esp_now_peer_info.ifidx = WIFI_IF_STA;

    esp_now_add_peer(&esp_now_peer_info);
    return ESP_OK;
}*/

/*static esp_err_t esp_now_send_data(const uint8_t *peer_addr, const uint8_t *data, size_t len)
{
    esp_now_send(peer_addr, data, len);
    return ESP_OK;
}*/

static esp_err_t init_led_strip(void)
{

    /// LED strip common configuration
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP,  // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_MAX_LEDS,                 // The number of LEDs in the strip,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format of the LED strip
        .led_model = LED_MODEL_WS2812, // LED strip model, it determines the bit timing
        .flags.invert_out = false, // don't invert the output signal
    };

    /// RMT backend specific configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // RMT counter clock frequency: 10MHz
        .flags.with_dma = false, // DMA feature is available on chips like ESP32-S3/P4
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(init_esp_now());

    //ESP_ERROR_CHECK(register_peer(peer_mac));
    ESP_ERROR_CHECK(init_led_strip());

}