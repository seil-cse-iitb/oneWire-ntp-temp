# One Wire Temperature Sensor with NTP

DS18B20 is One Wire Temperature sensor which is integrated with Arduino Mega. The Arduino Mega is also connected to LAN shield in order to send data over LAN using MQTT protocol.

### Working:
The setup is deployed in SIC 213. There are 12 DS18B20 connected to a single pin of Arduino Mega. All these sensors have their own addresses which are set at factory and cannot be changed and are used to distinguish between the sensors. 

The arduino requests for data from all the sensors based on their address, and get the data from them serially. Once it gets the data, it acquires current time using NTP and creates a data structure to be sent over MQTT. The data structure format is as follows:

sensor_id | timestamp | temperature | battery_voltage

## One Wire Sensor Connections with Arduino
![One Wire Sensor Connection with Arduino](one_wire_sensor_bb.png)