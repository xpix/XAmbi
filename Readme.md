XAmbi Network
----------------
By Frank Herrmann (xpixer@gmail.com)
<br/>
XAmbi Network with Atmel Attiny and RFM12B

##Synopsis
The XAmbi Network system are a bunch of Sensor nodes and a Raspberry Gateway. This can control everything in your house and get the data in a nice Webinterface.
This is the Hardware section, here the Software part: [Webinterface](https://github.com/xpix/xambiserver) with MQTT Support. 

What do you need for the first start are follow components:
- a [RaspberryPi](http://en.wikipedia.org/wiki/Raspberry_Pi) with installed [git](http://en.wikipedia.org/wiki/Git_(software))
- a [Jeenode](http://jeelabs.net/projects/hardware/wiki/JeeNode) connected via USB or Serial to RPI
- one or more [xambi sensor](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_mainboard) nodes with an RFM12B Transceiver

##Features
- the xambi network system are very modular
- one Gateway with max 999 nodes
- very low bit rate for stable and long range communicaton via [Jeelib](http://jeelabs.net/pub/docs/jeelib/)
- sensor board a kind of breadboard to connect sensors very easy
- Boost to 3.3V on sensor board to get all energy from battery cell
- mainboard very small and standard sizes for normal enclousures

##License
GPL 3.0. See License.txt file.
