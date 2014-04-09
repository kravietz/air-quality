/* Grove - Dust Sensor Demo v1.0
 Interface to Shinyei Model PPD42NS Particle Sensor
 Program by Christopher Nafis 
 Written April 2012
 
 http://www.seeedstudio.com/depot/grove-dust-sensor-p-1050.html
 http://www.sca-shinyei.com/pdf/PPD42NS.pdf
 
 JST Pin 1 (Black Wire)  => Arduino GND
 JST Pin 3 (Red wire)    => Arduino 5VDC
 JST Pin 4 (Yellow wire) => Arduino Digital Pin 8
 */

#define pinPPD42NS 8

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;

float ratio = 0;
float concentration = 0;
float pcsPerCF = 0;
float ugm3 = 0;
String out = "";

void setup() {
  Serial.begin(115200);
  Serial.println("#lowpulseoccupancy,ratio,concentration,pcsPerCF,ug/m3");
  pinMode(pinPPD42NS, INPUT);
  
  delay(10000); // warm up
  
  starttime = millis();//get the current time;
}

void loop() {
  // read PPD42NS
  duration = pulseIn(pinPPD42NS, LOW, sampletime_ms);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  
  if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    pcsPerCF = concentration*100;
    ugm3 = pcsPerCF/3000;
   
    out = String(lowpulseoccupancy);
    out += ",";
    out += String(ratio);
    out += ",";
    out += String(concentration);
    out += ",";
    out += String(pcsPerCF);
    out += ",";
    out += String(ugm3);
    
    Serial.println(out);
    // reset counters
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}
