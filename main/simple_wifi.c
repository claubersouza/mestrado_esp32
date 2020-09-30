/* Simple WiFi Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "include/udp_server.h"
#include "include/udp_client.h"
#include "include/scan_wifi.h"
#include "include/storage.h"
#include "include/leach.h"

char ptrTaskList[250];


TaskHandle_t Handle = NULL; 
eTaskState statusOf;

static void count_down_init(void *pvParameters);
static void check_socket(void *pvParameters);

void init_task();


/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_MODE_AP   true //TRUE:AP FALSE:STA
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       CONFIG_MAX_STA_CONN

#define CONFIG_ESP_WIFI_SSID_STA "Clai2.4"
#define CONFIG_FREERTOS_USE_TRACE_FACILITY 1

#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS 1

bool switchConn = false;
bool switchServer = false;
bool staconn = false;


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

///void count_down_init();

/* The event group allows multiple bits for each event,-
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

//void tcp_server_task(void *pvParameters);

void init_task(void) 
{

    xTaskCreate(initLeach, "leach", 4096, NULL, 5, NULL);
    //xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
    //xTaskCreate(count_down_init,"count_down",1024,NULL,2,NULL);
    //xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 1, NULL);
    //xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 1, NULL);
   // xTaskCreate(initLeach, "random", 2048, NULL, 5, NULL);  

    //xTaskCreate(tcp_server_task,"tcp_server",4096,NULL,1,&Handle);

    //xTaskCreate(check_socket,"count_down",1024,NULL,3,NULL);

   // vTaskStartScheduler();

   //while (1);
    
    
}


static const char *TAG = "simple wifi";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;
    
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);

        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        got_scan_done_event = true;
        break;   
    default:
        break;
    }
    return ESP_OK;
}


static void count_down_init(void *pvParameters) {
    int i = 0;

    while (i < 3) {
        ESP_LOGI(TAG, "Count Down:%d",i);
        vTaskDelay(200);
        i++;
    }
       vTaskDelete(NULL);
}

static void check_socket(void *pvParameters) {      
    while (1) {
      statusOf =  eTaskGetState(Handle);
        ESP_LOGI(TAG,"Estado: %d\n",statusOf);
        vTaskDelay(1000);
        if (statusOf == 3) {
            vTaskDelay(500);
           vTaskResume(Handle);

        }
    }

    vTaskDelete(NULL);
}

void wifi_init_scan() {
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    wifi_event_group = xEventGroupCreate();
    /* Initialize Wi-Fi as sta and start scan */
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_LOGI(TAG,"WIFI SCAN START");
    while (1) {
        if (got_scan_done_event == true) {
            perform_scan();
            ESP_LOGI(TAG,"WIFI SCAN DONE");
            got_scan_done_event = false;
            
            break;
        }else {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
}

void wifi_init_softap()
{
    
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    
    if (!switchConn)
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    switchConn = false;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

uint8_t* getMacAddressWifi() {
    uint8_t l_Mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, l_Mac);
    return l_Mac;
}


void wifi_cont_sta()
{
    
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    switchConn = false;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID_STA,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             CONFIG_ESP_WIFI_SSID_STA, EXAMPLE_ESP_WIFI_PASS);

        
}

void wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    
    if (!switchConn)
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    switchConn = false;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID_STA,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             CONFIG_ESP_WIFI_SSID_STA, EXAMPLE_ESP_WIFI_PASS);
}


void switch_conn_task() {
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    switchConn = true;

    if (staconn) {
        wifi_init_softap();
        staconn = false;
    }
    else {
        wifi_init_sta();
        staconn = true;
    }
}

void switch_client_server_task() {

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
   // xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 1, NULL);

}

void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //init_task();
    wifi_init_sta();
    init_task();
    /*
    spiff_init();
    writeFile("teste.txt","Ola");
    vTaskDelay(1000);
    readFile("teste.txt");
    
    staconn = true;
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    setup_wifi();
   */ 

   // wifi_init_sta();
  //  init_task();
  //  setup_wifi();
    //initLeach();
    //wifi_init_sta();
    //init_task();
}

