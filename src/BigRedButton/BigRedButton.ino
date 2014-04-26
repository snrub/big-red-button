/**
 * BigRedButton
 * Quick and dirty sketch to connect to a wifi network, and request a URL when a button is pressed.
 * The LED is used to indicate status - if the light is lit, then the button is ready to be pressed.
 *
 * @todo: The aim of this version is to allow you to change the wifi settings and webservice endpoint
 *        without having to dismantle the button and plug it into the computer.
 *        If the button is depressed and held as the button is switched on, an adhoc network will be
 *        created. If you join that adhoc network, there should be some way of showing the current
 *        settings and changing them if desired.
 *        The changes should be maintained after power-down and reboot, so some kind of persistent
 *        storage is required. The WiFly module provides some basic storage and an interface to change
 *        settings, so it might be possible to use those.
 *
 * @author: Julian Lloyd-Phillips <jules@tasticl.es>
 * @date: 2012-08-12
 * @license: MIT
 * @link: https://github.com/snrub/big-red-button
 */

#include <Arduino.h>
#include <Streaming.h>
#include <PString.h>
#include <SoftwareSerial.h>
#include <WiFlySerial.h>
#include "Configuration.h"

#define REQUEST_BUFFER_SIZE 180
#define BODY_BUFFER_SIZE 100

char bufRequest[REQUEST_BUFFER_SIZE];
char bufBody[BODY_BUFFER_SIZE];

// pin details
const int buttonPin = 3;
const int ledPin =    5;
const int ardTxPin = 7;
const int ardRxPin = 8;
const int adhocPin = 16;

int buttonState = 0;
int requestMade = 0;

boolean wifiReady = false;

// initialise card
WiFlySerial WiFly(ardRxPin, ardTxPin);

void setup() 
{
    Serial.begin(9600);
    delay(1000);
    Serial.println("THE BUTTON");
    
    pinMode(ledPin, OUTPUT);
    // we'll keep the LED off until we can press the button
    digitalWrite(ledPin, LOW);
    // don't go into adhoc mode yet
    digitalWrite(adhocPin, LOW);

    WiFly.begin();
    // get MAC address
    Serial << F("MAC: ") << WiFly.getMAC(bufRequest, REQUEST_BUFFER_SIZE) << endl;

    // if the button is pressed on start
    // skip the normal setup routine, and create
    // an adhoc network instead
    if(digitalRead(buttonPin) == HIGH)
    {
      Serial << F("Button is pressed. Setting up adhoc network") << endl;
      createAdHoc();
    }
    else
    {
      Serial << F("No button pressed - attempting association") << endl;
      //wifiJoin();
      //Serial << F("SSID: ") << WiFly.getSSID(bufRequest, REQUEST_BUFFER_SIZE) << endl;
      
      WiFly.SendCommand("get wlan ssid",">", bufBody, BODY_BUFFER_SIZE);
      Serial << F("NEWSSID: ") << bufBody  << endl;
    }
}


void loop() 
{
  
  // only execute the loop if wifi is available
  if(wifiReady == 1)
  {
    // read the state of the button
    buttonState = digitalRead(buttonPin);
    // HIGH == pressed
    if (buttonState == HIGH) 
    {
        // only make one request per press
        if(requestMade == 0) 
        {
            requestMade = 1;
            // make the request
            request();
        }
        else
        {
            //Serial.println("request already made");
        }
    } 
    else 
    {
        requestMade = 0;
        //Serial.println("button off");
    }
  }
  else
  {
    // slow flash
    flashLed(1, 2000);
    // attempt reconnect
    // wifiJoin();
  }
}


void wifiSetAuth()
{
    // set our auth options
    WiFly.setAuthMode( WIFLY_AUTH_WPA2_PSK );
    WiFly.setJoinMode( WIFLY_JOIN_AUTO );
    WiFly.setDHCPMode( WIFLY_DHCP_ON );
    // use our expensive external antenna
    WiFly.setUseExternalAnt(WIFLY_EXTERNAL_ANT_ON);
}

boolean wifiJoin()
{
    Serial << F("Attempting to join network '") << wifiSSID << F("'...") << endl;

    // set out auth options
    wifiSetAuth();

    // open link, if we don't already have one
    WiFly.getDeviceStatus();
    if (! WiFly.isifUp() ) 
    {
        WiFly.leave();
        // join
        WiFly.setSSID(wifiSSID) ;
        WiFly.setPassphrase(wifiPassphrase);
        if ( WiFly.join() ) 
        {
            Serial << F("We associated with ") << wifiSSID << F(" successfully.") << endl;
            // disable default *hello* bullshit
            WiFly.SendCommand("set comm remote 0",">", bufBody, BODY_BUFFER_SIZE);
            memset (bufBody,'\0',BODY_BUFFER_SIZE);
        
            // clear out prior requests.
            WiFly.flush();
            while (WiFly.available())
                WiFly.read();

            // we're ready for the button - switch the LED on
            digitalWrite(ledPin, HIGH);
            wifiReady = true;
            return true;
        }
        else
        {
            Serial << F("Association with ") << wifiSSID << F(" failed.") << endl;
            wifiReady = false;
            return false;
        }
    }
    else
    {
        Serial << F("Network interface is not available") << endl;
    }
    wifiReady = false;
    return false;
}


// make an HTTP GET request
int request() 
{
    Serial.println("making request");
    char bufRequest[REQUEST_BUFFER_SIZE];

    // Build request:
    //" GET /v1.0/button HTTP/1.1\nHost: api.host.com\nConnection: close\n\n\n"
    PString strRequest(bufRequest, REQUEST_BUFFER_SIZE);
    strRequest << F("GET ") << serverGetUrl 
        << F("HTTP/") << httpVersion << "\n"
        << F("Host: ") << serverHost << "\n"
        << F("Connection: close") << "\n"
        << "\n\n";

    // busy - switch the LED off
    digitalWrite(ledPin, LOW);

    // Open connection, then send GET Request
    Serial << F("Opening connection to ") << serverHost << endl;
    if (WiFly.openConnection( serverHost ) ) 
    {
        
        WiFly <<    strRequest << endl; 

        // buffer server response
        unsigned long TimeOut = millis() + 4000;

        while ( TimeOut > millis() && WiFly.isConnectionOpen() )
        {
            if ( WiFly.available() > 0 ) 
            {
                // display
                Serial << (char) WiFly.read();
            }
        }

        // Force close connection
        WiFly.closeConnection();
        // we're done - switch the LED back on
        digitalWrite(ledPin, HIGH);
    }
    else
    {
        // Failed to open connection
        Serial << F("Failed to open connection to ") << serverHost << endl;
    }

    WiFly.setDebugChannel( NULL );
    // stop jitter
    delay(100);
    return 0;
}


void createAdHoc()
{
  Serial << F("Entering Ad Hoc mode... ");
  flashLed(40, 100);
  digitalWrite(adhocPin, HIGH);
  Serial << F("Pin high, rebooting...");
  WiFly.getDeviceStatus();
  WiFly.reboot();
  // use our expensive external antenna
  WiFly.setUseExternalAnt(WIFLY_EXTERNAL_ANT_ON);
  Serial << F("Back from reboot");
  delay(1000);
  flashLed(50, 100);
  digitalWrite(adhocPin, LOW);
}


void flashLed(int flashes, int frequency)
{
  for(int i=0;i<flashes;i++)
  {
    digitalWrite(ledPin, HIGH);
    delay (frequency/2);
    digitalWrite(ledPin, LOW);
    delay (frequency/2);
  }
}

