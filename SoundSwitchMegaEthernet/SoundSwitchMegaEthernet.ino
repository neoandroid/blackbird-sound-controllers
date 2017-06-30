/*
    Copyright 2008 Jordi Miguel, Jose Manuel Calahorra

    This file is part of SoundSwitch.

    SoundSwitch is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SoundSwitch is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SoundSwitch.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <SPI.h>
#include <Ethernet.h>

// Version string
char versionString[] = "v1.0.0-201708191318";

// network configuration.  gateway and subnet are optional.

// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xA5, 0x78 };
// the IP address for the shield:
byte ip[] = {192, 168, 15, 115};
// the router's gateway address:
byte gateway[] = { 192, 168, 15, 1 };
// the subnet:
byte subnet[] = { 255, 255, 255, 0 };

// Create a server that listens for incoming connections on the specified port.
EthernetServer server = EthernetServer(10001);
EthernetClient client;
boolean alreadyConnected = false; // whether or not the client was connected previously

char serInString[21];  // array that will hold the  bytes of the incoming string.
char serOutString[21]; // Array for output of input pins

char digPins[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int val = 0;  // temporary value for inputs & outputs
int firstPin = 30;  // start pin for digital out sequence

void setup()
{
  // Initialize USB connection
  Serial.begin(9600);

  // Initialize Ethernet connection
  Ethernet.begin(mac, ip);
  // start listening for clients
  server.begin();

  // Initialize pin state
  for (int i=0; i < 20; i++) {
    if (digPins[i] == 1) {
      pinMode(i+firstPin, OUTPUT);
    }
  }
}

void loop () {
  // wait for a new client
  client = server.available();
  
  if (client) {
    while (client.connected()) {
      //read the serial port and create a string out of what you read
      readEthernetString(serInString);

      if (isStringEmpty(serInString) == false) {
        //Process for D Command + 20 H or L
        // "D" = code for digital out to arduino looks for D as first
        // character in the input string (ascii 68)
        // then looks for H & L letters
        if ( serInString[0]==68 ) {
          // Is this D
          //Serial.println("Got D Command");
          for (int i=0; i < 20; i++) {
            if (digPins[i] == 1) {
              val = serInString[i+1];  // what is the letter
              if (val == 72) {
                // if it is an H (high - ascii 72)
                digitalWrite(i+firstPin, 0);  // output to the pin
                //Serial.print(String(i+firstPin) + "h");
              } else if (val == 76) {
                // if it is a L (low - ascii 76)
                digitalWrite(i+firstPin, 1);  // output to the pin
                //Serial.print(String(i+firstPin) + "l");
              }
            }
          }
          Serial.println("OK - Digital Out Confirmed");
          client.println("OK - Digital Out Confirmed");
        }

        // "S" (ascii83) = code for read arduino digitalOutput state & send to computer
        if (serInString[0]==83) {
          // just send an S from computer
          //Serial.println("Got an S Command");
          serOutString[0] = 83;  // S for return to computer
          
          for (int i=0; i < 20; i++) {
            if (digPins[i] == 1) {
              val = digitalRead(i+firstPin);
              if (val == HIGH) {
                serOutString[i+1] = 76; //L for high means NO sound
              }
              if (val == LOW) {
                serOutString[i+1] = 72; //H for low means YES sound
              }
            } else {
              serOutString[i+1] = 85; //U for Undefined status
            }
          }
          Serial.print(serOutString); 
          Serial.println("$");  // for end
          client.print(serOutString); 
          client.println("$");  // for end
        }

        // "V" (ascii86) = code to show firmware version
        if (serInString[0] == 86) {
          Serial.print("Firmware version: ");
          Serial.println(versionString);
          client.print("Firmware version: ");
          client.println(versionString);
        }

        // Waits until all outgoing characters in buffer have been sent.
        client.flush();
      }

      //cleans out arrays
      for (int i=0; i < 21; i++) {
        serInString[i]=0;
        serOutString[i]=0;
      }
    }

    if (!client.connected()) {
      Serial.println("disconnecting.");
      client.stop();
      alreadyConnected = false ;
    }
    
    //slows down the visualization in the terminal
    delay(200);
  }
}

void readEthernetString (char *strArray)
{
  int i = 0;
  int chars = 0;
  char firstChar;

  // when the client sends the first byte, Flush and read data
  if (!alreadyConnected) {
    // clear out the output buffer:
    client.flush();
    //Serial.println("We have a new client");
    //client.println("Hello, client!");
    alreadyConnected = true;
  }

  chars = client.available(); // Returns the number of bytes available for reading
  if (chars > 0) {
    firstChar = client.read();
    if ((firstChar == 68 && chars == 23) || (firstChar == 83 && chars == 3 ) || (firstChar == 86 && chars == 3 )) {
      strArray[i] = firstChar;
      chars = chars - 3;
      i++;
      while (chars > 0) {
        strArray[i] = client.read();
        i++;
        chars--;
      }
      client.read();
      client.read();
    } else {
      // This is not a valid command.
      // Flush buffer and finish.
      client.print("ERROR - Chars: ");
      client.println(chars);
      while (chars > 0) {
        client.read();
        chars--;
      }
    }
  }
}

//utility function to know wither an array is empty or not
boolean isStringEmpty(char *strArray) 
{
  if (strArray[0] == 0) {
    return true;
  } else {
    return false;
  }
}

