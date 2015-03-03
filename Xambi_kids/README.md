XAmbi Kids
----------------
By Frank Herrmann (xpixer@gmail.com)
<br/>
XAmbi Network with Atmel Attiny and RFM12B

The XAmbi_Kids are sensors with an Attiny84 and a Transceiver RFM12B or for a cheaper solutuion with a infrared send led. 

Features:

* small power consumtion. i.e. send every 5min a packet, run 1 year with a CR2032 battery cell
* send via serial with 2400 baud/s
* range up to ~15m in house
* range up to ~80m over the air
* cheap solution
* send via IR LED if you want and a revceiver exists
* small footprint (3x5cm)
* sensorboard with optional booster (1.5V => 3.3V or 5V)


Hardware:

A xambi kid has 2 PCB's, they are stacked together. I call this [XAmbi-Mainboard](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_mainboard) and [XAmbi-Sensorboard](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_sensorboard). 
They are designed for a plastik enclouser [PDF Datasheet](https://cdn-reichelt.de/documents/datenblatt/C700/DS_1551G.pdf), they are standard and very cheap ~$2:

http://www.reichelt.de/Kunststoffgehaeuse-HAMMOND/1551GBK/3/index.html?ACTION=3&GROUPID=5502&ARTICLE=121020&OFFSET=16&WKID=0&

We have to only drill a hole for the antenna.


