/*
 Interface to Sharp GP2Y1010AU0F Particle Sensor
 Program by Christopher Nafis 
 Written April 2012
 
 http://www.sparkfun.com/datasheets/Sensors/gp2y1010au_e.pdf
 http://sensorapp.net/?p=479
 
 Sharp pin 1 (V-LED)   => 5V (connected to 150ohm resister)
 Sharp pin 2 (LED-GND) => Arduino GND pin
 Sharp pin 3 (LED)     => Arduino pin 2
 Sharp pin 4 (S-GND)   => Arduino GND pin
 Sharp pin 5 (Vo)      => Arduino A0 pin
 Sharp pin 6 (Vcc)     => 5V
 */
#include <stdlib.h>

#define HTTP 1
//#define VREF 1

#ifdef HTTP 
#include <SPI.h>
#include <Ethernet.h>

#include <b64.h>
#include <HttpClient.h>

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>

#include <CountingStream.h>
#include <Xively.h>
#include <XivelyClient.h>
#include <XivelyDatastream.h>
#include <XivelyFeed.h>

// Ethernet settings (MAC only, IP configured via DHCP)

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;

// Xively settings

#define FEED_ID 15932205
#define FEED_API_KEY "DY6pJh3wnmu7SKQjquSUyfNIGvg5C2uYVe9QdtEbrpLV0azR"
 
XivelyClient xivelyclient(client);

// Xively feed

char feed1[]     = "voltage"; // raw voltage from sensor (V)
char feed2[]     = "tsp"; // dust density (Total Suspended Particulates) µg/m3 
char feed3[]     = "limit"; // percentage of US 1987 Air Quality Objectives for TSP

XivelyDatastream datastreams[] = {
  XivelyDatastream(feed1, strlen(feed1), DATASTREAM_FLOAT),
  XivelyDatastream(feed2, strlen(feed2), DATASTREAM_FLOAT),
  XivelyDatastream(feed3, strlen(feed3), DATASTREAM_FLOAT),
};
XivelyFeed xivelyfeed(FEED_ID, datastreams, 3 /* number of datastreams */);

// other variables

unsigned long lastConnectionTime = 0;        // last time connected to the server, in milliseconds
const unsigned int postingInterval = 10000;  // data is being sent to Xively.com in 10 second intervals
#endif

// http://www.epd.gov.hk/epd/english/environmentinhk/air/air_quality_objectives/files/Table_of_historical_AQOs_Eng.pdf
#define TSP_LIMIT  260  //  µg/m3

unsigned int dustPin=0;    // main GP2Y10 - provides actual dust readings

#ifdef VREF
int dustPin2=1; // second GP2Y10 - enclosed in hermetic bag that serves as "zero dust" reference
#endif

int ledPower=2;

// wait times taken from https://www.sparkfun.com/datasheets/Sensors/gp2y1010au_e.pdf
int delayTime=280;
int delayTime2=40;
int offTime=9680;

// these are integers returned by Arduino analog ports, so 0-1024
unsigned int analog=0;

#ifdef VREF
int analog2=0;
#endif

// these are sums of analog readings
unsigned long total=0;

#ifdef VREF
int total2=0;
#endif

int i=0;

char s[32];
float minVoltage = 0.9; // "typical" value per GP2Y
#ifdef VREF
float voltage2 = 0.0;
#endif
float dustdensity = 0.0;
float limit = 0.0;

int http_status;

void setup(){
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(ledPower,OUTPUT);
#ifdef HTTP
  // give the ethernet module time to boot up
  delay(1000);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(;;); // endless loop if no network
  } else {
    Serial.println("Got IP");
  }
#endif
  i=0;
  total =0;
  Serial.println("Starting sensor reads...");
}

void loop(){
  // the sensor is being read much faster than we are able to report to Xively.com
  // because of this we are adding up all readings from the interval (10 sec)
  // and calculating average when ready to send
  i=i+1;
  // start measurements
  digitalWrite(ledPower, LOW); // power on the LED
  delayMicroseconds(delayTime);
  
  analog=analogRead(dustPin); // read the Arduino encoded voltage
  
  #ifdef VREF
  analog2=analogRead(dustPin2);
  #endif
  
  total = total+analog;
  
  #ifdef VREF
  total2 = total2+analog2;
  #endif
  
  delayMicroseconds(delayTime2);
  digitalWrite(ledPower, HIGH); // turn the LED off
  delayMicroseconds(offTime);

  if((millis() - lastConnectionTime > postingInterval)) {
    
    // calculate average analog readings for the measurement period
    unsigned int analogAvg = total/i;
    
    // convert from Arduino units to voltage (4.9 mV/unit)
    float voltage = analogAvg * 0.0049;
    
    // GP2Y10 can return between 0 and 1.5V (!) at *no dust*
    // so assuming a static voltage at no dust will lead to misleading results
    // the code will initially assume the no dust voltage is 0.9 ("typical")
    // and then adaptatively reduce it if the sensor returns smaller values
    // the value of 0.0V is an exception as the sensor tends to return it on initial
    // reads and it doesn't seem to be actual reading
    // Reference: https://www.sparkfun.com/datasheets/Sensors/gp2y1010au_e.pdf page 4
    Serial.print("raw voltage=");
    Serial.println(voltage);
    if(voltage>0.0 && voltage<minVoltage) {
      minVoltage = voltage;
      Serial.print("Resetting minVoltage to ");
      Serial.println(minVoltage);
      
    }
   
    // compensate for GP2Y10 reference voltage
    voltage = voltage - minVoltage;

    #ifdef VREF
    voltage2 = (total2/i) * 0.0049;
    #endif
    
    // convert from GP2Y10 voltage to dust density (µg/m3)
    // sensitivity is 0,5 V per 100 µg/m3
    dustdensity = 200*voltage;
    
    // fix GP2Y10 inaccuracy   
    if (dustdensity < 0 )
      dustdensity = 0;
     
    // calculate density as percentage of allowed US limit
    limit = (100*dustdensity) / TSP_LIMIT;  
    
    // the serial output is purely for debugging

    String dataString = "i=";
    dataString += dtostrf(i, 9, 2, s);
    dataString += " analogAvg=";
    dataString += dtostrf(analogAvg, 9, 0, s);
    dataString += " voltage=";
    dataString += dtostrf(voltage, 9, 4, s);
    dataString += "V minVoltage=";
    dataString += dtostrf(minVoltage, 9, 4, s);
    dataString += "V TSP=";
    dataString += dtostrf(dustdensity, 9, 4, s);
    dataString += " mcg/m3 limit=";
    dataString += dtostrf(limit, 8, 2, s);
    dataString += "%";  

    i=0;
    
    // reset total counters
    total=0;
    
    #ifdef VREF
    total2=0;
    #endif

#ifdef HTTP
    datastreams[0].setFloat(voltage);
    datastreams[1].setFloat(dustdensity);
    datastreams[2].setFloat(limit);
    
    // actually send data to Xively
    http_status = xivelyclient.put(xivelyfeed, FEED_API_KEY);
    
    dataString += dtostrf(http_status, 4, 0, s);
    #endif

    Serial.println(dataString);
    
    lastConnectionTime = millis();
  }

}



















