/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME  "BryanOrCas"
#define IO_KEY       "aio_aEMN95enMUgl4ovlRAoBLbO5Ifsc"

/******************************* WIFI **************************************/

#define WIFI_SSID "Bryan"
#define WIFI_PASS "bryanOrt"

// comment out the following lines if you are using fona or ethernet
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
