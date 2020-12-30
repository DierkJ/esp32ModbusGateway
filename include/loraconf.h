#ifdef HAS_LORA
/************************************************************
 * LMIC LoRaWAN configuration
 *
 * Read the values from TTN console (or whatever applies), insert them here,
 * and rename this file to src/loraconf.h
 *
 * Note that DEVEUI, APPEUI and APPKEY should all be specified in MSB format.
 * (This is different from standard LMIC-Arduino which expects DEVEUI and APPEUI
 * in LSB format.)
 *
 * NOTE: Use MSB format (as displayed in TTN console, so you can cut & paste
 * from there)
 * For TTN, APPEUI in MSB format always starts with 0x70, 0xB3, 0xD5
 *
 * Note: Exchange the static definition later with JSON Config from SPIFFS
 *
 ************************************************************/

/*
** 20201230: hahismbgw01 in TTN
*/
static const u1_t DEVEUI[8] = { 0x00, 0x4A, 0x6D, 0xFF, 0x55, 0x00, 0x30, 0x05 };

/*
** TTN App: hahispm  HahisPowerMeter App
*/
static const u1_t APPEUI[8] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x03, 0xAA, 0x9F };

static const u1_t APPKEY[16] = { 0xC3, 0x3F, 0x39, 0x1B, 0x62, 0xF5, 0x20, 0xEC, 
                                 0x92, 0xC0, 0xF5, 0x62, 0xEC, 0x88, 0x00, 0xC0 };


#endif // HAS_LORA

