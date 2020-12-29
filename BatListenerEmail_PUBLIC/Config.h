#pragma once

// General Function Libraries - Already installed in Arduino IDE
#include <Arduino.h>
#include <stdint.h>
#include <Millis.h>
#include <EEPROM.h>        // For writing values to the EEPROM

const uint8_t   BAT_TRIGGER                = D1;    // For software serial

const uint8_t   ROT_A_PIN2                 = D7;
const uint8_t   ROT_B_PIN2                 = D6;
const uint8_t   ROT_PUSH_PIN2              = D8;

const uint8_t   CBOLED_SDA_PIN             = D5;
const uint8_t   CBOLED_SCK_PIN             = D4;

const uint8_t   RGBLED_DATA_PIN            = D3;
const uint8_t   RGBLED_COUNT               = 5;
const uint8_t   RGBLED_BRIGHTNESS          = 50;
#define         RGBLED_TYPE                (NEO_GRB + NEO_KHZ800)

#define         SAMPLE_TIME             100     // millis seconds between checks
#define         MAX_PIXELS              5
#define         PULSES_TO_FREQ          7       // Conversion factor from pulses measured to actual bat frequency. From bat listener circuit
#define         MIN_FREQ                2000    // Minimum frequency in time period to trigger bat sample algorithm

#define         DETECTION_WINDOW        10000   // milli seconds for how long there should be no bats detected before sending an email.

// NTP Servers:
static const char ntpServerName[]     = "pool.ntp.org";
const int         timeZone            = 0;     // UK Time

const int       NTP_PACKET_SIZE         = 48; // NTP time is in the first 48 bytes of message
