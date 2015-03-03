XAmbi Kids
----------------
By Frank Herrmann (xpixer@gmail.com)
<br/>
XAmbi Network with Atmel Attiny and RFM12B

![XAmbi_kid_dht11](https://github.com/xpix/XAmbi/blob/master/Xambi_kids/IMG_2490.JPG?raw=true)

The XAmbi_Kids are sensors with an Attiny84 and a Transceiver RFM12B or for a cheaper solutuion with a infrared send led. 

##Features:

* small power consumtion. i.e. send every 5min a packet, run 1 year with a CR2032 battery cell
* send via serial with 2400 baud/s
* range up to ~15m in house
* range up to ~80m over the air
* cheap solution
* send via IR LED if you want and a revceiver exists
* small footprint (3x5cm)
* sensorboard with optional booster (1.5V => 3.3V or 5V)


##Hardware:

A xambi kid has 2 PCB's, they are stacked together. I call this [XAmbi-Mainboard](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_mainboard) and [XAmbi-Sensorboard](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_sensorboard). 
They are designed for a plastik enclouser [PDF Datasheet](https://cdn-reichelt.de/documents/datenblatt/C700/DS_1551G.pdf), they are standard and very cheap ~$2. We have to drill a hole for the antenna and thats all. 

##Software:

- DHT11 [temperature and humidity Sensor](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_dht11)
- Dalles DS18B20 [Temperature Sensor](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_DS18B20)
- [Mailbox Sensor](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_mailbox) via infrared reflection
- Plant [Moisture Sensor](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_moisture)



