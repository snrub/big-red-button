/**
 * BigRedButton
 * Quick and dirty sketch to hit a URL when a button is pressed.
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

int buttonState = 0;
int requestMade = 0;

WiFlySerial WiFly(ardRxPin, ardTxPin); 

void setup() 
{
  // initialise card
    delay(2000);
    Serial.begin(9600);
    Serial.println("THE BUTTON");
    
    pinMode(ledPin, OUTPUT);
    // we'll keep the LED off until we can press the button
    digitalWrite(ledPin, LOW);

    WiFly.begin();
   //WiFly.reboot();
    // get MAC address
    Serial << F("MAC: ") << WiFly.getMAC(bufRequest, REQUEST_BUFFER_SIZE) << endl;

    // set our auth options
    WiFly.setAuthMode( WIFLY_AUTH_WPA2_PSK );
    WiFly.setJoinMode( WIFLY_JOIN_AUTO );
    WiFly.setDHCPMode( WIFLY_DHCP_ON );
    // use our expensive external antenna
    WiFly.setUseExternalAnt(WIFLY_EXTERNAL_ANT_ON);

    Serial << F("Attempting to join network '") << wifiSSID << F("'...") << endl;
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
        }
        else
        {
            Serial << F("Association with ") << wifiSSID << F(" failed.") << endl;
        }
    }
    else
    {
            Serial << F("Network interface is not available") << endl;
    }
}

void loop() 
{
    // read the state of the button
    buttonState = digitalRead(buttonPin);
    // HIGH == pressed
    if (buttonState == HIGH) 
    { 
        Serial.println("button on");
        // only make one request per press
        if(requestMade == 0) 
        {
            requestMade = 1;
            // make the request
            request();
        }
        else
        {
            Serial.println("request already made");
        }
    } 
    else 
    {
        requestMade = 0;
        //Serial.println("button off");
    }
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

        while (    TimeOut > millis() && WiFly.isConnectionOpen() ) 
        {
            if (    WiFly.available() > 0 ) 
            {
                // display
                Serial << (char) WiFly.read();
            }
        }

        // Force close connection
        WiFly.closeConnection();
        
        // force a 2 second delay
        delay(2000);
        // we're done - switch the LED back on
        digitalWrite(ledPin, HIGH);
    } 
    else 
    {
        // Failed to open connection
        Serial << F("Failed to open connection to") << serverHost << endl;
        // force a 2 second delay
        delay(2000);
        digitalWrite(ledPin, HIGH);
    }

    WiFly.setDebugChannel( NULL );
    return 0;
}

