#include "esp_log.h"
#include "esp_ota_ops.h"
static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "My Watch - boot");

    const esp_partition_t *running = esp_ota_get_running_partition();
ESP_LOGI(TAG, "running from partition '%s' at 0x%lx", running->label, (unsigned long)running->address);
}
