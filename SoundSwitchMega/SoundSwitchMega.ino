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

// Version string
char versionString[] = "v1.0.0-201708191318";

char serInString[21];  // array that will hold the  bytes of the incoming string.
char serOutString[21]; // Array for output of input pins

char digPins[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int val = 0;  // temporary value for inputs & outputs
int firstPin = 30;  // start pin for digital out sequence

//read a string from the serial and store it in an array
//you must supply the array variable
void readSerialString (char *strArray)
{
  int i = 0;
  if(Serial.available()) {
    int chars = Serial.available();
    while (Serial.available()) {
      strArray[i] = Serial.read();
      i++;
    }
  }
}

void readEthernetString (char *strArray)
{
  int i = 0;
  int chars = 0;
  char firstChar;

  chars = Serial3.available();
  if(chars > 0) {
    firstChar = Serial3.read();
    if ((firstChar == 68 && chars == 23) || (firstChar == 83 && chars == 3 ) || (firstChar == 86 && chars == 3 )) {
      strArray[i] = firstChar;
      chars = chars - 3;
      i++;
      while (chars>0) {
        strArray[i] = Serial3.read();
        i++;
        chars--;
      }
      Serial3.read();
      Serial3.read();
    } else {
      // This is not a valid command.
      // Flush buffer and finish.
      Serial3.print("ERROR - Chars: ");
      Serial3.println(chars);
      while (chars > 0) {
        Serial3.read();
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

void setup()
{
  // Initialize Ethernet connection
  Serial3.begin(9600);

  // Initialize USB connection
  Serial.begin(9600);

  // Initialize pin state
  for (int i=0; i < 20; i++) {
    if (digPins[i] == 1) {
      pinMode(i+firstPin, OUTPUT);
    }
  }
}

void loop () {
  //read the serial port and create a string out of what you read
  readEthernetString(serInString);
  //readSerialString(serInString);

  if( isStringEmpty(serInString) == false) {
    Serial.println(serInString[0]);  
    //Process for D Command + 20 H or L
    // "D" = code for digital out to arduino looks for D as first
    // character in the input string (ascii 68)
    // then looks for H & L letters
    if ( serInString[0] == 68 ) {
      for (int i=0; i < 20; i++) {
        if (digPins[i] == 1) {
          val = serInString[i+1];  // what is the letter
          if (val == 72) {
            // if it is an H (high - ascii 72)
            digitalWrite(i+firstPin, 0);  // output to the pin
          } else if (val == 76) {
            // if it is a L (low - ascii 76)
            digitalWrite(i+firstPin, 1);  // output to the pin
          }
        }
      }
      Serial.println("OK - Digital Out Confirmed");
      Serial3.println("OK - Digital Out Confirmed");
    }

    // "S" (ascii83) = code for read arduino digitalOutput state & send to computer
    if (serInString[0] == 83) {
      // just send an S from computer
      serOutString[0] = 83;  // S for Status return to computer

      for (int i=0; i < 20; i++) {
        if (digPins[i] == 1) {
          val = digitalRead(i+firstPin);
          if (val == HIGH) {
            serOutString[i+1] = 76; //L for high Means NO Sound
          }
          if (val == LOW) {
            serOutString[i+1] = 72; //H for low Means YES Sound
          }
        } else {
          serOutString[i+1] = 85; //U for Undefined status
        }
      }
      Serial.print(serOutString);
      Serial.println("$");  // for end
      Serial3.print(serOutString);
      Serial3.println("$");  // for end
    }

    // "V" (ascii86) = code to show firmware version
    if (serInString[0] == 86) {
      Serial.print("Firmware version: ");
      Serial.println(versionString);
      Serial3.print("Firmware version: ");
      Serial3.println(versionString);
    }

    // Waits for the transmission of outgoing serial data to complete.
    Serial.flush();
    Serial3.flush();

    //cleans out arrays
    for (int i=0; i < 21; i++) {
      serInString[i] = 0;
      serOutString[i] = 0;
    }
  }

  //slows down the visualization in the terminal
  delay(200);
}
