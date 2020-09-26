#include "netdb.h"

#include <stdio.h>

#include <esp_wifi.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "simple_wifi.h"
#include "include/leach.h"
#include "esp_log.h"
#include "include/consts.h"
#include <inttypes.h>
#include "esp_event_loop.h"
#include "include/list_wifi.h"


#define MAX_APs 60
DEFINE_DL_LIST(head);

int countRepeat = 0;
int repeat = 1;
char ssidScan[50];

int rssiScan = 0;
int channel = 0;

#define DEFAULT_SCAN_LIST_SIZE 10
static const char *TAG = "scan";
void scan_wifi(wifi_scan_config_t scan_config);
bool getMacAddress(uint8_t baseMac[6]);
bool startsWith(const char *pre, const char *str);
static void perform_scan(void);
static void init_wifi(void);
char baseMacChr[18] = {0};
static bool got_scan_done_event= false;

typedef struct  {
    char ssid [50];
    int channel;
    int rssi;
    char mac[20];
    struct dl_list node;
} WIFI;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_SCAN_DONE:
        got_scan_done_event = true;
        break;
    default:
        break;
    }
    return ESP_OK;
}


WIFI *init(char* ssid,int channel,int rssi,char* mac) {
    WIFI *wifi = malloc(sizeof(WIFI));
	strcpy(wifi->ssid,ssid);
	wifi->channel=channel;
    wifi->rssi = rssi;
	strcpy(wifi->mac,mac);
	return wifi;
}

void display(struct dl_list *list)
{
	WIFI *wifi;
	dl_list_for_each(wifi, list, WIFI, node)
	{
		printf("SSID:%s| channel:%d|, rssi=%d |, mac=%s\n",wifi->ssid,wifi->channel,wifi->rssi,wifi->mac);
	}
}

bool checkExists(struct dl_list *list,char* ssid,char* mac) {
    WIFI *wifi;
	dl_list_for_each(wifi, list, WIFI, node)
	{
        if(strcmp(wifi->mac,mac) == 0 && strcmp(wifi->ssid,ssid)== 0)
            return true;
	}

    return false;
}

void insert(struct dl_list *list,char* ssid,int channel,int rssi, char* mac)
{

	WIFI *wifi_init = init(ssid,channel,rssi,mac);

    if (!checkExists(list,ssid,mac))
	    dl_list_add_tail(list, &wifi_init->node);
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}


bool getMacAddress(uint8_t baseMac[6])
{   
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    if (startsWith(MAC_PREFIX_1,baseMacChr)) {
       return true;
    }
    else if (startsWith(MAC_PREFIX_2,baseMacChr)) {
        return true;
    }  

    return false;
}


int getCH(const  char *ssid) 
{
    strcpy(ssidScan,ssid);
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    ESP_LOGI("Scan Wifi", "Valor eh: %s",ssidScan);
    //global variable to scan_config
    wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 1,
        .show_hidden = true
    };

    scan_wifi(scan_config);

    free(ssidScan);
    return rssiScan;
}

/* Initialise a wifi_ap_record_t, get it populated and display scanned data */
static void perform_scan(void)
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        //ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        //ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        //ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);

        if (getMacAddress(ap_info[i].bssid)) {
            insert(&head,(char *) ap_info[i].ssid, ap_info[i].primary, ap_info[i].rssi,baseMacChr);
        }
    }

    display(&head);
}


/* Initialize Wi-Fi as sta and start scan */
static void init_wifi(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    while (1) {
        if (got_scan_done_event == true) {
            perform_scan();
            got_scan_done_event = false;
            break;
        } else {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
}

void setup_wifi(void) {
    init_wifi();
}