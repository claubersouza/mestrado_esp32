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
#include "include/scan_wifi.h"


#define MAX_APs 60
DEFINE_DL_LIST(head);

int countRepeat = 0;
int repeat = 1;
char ssidScan[50];

int rssiScan = 0;
int channel = 0;

#define DEFAULT_SCAN_LIST_SIZE 10
static const char *TAG = "scan";

bool getMacAddress(uint8_t baseMac[6]);
bool startsWith(const char *pre, const char *str);
static void wifi_scan(void);

char baseMacChr[18] = {0};


typedef struct  {
    char ssid [50];
    int channel;
    int rssi;
    char mac[20];
    struct dl_list node;
} WIFI;



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

    if (!checkExists(list,ssid,mac && strcmp(ssidScan,ssid) == 0)){
        rssiScan = rssi;
	    dl_list_add_tail(list, &wifi_init->node);
    }
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
    
    ESP_LOGI("Scan Wifi", "Valor eh:%s",ssidScan);

    wifi_scan();

    esp_wifi_disconnect();
    esp_wifi_stop();
 
    return rssiScan;
}

static void wifi_scan(void)
{
       esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    tcpip_adapter_init();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    
    for(int x = 0; x < 14; x++)
    {
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = x,
            .show_hidden = true,
            .scan_type = WIFI_SCAN_TYPE_ACTIVE,
            .scan_time.active.min = 10,
            .scan_time.active.max = 100        
        };
        ESP_LOGI(TAG, "Start Scan");
        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        uint16_t apCount = 0;
        esp_wifi_scan_get_ap_num(&apCount);
        ESP_LOGI(TAG, "Number of access points found: %d for channel %d", apCount, x);
        if (apCount > 0) 
        {
            wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
            int i;
            ESP_LOGI(TAG, "======================================================================");
            ESP_LOGI(TAG, "             SSID             |    RSSI    |           AUTH           ");
            ESP_LOGI(TAG, "======================================================================");
            for (i=0; i<apCount; i++) {
                char *authmode;
                switch(list[i].authmode) {
                case WIFI_AUTH_OPEN:
                    authmode = "WIFI_AUTH_OPEN";
                    break;
                case WIFI_AUTH_WEP:
                    authmode = "WIFI_AUTH_WEP";
                    break;           
                case WIFI_AUTH_WPA_PSK:
                    authmode = "WIFI_AUTH_WPA_PSK";
                    break;           
                case WIFI_AUTH_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA2_PSK";
                    break;           
                case WIFI_AUTH_WPA_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                    break;
                default:
                    authmode = "Unknown";
                    break;
                }

                if(strcmp(ssidScan,&list[i].ssid) == 0)
                    rssiScan =  list[i].rssi;
                

                ESP_LOGI(TAG, "%26.26s    |    % 4d    |    %22.22s",list[i].ssid, list[i].rssi, authmode);
            }
            free(list);   
        }
    }

}

/* Initialise a wifi_ap_record_t, get it populated and display scanned data */
void perform_scan()
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
        for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
            ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
            ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
            ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);

            if (getMacAddress(ap_info[i].bssid)) {
                insert(&head,(char *) ap_info[i].ssid, ap_info[i].primary, ap_info[i].rssi,baseMacChr);
            }
        }

    display(&head);
}