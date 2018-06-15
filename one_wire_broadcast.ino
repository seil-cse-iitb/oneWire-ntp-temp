#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include <EthernetUdp.h>
#include <NTPClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define sensorPin 22
#define TOPIC_HEADER "data/kresit/ds18/"

#define SLEEP_TIME 60   // Sleep time is seconds

//#define SERIAL_PRINT 1  // Comment this to remove the Serial.print from the code(Do it once the program is finalize)
#ifdef SERIAL_PRINT
#define _SER_BEGIN(x) Serial.begin(x)
#define _SER_PRINT(x) Serial.print(x)
#define _SER_PRINTLN(x) Serial.println(x)
#define _DELAY(x) delay(x)
#else
#define _SER_BEGIN(x)
#define _SER_PRINT(x)
#define _SER_PRINTLN(x)
#define _DELAY(x)
#endif

// A unique MAC ID has to be assigned for the Ethernet Module
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01
};

const char* mqtt_server = "10.129.149.9";

const char* mqtt_username = "<MQTT_BROKER_USERNAME>";
const char* mqtt_password = "<MQTT_BROKER_PASSWORD>";
const char* client_id = "oneWireTempNode_213";

char mqtt_topic_temp[35];

char address[16];

EthernetClient ethClient;
PubSubClient client(ethClient);

IPAddress ip(10, 129, 149, 70);
//IPAddress gateway(10, 129, 149, 250);
//IPAddress subnet(255, 255, 255, 0);

EthernetUDP udp;
NTPClient ntpClient(udp);

OneWire oneWire(sensorPin);
DallasTemperature sensor(&oneWire);

int sensorCount = 0;
bool gotAddress = false;

unsigned long count = 0;
unsigned long ntpValue = 0;
unsigned long prevNtpValue = 0;
unsigned int sleep_count = 0;

DeviceAddress sensorAddress[20];
unsigned long serialNo[20];

// Function to initialize timer interrupt for ONE second
void timerInterrupt()
{
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 3036;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

// interrupt service routine that wraps a user defined function
// supplied by attachInterrupt
ISR(TIMER1_OVF_vect)
{
  TCNT1 = 3036;            // preload timer
  count++;
  ntpValue++;
  sleep_count++;
}

// Connects to MQTT if there is a broken pipe
void reconnect()
{
  while (!client.connected())
  {
    _SER_PRINT("Connecting to MQTT....");

    if (client.connect(client_id))//, mqtt_username, mqtt_password))
    {
      _SER_PRINTLN("Connected");;
    }
    else
    {
      _SER_PRINT("Failed to connect");
      //_SER_PRINTLN(client.state());
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  _SER_BEGIN(115200);
  _SER_PRINTLN("____ONE WIRE BROADCAST____");

  Ethernet.begin(mac, ip);

  ntpClient.begin();
  ntpClient.update();
  ntpValue = ntpClient.getEpochTime();

  client.setServer(mqtt_server, 1883);

  sensor.begin();

  sensorCount = sensor.getDeviceCount();

  _SER_PRINT("Number of sensors are ");
  _SER_PRINTLN(sensorCount);

  for (int index = 0; index < sensorCount; index++)
  {
    sensor.getAddress(sensorAddress[index], index);
  }

  timerInterrupt();
}

void loop() {
  // put your main code here, to run repeatedly:

  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

  sensor.requestTemperatures();

  if (ntpValue != prevNtpValue)
  {
    String addressString;
    String tempString;
    String ntpString;
    String mqttString;
    char mqttData[50];

    prevNtpValue = ntpValue;

    //_SER_PRINTLN("-------------------------------");

    if (count == 600)
    {
      _SER_PRINTLN("---------------------------");
      ntpClient.update();

      unsigned long currNtpVal = ntpClient.getEpochTime();
      if ((currNtpVal < (ntpValue + 2)) || (currNtpVal > (ntpValue - 2)))
      {
        _SER_PRINTLN("No update");
      }
      else
      {
        _SER_PRINTLN("Updated");
        ntpValue = currNtpVal;
      }
      _SER_PRINTLN(ntpValue);

      _SER_PRINTLN("---------------------------");

      count = 0;
    }

    if (sleep_count == SLEEP_TIME)
    {
      for (int index = 0; index < sensorCount; index++)
      {
        for (int indexByte = 0; indexByte < 8; indexByte++)
        {
          if (indexByte == 0)
          {
            addressString = String(sensorAddress[index][indexByte], HEX);
            addressString += '-';
          }
          else if (indexByte != 1)
          {
            if (sensorAddress[index][8 - indexByte] < 16) addressString += '0';

            addressString += String(sensorAddress[index][8 - indexByte], HEX);
          }
        }

        serialNo[index]++;
        tempString = String(sensor.getTempC(sensorAddress[index]));
        mqttString = String(ntpValue) + "," + String(serialNo[index]) + ","  + tempString;    // To keep a count of number of sensors, create an array of counter. The index in the array will point to the count of the measured data.
        _SER_PRINTLN(addressString);
        _DELAY(20);

        addressString.toCharArray(address, 16);

        strcpy(mqtt_topic_temp, TOPIC_HEADER);
        strcat(mqtt_topic_temp, address);

        _SER_PRINTLN(mqtt_topic_temp);

        mqttString.toCharArray(mqttData, 50);
        client.publish(mqtt_topic_temp, mqttData);
      }

      sleep_count = 0;
    }
  }
}
