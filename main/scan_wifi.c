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
#include "include/list_wifi.h"


#define MAX_APs 60
DEFINE_DL_LIST(head);

int countRepeat = 0;
int repeat = 1;
char ssidScan[50];

int rssiScan = 0;
int channel = 0;


void scan_wifi(wifi_scan_config_t scan_config);
void next_channel(void);
bool getMacAddress(uint8_t baseMac[6]);
bool startsWith(const char *pre, const char *str);
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

    if (!checkExists(list,ssid,mac))
	    dl_list_add_tail(list, &wifi_init->node);
}


void scan_wifi(wifi_scan_config_t scan_config) {
    printf("Começando Escanear Redes Wi-Fi Canal:%d\n",channel);

    esp_wifi_scan_start(&scan_config, true);
    
    uint16_t apCount = 0;

    esp_wifi_scan_get_ap_num(&apCount);
    
    wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);

    esp_wifi_scan_get_ap_records(&apCount,list);

    printf("Total de lista: %d\n",apCount);
 
	for(int i = 0; i < apCount; i++){
        printf("Valor SSID: %s -> Canal: %d -> Valor RSSI:%d\n",(char *) list[i].ssid,list[i].primary,list[i].rssi);
        if (getMacAddress(list[i].bssid)) {
            insert(&head,(char *) list[i].ssid,list[i].primary,list[i].rssi,baseMacChr);
        }
        printf("\n");

        vTaskDelay(100);
    }

    free(list);
    free(apCount);
    vTaskDelay(100);

    next_channel();
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

void next_channel(void) {

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_start();
    
    if (channel <= 11) {
        channel++;
            // configure and run the scan process in blocking mode
	    wifi_scan_config_t scan_config = {
            .ssid = 0,
            .bssid = 0,
            .channel = channel,
            .show_hidden = true
        };

        scan_wifi(scan_config);
    }
    else
    {
        esp_wifi_stop();
        vTaskDelay(1000);
        channel = 0;
        esp_wifi_start();
        display(&head);
    }
}

void config_wifi(void) {

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    //global variable to scan_config
    wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 1,
        .show_hidden = true
    };

   scan_wifi(scan_config);

}

void setup_wifi(void) {
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    //global variable to scan_config
    wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 1,
        .show_hidden = true
    };

    scan_wifi(scan_config);
}