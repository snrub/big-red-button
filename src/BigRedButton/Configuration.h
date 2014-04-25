/**
 * BigRedButton
 * Quick and dirty sketch to hit a URL when a button is pressed
 *
 * @author: Julian Lloyd-Phillips <jules@tasticl.es>
 * @date: 2012-08-12
 * @license: MIT
 * @link: https://github.com/snrub/big-red-button
 */
 
#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

// details of the WiFi network to join
char wifiSSID[] = "yourWifiSSID";
char wifiPassphrase[] = "yourWifiPassphrase";


// endpoint details - the webservice to hit when the button is pressed
char serverHost[] = "api.myhost.com";
char serverGetUrl[] = "/button/press.php?buttonId=1";


// HTTP Version
char httpVersion[] = "1.1";

#endif
