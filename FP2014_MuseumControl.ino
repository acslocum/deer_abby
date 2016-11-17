#include <DmxSimple.h>

/* Welcome to DmxSimple. This library allows you to control DMX stage and
** architectural lighting and visual effects easily from Arduino. DmxSimple
** is compatible with the Tinker.it! DMX shield and all known DIY Arduino
** DMX control circuits.
**
** DmxSimple is available from: http://code.google.com/p/tinkerit/
** Help and support: http://groups.google.com/group/dmxsimple       */

/* To use DmxSimple, you will need the following line. Arduino will
** auto-insert it if you select Sketch > Import Library > DmxSimple. */
#define iR 0 // define index for red as 0
#define iG 1 // define index for green as 1
#define iB 2 // define index for blue as 2
#define iC 3 // define index for base channel number as 3, i.e. the channel id for red of this pod
uint8_t  podColors[8][4];  // eight pods, 3 colors + 1 base channel

uint8_t message[25]; // message from PC, starts with ':', then 8 channels of RGB

void setup() {
  /* The most common pin for DMX output is pin 3, which DmxSimple
  ** uses by default. If you need to change that, do it here. */
  DmxSimple.usePin(3);

  /* DMX devices typically need to receive a complete set of channels
  ** even if you only need to adjust the first channel. You can
  ** easily change the number of channels sent here. If you don't
  ** do this, DmxSimple will set the maximum channel number to the
  ** highest channel you DmxSimple.write() to. */
  DmxSimple.maxChannel(30);
  DmxSimple.write(1,0);
  DmxSimple.write(2,255);
  DmxSimple.write(3,0);
  DmxSimple.write(16,0);
  DmxSimple.write(17,255);
  DmxSimple.write(18,0);
  
  // zero out color matrix
  for (int i=0; i<8; i++)
  {
    for (int j=0; j < 4; j++)
    {
      podColors[i][j] = 0;
    }
  }
  
  // initialize channel numbers in array
  podColors[0][iC] = 4;  // bar 1, pod 1, red
  podColors[1][iC] = 7;  // bar 1, pod 2, red
  podColors[2][iC] = 10;  // bar 1, pod 3, red
  podColors[3][iC] = 13;  // bar 1, pod 4, red
  podColors[4][iC] = 19;  // bar 2, pod 1, red
  podColors[5][iC] = 22;  // bar 2, pod 2, red
  podColors[6][iC] = 25;  // bar 2, pod 3, red
  podColors[7][iC] = 28;  // bar 2, pod 4, red
  
  // setup serial port for communication with computer
  Serial.begin(115200);
}

void loop() 
{
  if (Serial.available() > 0)
  {
    if (Serial.peek() == ':')
    {
      // valid start of command
      if (Serial.available() >= 25)
      {
        for (int i=0; i < 25; i++)
        {
          message[i] = Serial.read();
        }
        setPodColor(0,message[1], message[2], message[3]);
        setPodColor(1,message[4], message[5], message[6]);
        setPodColor(2,message[7], message[8], message[9]);
        setPodColor(3,message[10], message[11], message[12]);
        setPodColor(4,message[13], message[14], message[15]);
        setPodColor(5,message[16], message[17], message[18]);
        setPodColor(6,message[19], message[20], message[21]);
        setPodColor(7,message[22], message[23], message[24]);
        writeColors();
      } // if (Serial.available() >= 25)
    } else
    {
      // garbage character, not a ':', read a byte and try again
      Serial.read(); // read byte out
    } // if (Serial.peek() == ':')
  } // if (Serial.available() > 0)
}

void writeColors()
{
  for (int i=0; i < 8; i++)
  {
    DmxSimple.write(podColors[i][iC], podColors[i][iR]); // red
    DmxSimple.write(podColors[i][iC]+1, podColors[i][iG]); // green
    DmxSimple.write(podColors[i][iC]+2, podColors[i][iB]); // blue
  }
}

void setPodColor(uint8_t i, uint8_t r, uint8_t g, uint8_t b)
{
  podColors[i][iR] = r;
  podColors[i][iG] = g;
  podColors[i][iB] = b;
}
