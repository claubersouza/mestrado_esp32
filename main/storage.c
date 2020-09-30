
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "include/storage.h"
#include <esp_wifi.h>


char *prefix = "/spiffs/";
static const char *basePath = "/spiffs";
static const char *TAG = "mestrado:";


void spiff_init() {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = basePath ,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

void writeFile(char* file, char* msg) {
    FILE* f = openFile(file,"w");
    fprintf(f, msg);
    fclose(f);
    ESP_LOGI(TAG, WRITE_FILE_SUCCESS);
}

void readFile(char* file) {
    // Open renamed file for reading
    ESP_LOGI(TAG, READ_FILE_MSG);
    FILE* f = openFile(file,"r");

    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}

void closeFile()  {
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}



FILE* openFile(char* file, char* mode) {

    ESP_LOGI(TAG, OPEN_FILE_MSG);
    ESP_LOGI(TAG,"%s",prefix);
    ESP_LOGI(TAG,"%s", file);
    const char* path = strcat("/spiffs/","teste.txt");
    ESP_LOGI(TAG,"%s",path);
    FILE* f = fopen(path, mode);
    if (f == NULL) {
        ESP_LOGE(TAG, OPEN_FILE_MSG_ERROR);
        return false;
    }
    //free(path);
    return f;
}

bool renameFile(char* fileOld,char* fileNew) {
    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return false;
    }
    
    return true;
}

void deleteFile(char* file) {

}