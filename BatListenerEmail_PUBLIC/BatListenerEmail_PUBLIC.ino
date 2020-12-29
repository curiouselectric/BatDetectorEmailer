
//  ***************** CheeseBoard ************************************
//  ***************** Bat Listener ***********************************
//
//  This code was:
//  Written by: Matt Little
//  Date: 28/12/2020
//  But is based on lots of other peoples examples!
//  This code is open and can be shared freely.
//  Contact:  hello@curiouselectric.co.uk
//
// This is a basic code for linking the bat listener to the internet!
// It will check the output from the bat listener
// If the output is greater than a trigger frequency then the unit
// will send a detection email.
// Email will only be sent after 5 mins of inactivity
// Email will contain the start and end times (from NTP server)
//
// If using Arduino IDE: Must use "NodeMCU 0.9 (ESP-12 Module)"

/*
  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

  You need to include the following libraries, via the library manager in Arduino
    WiFiManager (v 0.15.0) by tzapu
    Adafruit_NeoPixel by Adafruit
    Button2 by Lennart Hennings
    EMailSender by XReef
*/

// ************** External Libraries ************************************
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <EMailSender.h>          // https://github.com/xreef/EMailSender

#include <TimeLib.h>
#include <WiFiUdp.h>

// ********** For the RGB LEDS (Neopixels) *************************
#include <Adafruit_NeoPixel.h>

// *********** For the BUTTONS AND ENCODERS *************************
#include "Button2.h"; //  https://github.com/LennartHennigs/Button2
#include "ESPRotary.h";


// *********** Local headers ****************************************
// Config.h includes all the hardware and defines for the board
#include "Config.h"
#include "wifi.h";
#include "leds.h";

Adafruit_NeoPixel pixels(RGBLED_COUNT, RGBLED_DATA_PIN , RGBLED_TYPE);
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

/******************** Sketch Code ************************************/
ESPRotary r = ESPRotary(ROT_A_PIN2, ROT_B_PIN2, (int)4);
Button2 b = Button2(ROT_PUSH_PIN2);

long unsigned int old_millis;
volatile long unsigned int bat_pulses = 0;
long unsigned int bat_detections = 0;
long unsigned int detection_counter = 0;
bool bat_detected_flag = false;
int counter = 0;  // For the display

time_t prevDisplay = 0; // when the digital clock was displayed

String email_reciever = "YOUR RECIPIENT EMAIL";
String email_sender = "YOUR SENDER EMAIL";
String email_sender_password = "YOUR SENDER EMAIL PASSWORD";

// Checks if pulse detected and increments the counter
ICACHE_RAM_ATTR void detectPulse() {
  bat_pulses++;
}

void setup()
{
  Serial.begin(115200);
  // Sort out the LEDs - Use them to show when bat detected?
  pixels.begin();
  for (int i = 0; i < MAX_PIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
  }
  pixels.show();   // Send the updated pixel colors to the hardware.

  // Init the RotaryInput object
  r.setChangedHandler(rotate);
  b.setLongClickHandler(click);

  // Sort out Wifi
  setup_wifi();

  // Sort out NTP
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  getNtpTime(Udp);  // This usually fails!

  old_millis = millis();
  // https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/
  // Want a trigger for the bat pulses
  attachInterrupt(digitalPinToInterrupt(BAT_TRIGGER), detectPulse, RISING);
}

void loop()
{
  // Check the input encoder and button:
  r.loop();
  b.loop();

  // Want to check the ISR counter every second or so
  if (millis() > old_millis + SAMPLE_TIME)
  {
    // This is where we check the frequency
    float freq = ((bat_pulses * 1000.0 * (float)PULSES_TO_FREQ)  / (float)SAMPLE_TIME);
    if (freq > MIN_FREQ)
    {
      // Print out the line for debug!
      Serial.print("Count: ");
      Serial.print(bat_pulses);
      Serial.print(" Freq: ");
      Serial.print(freq);
      Serial.println("Hz");

      bat_detected_flag = HIGH;
      counter = 0;
      detection_counter = millis();    // Reset the detection counter
      bat_detections++;
      for (int i = 0; i < MAX_PIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 255, 0));
      }
      pixels.show();
    }
    else if (bat_detected_flag == HIGH)
    {

      if (counter > 10)
      {
        // Here we do a count down for the email send:
        Serial.print("Countdown to Email: ");
        Serial.println(((detection_counter + DETECTION_WINDOW) - millis())/1000.0);
        counter = 0;
      }
      else
      {
        for (int i = 0; i < MAX_PIXELS; i++)
        {
          pixels.setPixelColor(i, pixels.Color(0, 0, 125, 0));
        }
        pixels.show();
      }
      counter++;
    }
    else
    {
      for (int i = 0; i < MAX_PIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
      }
      pixels.show();
    }
    bat_pulses = 0; // Reset
    old_millis = millis();
  }

  if (millis() > (detection_counter + DETECTION_WINDOW) && bat_detected_flag == HIGH)
  {

    time_t my_time = getNtpTime(Udp);
    // If we are here then we need to send an email with the interetsing info
    String message = "Bat detected at ";
    message = message + returnTime(my_time);
    message = message + " Count: " + (String)bat_detections;
    Serial.println(message);

    // Give a brief flash to show email sent...
    for (int i = 0; i < MAX_PIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0, 0));
    }
    pixels.show();

    // Send email here:
    send_email(email_sender, email_sender_password, email_reciever, "Bat Detected!", message);

    bat_detections = 0;
    bat_detected_flag = LOW;
    bat_pulses = 0;           // Reset to stop multiple sends
  }

  // NTP display clock
  if (timeStatus() != timeNotSet)
  {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }

}

// ****** ENCODER & BUTTON FUNCTIONS *****************
void rotate(ESPRotary & r)
{
  Serial.println(r.directionToString(r.getDirection()));
  Serial.print("Position: ");
  Serial.println(r.getPosition());
  if (r.directionToString(r.getDirection()) == "RIGHT")
  {
    for (int i = 0; i < MAX_PIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0, 0));
    }
  }
  else if (r.directionToString(r.getDirection()) == "LEFT")
  {
    for (int i = 0; i < MAX_PIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0, 0));
    }
  }
  pixels.show();
}

// long click of button
void click(Button2 & btn)
{
  Serial.println("Button Press");
  send_email(email_sender, email_sender_password, email_reciever, "Bat Detected!", "Bat detected at 12:00 on 1/2/20");

  for (int i = 0; i < MAX_PIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
  }
  pixels.show();
}
