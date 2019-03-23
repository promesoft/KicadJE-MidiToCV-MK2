/*
 * Inspiration from
 * https://github.com/dlynch7/MIDI2CV/blob/master/arduino/midi2pitchvelcv.ino
 */
#include "globals.h"
#include <MIDI.h>  // Add Midi Library
MIDI_CREATE_DEFAULT_INSTANCE();

void setupMidi(){
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initialize the Midi Library.
// OMNI sets it to listen to all channels.. MIDI.begin(2) would set it
// to respond to channel 2 notes only.
  MIDI.setHandleNoteOn(MyHandleNoteOn); // This is important!! This command
  // tells the Midi Library which function I want called when a Note ON command
  // is received. in this case it's "MyHandleNoteOn".
  MIDI.setHandleNoteOff(MyHandleNoteOff);
  dac.begin(MCP4725_ADDR);
  dac2.begin(MCP4725_ADDR2);
}

byte midiRead(){
  MIDI.read(); // Continually check what Midi Commands have been received. 
//  return midi_channel;
}

// Below is my function that will be called by the Midi Library
// when a MIDI NOTE ON message is received.
// It will be passed bytes for Channel, Pitch, and Velocity
void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
  int DACnum = 0;
  int GATEval = LOW;

  DACnum = pitches[pitch-36]; //-36 to transpose bottom of CASIO keyboard to C0.
  if (channel == MIDI_CH[0]) {
    dac.setVoltage(DACnum, false);
    if (velocity == 0) GATEval = LOW;
    else GATEval = HIGH;
    digitalWrite(Gate1,GATEval);  //Turn GATE1 on
    
  }
  if (channel == MIDI_CH[1]) {
    dac2.setVoltage(DACnum, false);
    if (velocity == 0) GATEval = LOW;
    else GATEval = HIGH;
    digitalWrite(Gate2,GATEval);  //Turn GATE2 on
  }
  digitalWrite(Gate1,GATEval);  //Turn GATE1 on
  digitalWrite(Gate2,GATEval);  //Turn GATE1 on
}

void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
  int DACnum = 0;
  int GATEval = LOW;
  
  digitalWrite(Gate1,GATEval);  //Turn GATE1 off
  digitalWrite(Gate2,GATEval);  //Turn GATE2 off
}
