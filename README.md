# Air quality monitoring with Arduino

These are Arduino prototypes for monitoring air quality (mostly particulates) with various sensors available on the market. Main motivation is to monitor air quality in Krakow, my home town in Poland, which is long known for notorious exceeding of air quality limits due to large usage of coal and rubbish for heating by the local population ([Krakow combats smog menace](http://www.thenews.pl/1/6/Artykul/152431,Krakow-combats-smog-menace)).

The town is monitoring pollution levels but the monitoring stations are few and not really located in the worst areas. This effort is an attempt to build a cheap pollution monitor that could be deployed by volunteers in a wide range of locations across the city. Even if the data provided is not really scientific or industrial quality, it can be still very useful to see that, for example, *"today it's 10x more polluted than yesterday"*.

Live data feeds from prototype sensors:

* [Reading, UK](https://xively.com/feeds/448897705) - output from GP2Y10 PPD42NS TP401A

## GP2Y10 PPD42NS TP401A
Arduino Uno reading three sensors through Grove Sensor Shield:

* Sharp [GP2Y10](http://sharp-world.com/products/device/lineup/data/pdf/datasheet/gp2y1010au_e.pdf) - dust sensor; Arudino outputs analog port values (from 0-1024), proportional to sensor's output voltage, which are in turn proportional to particulates concentration; basically, **the less the better**, but this sensor has a non-zero out even at no dust, so expect some positive baseline output at all time;
* Shinyei [PPD42NS](http://sca-shinyei.com/pdf/PPD42NS.pdf) - dust sensor; Arduino outputs percentage of low signal in sampling period (30 seconds) which is a non-linear function of dust concentration; again, **the less the better**, and zero dust means zero percentage;
* Dovelet [TP401A](http://seeedstudio.com/wiki/images/e/eb/TP-401A_Indoor_Air_quality_gas_sensor.pdf) - hazardous gases sensor; Arduino outputs analog port values (0-1024), which are proportional to sensor's resistance, which is inversely proportional to hazardous gases concentration; again, **the less the better**, but as this sensor is basically a resistor, expect some output all the time even when there's no gases;
 

