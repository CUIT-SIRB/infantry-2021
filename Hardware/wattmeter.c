#include "wattmeter.h"

wattmeter_t wattmeter_data;

void wattmeter_update_data(uint8_t *data) {
    uint16_t *data_u16     = (uint16_t *)data;
    wattmeter_data.voltage = data_u16[0] / 100.0F;
    wattmeter_data.current = data_u16[1] / 100.0F;
    wattmeter_data.power   = wattmeter_data.voltage * wattmeter_data.current;
}
