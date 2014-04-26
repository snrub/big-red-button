/**
 * BigRedButton
 * Quick and dirty sketch to connect to a wifi network, and request a URL when a button is pressed.
 * The LED is used to indicate status - if the light is lit, then the button is ready to be pressed.
 *
 * Wifi credentials and the URL that should be requested should be defined in
 * Configuration.h
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

boolean wifiReady = false;

// initialise the WiFly card
WiFlySerial WiFly(ardRxPin, ardTxPin); 

/**
 * Startup sequence
 *  - get the mac address of the wifi card
 *  - join the wifi network
 */
void setup() 
{
    Serial.begin(9600);
    delay(2000);
    Serial.println("THE BUTTON");

    // we'll keep the LED off until we can press the button
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // initialise a session with the wifi card
    WiFly.begin();

    // get MAC address
    Serial << F("MAC: ") << WiFly.getMAC(bufRequest, REQUEST_BUFFER_SIZE) << endl;

    // and join our network
    Serial << F("No button pressed - attempting association") << endl;
    wifiJoin();
}

/**
 * Main loop - waiting for a button press
 * Indicates status through the LED:
 *  - solid LED: network available and button ready
 *  - slow flashing: network unavailable
 */
void loop()
{
    // only execute the loop if wifi is available
    if(wifiReady == 1) {

        // read the state of the button
        buttonState = digitalRead(buttonPin);

        // HIGH == pressed
        if (buttonState == HIGH) {

            // if a request is in progress, ignore
            // additional button presses
            if(requestMade == 0) {

                // make the request
                requestMade = 1;
                request();

            } else {
                //Serial.println("request already made");
            }

        } else {
            requestMade = 0;
            //Serial.println("button off");
        }

    } else {
        // slow flash
        flashLed(1, 2000);
        // attempt reconnect
        // wifiJoin();
    }
}

/**
 * Set the connection parameters for the wifi card
 */
void wifiSetParams()
{
    // set our auth options
    WiFly.setAuthMode( WIFLY_AUTH_WPA2_PSK );
    WiFly.setJoinMode( WIFLY_JOIN_AUTO );

    // enable DHCP
    WiFly.setDHCPMode( WIFLY_DHCP_ON );

    // use our expensive external antenna
    WiFly.setUseExternalAnt(WIFLY_EXTERNAL_ANT_ON);
}

/**
 * Join a wifi network
 */
boolean wifiJoin()
{
    Serial << F("Attempting to join network '") << wifiSSID << F("'...") << endl;

    // set out auth options
    wifiSetParams();

    // open link, if we don't already have one
    WiFly.getDeviceStatus();
    if (! WiFly.isifUp()) {

        // leave any joined networks
        WiFly.leave();

        // join our network
        WiFly.setSSID(wifiSSID) ;
        WiFly.setPassphrase(wifiPassphrase);

        // check that we managed to join
        if (WiFly.join()) {

            Serial << F("We associated with ") << wifiSSID << F(" successfully.") << endl;

            // disable default *hello* bullshit
            WiFly.SendCommand("set comm remote 0",">", bufBody, BODY_BUFFER_SIZE);
            memset (bufBody,'\0',BODY_BUFFER_SIZE);

            // clear out prior requests.
            WiFly.flush();
            while (WiFly.available()) {
                WiFly.read();
            }

            // we're ready for the button - switch the LED on
            digitalWrite(ledPin, HIGH);
            wifiReady = true;

        } else {
            // could not join network
            Serial << F("Association with ") << wifiSSID << F(" failed.") << endl;
            wifiReady = false;
        }

    } else {
        Serial << F("Network interface is not available") << endl;
        wifiReady = false;
    }

    return wifiReady;
}

/**
 * Make an HTTP GET request
 */
int request() 
{
    Serial.println("making request");
    char bufRequest[REQUEST_BUFFER_SIZE];

    // Build request string. Request will look like:
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
    if (WiFly.openConnection(serverHost)) {

        // send the request string to the server
        WiFly << strRequest << endl;

        // buffer server response
        unsigned long TimeOut = millis() + 4000;

        while (TimeOut > millis() && WiFly.isConnectionOpen()) {
            if (WiFly.available() > 0) {
                // display response
                Serial << (char) WiFly.read();
            }
        }

        // Force close connection
        WiFly.closeConnection();

    } else  {
        // Failed to open connection
        Serial << F("Failed to open connection to ") << serverHost << endl;
    }

    // force a 2 second delay
    delay(2000);

    // we're done - switch the LED back on
    digitalWrite(ledPin, HIGH);

    WiFly.setDebugChannel( NULL );
    return 0;
}

/**
 * Flash the status LED
 */
void flashLed(int flashes, int frequency)
{
    for(int i=0;i<flashes;i++) {
        digitalWrite(ledPin, HIGH);
        delay (frequency/2);
        digitalWrite(ledPin, LOW);
        delay (frequency/2);
    }
}
