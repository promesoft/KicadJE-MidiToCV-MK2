#include <EEPROM.h>
#include <Adafruit_MCP4725.h>
/* -----------------------------------------------
 *  GLOBAL DEFINES
 * -----------------------------------------------
 */
//OVERRIDE PINS FOR LFO
#define PWM1 10 // Sets Pin PWM1 PWM-Output = Gate 3
#define PWM2 11 // Sets Pin PWM2 PWM-Output = Gate 4

//OVERRIDE PIN FOR SQUARE LFO
#define Square 9    //  Pin as LED for Tempo LED and as a square-LFO 
#define InvSquare 6 //  Pin as LED for Tempo LED and as a inverted square-LFO 

#define SWpin A0 //add 100nF to GND for debounce 
#define Apin A1 //add 100nF to GND for debounce
#define Bpin A2 //add 100nF to GND for debounce
#define CV3in A3 // Base board
#define SDA A4 // Addon board
#define SCL A5 // Addon board
#define CV6in A6 // Doepfer Gate in + Base board
#define CV7in A7 // Doepfer CV in + Base board

#define LED1 2
#define LED2 3
#define LED3 4
#define LED4 5
#define LED_MIDI 7
#define LED_CV1 8
#define LED_CV2 12
#define LED_LFO 13

#define Gate1 6 // Addon board
#define Gate2 9 // Addon board
#define Gate3 10 // Base board
#define Gate4 11 // Base board

//#define RDY-BSY 12
//#define LDAC 13

#define MCP4725_ADDR 0x60   
#define MCP4725_ADDR2 0x61   
/* -----------------------------------------------
 *  GLOBAL VAR
 * -----------------------------------------------
 */
boolean A_old=0, B_old=1, SW_old=0;
boolean left = false, right = false, button = false;

//Structure for keeping MIDI CH data for LED
boolean LEDData[4][4]; //(see state)

boolean LEDSelData = false, LEDSel2Data = false; 
unsigned int encoder[4], buttoncnt = 0;
unsigned int state = 0; 
//For MIDI In/Out/CV - State 00 = MidiDataInChannel - 01 = MIDI CH in select - 10 = MIDI CH OUT select - 11 = MIDI CH 2 CV select
//For MIDI In/CV/LFO - State 00 = MIDI CH 1 CV select - 01 = MIDI CH 2 CV select - 10 = LFO Waveform - 11 = LFO Speed

//byte midi_channel=4;
//Structure for keeping MIDI CH data for processing
//With LFO state 0 and 1 are reserved for CV MIDI channels
//With LFO state 2 and 3 are reserved for the LFO settings
byte MIDI_CH[4]; //(see state above)

unsigned long lastwaveupdate = 0; // LFO millis timer between samples
//unsigned int PWMshape = 0;
unsigned int delayTime = 4; 
uint8_t tableStep = 0;
uint8_t PWMdata = 0;

/* -----------------------------------------------
 *  MIDI2CV VAR
 * -----------------------------------------------
 */
Adafruit_MCP4725 dac;
Adafruit_MCP4725 dac2;
 
int pitches[61] = 
{
     0,   70,  142,  214,  286,  358,  419,  500, 
   571,  642,  713,  786,  860,  933, 1006, 1080, 
  1153, 1226, 1299, 1372, 1445, 1518, 1591, 1664, 
  1737, 1808, 1880, 1952, 2024, 2096, 2168, 2240,
  2310, 2380, 2452, 2525, 2600, 2670, 2740, 2810,
  2880, 2950, 3020, 3090, 3160, 3230, 3300, 3370,
  3440, 3510, 3580, 3650, 3720, 3790, 3860, 3930,
  4000, 4070
};

/* -----------------------------------------------
 *  PROTOTYPES
 * -----------------------------------------------
 */
void MyHandleNoteOn(byte channel, byte pitch, byte velocity);
void MyHandleNoteOff(byte channel, byte pitch, byte velocity);

/*void checkencoder();
//void updateLEDValue(unsigned int nibble,unsigned int updstate);
void updateLED();
void clearLED();
*/
