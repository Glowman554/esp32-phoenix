#include <spiffs.h>

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "esp_log.h"
#include "esp_spiffs.h"

static const char* TAG = "spiffs";

void spiffs_list_files(const char* base)
{
    char path[256] = { 0 };

    DIR *dir = opendir(base);

    if (!dir)
    {
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            strcpy(path, base);
            strcat(path, "/");
            strcat(path, dp->d_name);

            ESP_LOGI(TAG, "content: %s", path); 

            spiffs_list_files(path);
        }
    }

    closedir(dir);
}

void spiffs_mount()
{
    esp_vfs_spiffs_conf_t conf =
    {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

     if (ret != ESP_OK)
     {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0;
    size_t used = 0;

    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    spiffs_list_files("/spiffs");
}
