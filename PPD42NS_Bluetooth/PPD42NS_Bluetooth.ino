#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 3
#define dustPin 8

/* 
Scaling http://www.metone.com/particulate-aero531.php
Concentration Range	0 – 1000 ug/m3
Concentration Range	0 – 3,000,000 particles per cubic foot (105,900 particles/L)

PPD42NS
Detectable range of concentration 0~28,000 pcs/liter (0~8,000pcs/0.01 CF=283ml)

1 CF = 28.32 L
1 CF = 0.02832 m3
1 m3 = 35.32 CF
detectable range = 800'000 pcs/CF
detactable range = 270 ug/m3

initial PPD42NS output (curve) is in pcs/0.01cf
convert to pcs/cf -> pcsPerCF = concentration*100
convert to ug/m3 -> ugm3 = pcsPerCF/3000
*/

// Bluetooth serial
SoftwareSerial BT(rxPin, txPin);

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float pcsPerCF = 0;
float ugm3 = 0;
bool client_connected = 0;
String ret = "";

String BTread() {
  String s = "";
  if (BT.available() > 0) {
    while (BT.available() > 0) {
      s += (char)BT.read();
      delay(3);
    }
  }
  return s;
}

String BTcmd(String s) {
  s += "\r\n";
  Serial.print(s);
  BT.print(s);
  delay(500);
  return BTread();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  // dust sensor at D8
  pinMode(dustPin, INPUT);

  BT.begin(38400);
  ret = BTcmd("AT+INIT");
  Serial.print(ret);
  
  if(ret.startsWith("ERROR")) {    
    Serial.print(BTcmd("AT+RESET"));
  }
  
  if(ret.length() == 0) {
    Serial.println("Client seems to be already connected");
    client_connected = true;
  } else {
  Serial.print(BTcmd("AT+ROLE=0")); // slave - accept connections
  Serial.print(BTcmd("AT+CLASS=0")); // device class
  Serial.print(BTcmd("AT+SENM=0,0")); // enable encryption & auth
  Serial.print(BTcmd("AT+CMODE=1")); // allow all to connect
  Serial.print(BTcmd("AT+RMAAD"));
  Serial.print(BTcmd("AT+STATE?"));
  Serial.print(BTcmd("AT+ADCN?"));
  Serial.print(BTcmd("AT+INQM=1,9,48")); // configure HOW we are discoverable
  Serial.print(BTcmd("AT+INQ")); // actually BECOME discoverable 
  }
  
  Serial.println("Init completed...");
}

void loop()
{

  ret = BTread();
  if (ret.length() > 0) {
    Serial.print(ret);
    if (ret.startsWith("+DISC")) {
      Serial.println("Got disconnect");
      Serial.println(BTcmd("AT+RESET"));
      Serial.println(BTcmd("AT+STATE?"));
      client_connected = false;
    }
    if (ret.startsWith("OK")) {
      Serial.println("Got client");
      client_connected = true;
      starttime = millis();
    }
  }

  if (client_connected == true) {
    duration = pulseIn(dustPin, LOW);
    lowpulseoccupancy = lowpulseoccupancy + duration;

    if ((millis() - starttime) > sampletime_ms) //if the sampel time == 30s
    {
      ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
      concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve, in pcs/0.01cf
      pcsPerCF = concentration*100;
      ugm3 = pcsPerCF/3000;
      
      starttime = millis();
      String output = String("lowpulseoccupancy=");
      output += lowpulseoccupancy;
      output += " ratio=";
      output += ratio;
      output += " conc=";
      output += concentration;
      output += " pcs/0.01cf ";
      output += ugm3;
      output += "ug/m3";
      Serial.println(output);
      BT.println(output); // we don't expect an answer
      lowpulseoccupancy = 0;
    }
  }

}
