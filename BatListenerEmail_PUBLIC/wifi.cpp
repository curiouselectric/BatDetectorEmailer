#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EMailSender.h>          // https://github.com/xreef/EMailSender

#include <TimeLib.h>
#include <WiFiUdp.h>

#include "wifi.h";
#include "Config.h"

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(180);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();  // Reset of cannot connect - got to AP mode
    delay(5000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void send_email(String _email_sender, String _email_password, String _email_recipient, String _subject, String _message)
{
  char email_sender_char[_email_sender.length() + 1];
  _email_sender.toCharArray(email_sender_char, _email_sender.length() + 1);
  char email_pass_char[_email_password.length() + 1];
  _email_password.toCharArray(email_pass_char, _email_password.length() + 1);


  Serial.print("Info:" );
  Serial.print(email_sender_char);
  Serial.print(" : ");
  Serial.println(email_pass_char);

  EMailSender emailSend(email_sender_char, email_pass_char);

  EMailSender::EMailMessage message;

  message.subject = _subject; //"Button has been pressed";
  message.message = _message; // "Hello from your ESP8266";

  EMailSender::Response resp = emailSend.send(_email_recipient, message);

  Serial.println("Sending status: ");

  Serial.println(resp.status);
  Serial.println(resp.code);
  Serial.println(resp.desc);
}


void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

time_t getNtpTime(WiFiUDP _Udp)
{
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  IPAddress ntpServerIP; // NTP server's ip address

  while (_Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP, _Udp);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = _Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      _Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address, WiFiUDP _Udp)
{
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  _Udp.beginPacket(address, 123); //NTP requests are to port 123
  _Udp.write(packetBuffer, NTP_PACKET_SIZE);
  _Udp.endPacket();
}

String returnTime(time_t _time)
{
  struct tm timeinfo;
  
  localtime_r(&_time, &timeinfo);

  String timeDate = (String)(timeinfo.tm_year + 1900);
  if ((timeinfo.tm_mon + 1) < 10)
  {
    timeDate += "-0" + (String)(timeinfo.tm_mon + 1);
  }
  else
  {
    timeDate += "-" + (String)(timeinfo.tm_mon + 1);
  }
  if (timeinfo.tm_mday < 10)
  {
    timeDate += "-0" + (String)(timeinfo.tm_mday);
  }
  else
  {
    timeDate += "-" + (String)(timeinfo.tm_mday);
  }
  timeDate += " " + (String)(timeinfo.tm_hour);
  timeDate += ":" + (String)(timeinfo.tm_min);
  timeDate += ":" + (String)(timeinfo.tm_sec);
  
  return (timeDate);
}
