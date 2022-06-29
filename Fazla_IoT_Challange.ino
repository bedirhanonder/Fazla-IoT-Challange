#include <Nextion.h> 
#include <NextionText.h>
#include <SoftwareSerial.h>
#include "HX711.h"
#define CALIBRATION_FACTOR -459.542
SoftwareSerial nextionSerial(0, 1); // RX, TX pins
NexText weightText = NexText(0, 4, "weightText"); 
HX711 loadcell;
SoftwareSerial mySerial(2, 3); // sim800l_TX_pin, sim800l_RX_pin;
String apn = "internet";                    //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
String url = "..........";  //URL of Server
int counter = 0;

void setup() 
{
  // Load cell amp start-up
  // 1. HX711 circuit wiring
  const int LOADCELL_DOUT_PIN = 6;
  const int LOADCELL_SCK_PIN = 7;
  
  // 2. Adjustment settings
  const long LOADCELL_OFFSET = 50682624;
  const long LOADCELL_DIVIDER = 5895655;
  
  // 3. Initialize library
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_scale(LOADCELL_DIVIDER);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.tare();               // reset the scale to 0

  //Display start-up
  Serial.begin(9600);
  nexInit();

  //Initialize SIM800L module
  Serial.println("SIM800 AT CMD Test");
  mySerial.begin(9600);
  delay(5000);
  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  delay(2000);
  configure_gprs();
}

void loop() 
{
  // 1. Acquire reading
  Serial.print("Weight: ");
  Serial.println(loadcell.get_units(10), 2);
  // 2. Set the reading to a double element
  double acqWeight = loadcell.get_units((10),2);
  // 3. Set the text on display as the reading
  weightText.SetText(acqWeight);
  // 4. Send the data to server
  counter++;
  String data = String(counter); //write your data here
  send_to_server(data);
  delay(2000);
}
void send_to_server( String data) {
  String sendURL = url+"?val="+data;
  Serial.println(" --- Start GPRS & HTTP --- ");
  send_to_serial("AT+SAPBR=1,1");
  send_to_serial("AT+SAPBR=2,1");
  send_to_serial("AT+HTTPINIT");
  send_to_serial("AT+HTTPPARA=CID,1");
  send_to_serial("AT+HTTPPARA=URL," + sendURL);
  send_to_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  send_to_serial("AT+HTTPDATA=192,5000");
  send_to_serial(data);
  send_to_serial("AT+HTTPACTION=1");
  send_to_serial("AT+HTTPREAD");
  send_to_serial("AT+HTTPTERM");
  send_to_serial("AT+SAPBR=0,1");
}
void configure_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  send_to_serial("AT+SAPBR=3,1,Contype,GPRS");
  send_to_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    send_to_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    send_to_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void send_to_serial(String command) {
  Serial.println("Send ->: " + command);
  mySerial.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (mySerial.available()) {
      Serial.write(mySerial.read());
    }
  }
  Serial.println();
}
