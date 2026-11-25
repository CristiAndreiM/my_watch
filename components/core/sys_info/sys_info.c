#include "sys_info.h"
#include "esp_app_desc.h"
#include "esp_chip_info.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_log.h"

static const char* TAG = "sys_info";

void sys_info_print(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

   ESP_LOGI(TAG, "Version: %s", esp_app_get_description()->version);
   ESP_LOGI(TAG, "Cores: %d, Revision: v%d.%d", chip_info.cores, chip_info.revision, chip_info.revision % 100);
   ESP_LOGI(TAG, "Running partition: %s", running->label);
   ESP_LOGI(TAG, "Free heap: %lu bytes", (unsigned long)esp_get_free_heap_size());
}