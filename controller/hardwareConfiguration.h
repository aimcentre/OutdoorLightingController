// Sensor GPIO pin assignments
// References: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//             https://wiki.keyestudio.com/KS0413_keyestudio_ESP32_Core_Board

#ifndef HARDWARE_SETTINGS
#define HARDWARE_SETTINGS

#define AIN_1 35
#define AIN_2 34
#define AIN_3 39
#define AIN_4 36

#define DIN_1 23
#define DIN_2 22
#define DIN_3 5
#define DIN_4 19
#define DIN_5 18
#define DIN_6 17
#define DIN_7 16
#define DIN_8 15

#define MOTION_A DIN_1
#define MOTION_B DIN_2
#define MOTION_C DIN_3
#define MOTION_D DIN_4
#define MOTION_E DIN_5
#define MOTION_F DIN_6

#define WIFI_RESET 13
#define DAYLIGHT_SENSOR AIN_1

#define TEMP_SENSOR_P AIN_3
#define TEMP_SENSOR_N AIN_4

#define STATUS_R 14
#define STATUS_G 2
#define STATUS_B 4

// Lamp-segment GPIO pin assignments
#define LAMP_PIN_A 27
#define LAMP_PIN_B 26
#define LAMP_PIN_C 25
#define LAMP_PIN_D 33
#define LAMP_PIN_E 32

#endif
