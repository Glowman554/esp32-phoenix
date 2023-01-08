#include <stdio.h>

#define SILENT

#include <phoenix/cpu_core.h>
#include <spiffs.h>
#include <wifi.h>
#include <prog_sv.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "driver/gpio.h"

static const char* TAG = "phoenix";

uint8_t memory_[0xffff] = { 0 };
const esp_partition_t* boot_partition;

uint8_t cpu_fetch_byte(uint16_t addr)
{
	// silent(debugf("fetching byte at 0x%x", addr));
	return memory_[addr];
}

void cpu_write_byte(uint16_t addr, uint8_t val)
{
	// silent(debugf("writing byte 0x%x at 0x%x", val, addr));
	memory_[addr] = val;
}

uint8_t cpu_io_read(uint16_t addr)
{
	// silent(debugf("reading byte from io at 0x%x", addr));
    return 0;
}

#define OUT_CLK 5
#define OUT_LATCH 18
#define OUT_DATA 19

void cpu_io_serial_out(uint8_t val)
{
    gpio_set_level(OUT_LATCH, 0);
    
    for (int i = 0; i < 8; i++)
    {
        gpio_set_level(OUT_DATA, val & (1 << i));
        
        gpio_set_level(OUT_CLK, 1);
        gpio_set_level(OUT_CLK, 0);
    }

    gpio_set_level(OUT_LATCH, 1);
}

void cpu_io_out_init()
{
    ESP_LOGI(TAG, "(output) io serial out @ %d(clk), %d(latch), %d(data)", OUT_CLK, OUT_LATCH, OUT_DATA);

    gpio_reset_pin(OUT_CLK);
    gpio_set_direction(OUT_CLK, GPIO_MODE_OUTPUT);
    gpio_reset_pin(OUT_LATCH);
    gpio_set_direction(OUT_LATCH, GPIO_MODE_OUTPUT);
    gpio_reset_pin(OUT_DATA);
    gpio_set_direction(OUT_DATA, GPIO_MODE_OUTPUT);

    cpu_io_serial_out(0);
}


void cpu_io_write(uint16_t addr, uint8_t val)
{
    switch (addr) {
        case 0x0:
                // gpio_set_level(io_out_map[i], val & (1 << i));
            cpu_io_serial_out(val);
            break;
        default:
            silent(debugf("invalid io address %x", addr));
            return;
    }
    // silent(debugf("writing byte 0x%x to io at 0x%x", val, addr));
}

void wifi_task()
{
    ESP_LOGI(TAG, "connecting to wifi...");
    wifi_init_sta();

    init_prog_sv();
    vTaskDelete(NULL);
}

bool pxhalt = false;

void px_save()
{
    // FILE* boot = fopen("/spiffs/boot.bin", "wb");
    // rewind(boot);
    // fwrite(memory_, sizeof(memory_), 1, boot);
    // fclose(boot);

    ESP_ERROR_CHECK(esp_partition_erase_range(boot_partition, 0, 0xffff + 1));
    ESP_ERROR_CHECK(esp_partition_write(boot_partition, 0, memory_, 0xffff));
}

void run_task()
{
    cpu_state_t state = { 0 };
    while (cpu_tick(&state) && !pxhalt)
    {
    #if 0
        silent({
            char out[0xff] = { 0 };
		    cpu_dbg(&state, out);
		    debugf("%s", out);
        });
    #endif
        // taskYIELD();
        // esp_task_wdt_reset();
    #if 0
        vTaskDelay(10 / portTICK_PERIOD_MS);
    #endif
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskCreate(wifi_task, "wifi", 0x4000, NULL, 1, NULL);

    spiffs_mount();

    boot_partition = esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, "boot");
    ESP_LOGI(TAG, "partition %s @ %p is %d bytes big", boot_partition->label, (void*) boot_partition->address, (int) boot_partition->size);
    ESP_ERROR_CHECK(esp_partition_read(boot_partition, 0, memory_, 0xffff));

    // FILE* boot = fopen("/spiffs/boot.bin", "r+");
    // if (boot == NULL)
    // {
    //     ESP_LOGE(TAG, "could not open /boot.bin");
    //     return;
    // }

    // fseek(boot, 0, SEEK_END);
    // int sz = ftell(boot);
    // fseek(boot, 0, SEEK_SET);

    // ESP_LOGI(TAG, "boot rom size: %d bytes", sz);
    
    // fread(memory_, sz, 1, boot);
    // fclose(boot);

    cpu_io_out_init();

    xTaskCreate(run_task, "run", 0x4000, NULL, 1, NULL);
}
