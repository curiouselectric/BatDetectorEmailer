#pragma once

void setup_wifi();
void send_email(String my_email_sender, String my_email_password, String my_email_recipient, String my_subject, String my_message);

time_t getNtpTime(WiFiUDP My_Udp);
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address, WiFiUDP My_Udp);
String returnTime(time_t my_time);
