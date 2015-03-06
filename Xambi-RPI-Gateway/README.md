XAmbi Gateway for Raspberry PI
----------------
By Frank Herrmann (xpixer@gmail.com)
<br/>
RPI Shield with Atmega329P and RFM12B Transceiver 

![XAmbi_gateway](https://github.com/xpix/XAmbi/blob/master/Xambi-RPI-Gateway/IMG_2951.JPG?raw=true)

Here you find all the design for the Shield, on the pic you see the first Prototype. You can also connect a WS2812 LED Strip to install an Ambilight to your TV. But the first function from this board are a kind of [Jeenode](http://www.digitalsmarties.net/products/jeenode). 

* Solder an RF12B Board and Atmega with Quartz and other Components
* burn the fuses over ISCP and an Bootloader (Arduino UNO) to Atmega: [Tutorial](http://playground.arduino.cc/Learning/Burn168)
* Connect the board with an Raspberry PI
* Use the bootloader and burn the [simple receiver](https://github.com/xpix/XAmbi/tree/master/Xambi_kids/xambikid_simplereceiver) to the XAmbi Board
* connect via screen command and check for the "Init ..." text

You can use this Wiki: http://wiki.openenergymonitor.org/index.php?title=RFM12Pi_V2 they describe very good the steps to burn some firmware to an Arduno clone. This "Xambi-Gateway" are a arduino clone at the internal serial Line from Raspberry. 


