
/* 
 * Wavetable LFO inspiration: 
 * https://www.muffwiggler.com/forum/viewtopic.php?t=169706&sid=4b0d7d4af599d0df3e7b6fc29ece21b8 
 *  
 */ 

#include "wavetable.h";
//#include "globals.h";
#include "midi.h";


/* =====================================================
==============INIT DATA STRUCTURES======================
======================================================*/  
void setupDataStruct(){
   for (int i=0; i <= 3; i++){
     for (int j=0; j <= 3; j++){
      LEDData[i][j]=false;
     }
     MIDI_CH[i] = EEPROM.read(i);
     if (MIDI_CH[i] > 15) MIDI_CH[i] = 0;
      updateLEDValue(MIDI_CH[i], i);
      encoder[i] = MIDI_CH[i] << 2;
   }
}
/* =====================================================
==============SETUP=====================================
======================================================*/  
void setup() {  
  Serial.begin(38400);
  Serial.println();
  Serial.println("");
  Serial.println(F(__FILE__));
  Serial.print(F("Build date: "));
  Serial.println(F(__DATE__));
  delay(50);  
  setupDataStruct();
  
/* ===========Encoder Setup=============================*/
  pinMode(SWpin, INPUT);
  pinMode(Apin, INPUT);
  pinMode(Bpin, INPUT);
  // Enable internal pull-up resistors
  digitalWrite(SWpin, HIGH);
  digitalWrite(Apin, HIGH);
  digitalWrite(Bpin, HIGH);

/* ===========LED Setup=================================*/
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED_MIDI, OUTPUT);
  pinMode(LED_CV1, OUTPUT);
  pinMode(LED_CV2, OUTPUT);
  pinMode(LED_LFO, OUTPUT);
    
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED_MIDI, HIGH);
  digitalWrite(LED_CV1, HIGH);
  digitalWrite(LED_CV2, HIGH);
  digitalWrite(LED_LFO, HIGH);
  delay(150);

  updateLED();

/* =========Saw wave for table 1 =======================*/
/*  for(int i=0; i<256; i++) {                                  // Precalculates a simple Tri-Wavetable in the first wavetable. Delete these 2 for-loops if you have filled up your wavetables 
    waveTable[1][i]=i; 
    }*/   
/* =========Triangle wave for table 2===================*/
/*  for(int i=0; i<128; i++) {                                  // Precalculates a simple Tri-Wavetable in the first wavetable. Delete these 2 for-loops if you have filled up your wavetables 
    waveTable[2][i]=i*2; 
    } 
  for(int i=0; i<128; i++)  { 
    waveTable[2][i+128]=256-i*2; 
  } 
  waveTable[2][128]=255;*/
/* =========Step 8 wave table 3=========================*/
/*  for(int i=0; i<256; i++) {                                  // Precalculates a simple Tri-Wavetable in the first wavetable. Delete these 2 for-loops if you have filled up your wavetables 
    waveTable[3][i]=(i&0b11100000); 
    }*/ 
/* =========Setup LFO Output pins=======================*/
  pinMode(PWM1, OUTPUT);                                           // Sets Pin PWM1 PWM-Output 
  pinMode(PWM2, OUTPUT);                                           // Sets Pin PWM2 PWM-Output 
  pinMode(Square, OUTPUT);                                          // Pin as LED for Tempo and as a square-LFO 
  pinMode(InvSquare, OUTPUT);                                          // Pin as LED for Tempo and as a inverted square-LFO 
  
/* =========Enable interrupt on A0,1,2==================*/
  // 1. PCIE1: Pin Change Interrupt Enable 1
  PCICR =  0b00000010; 
  // Enable Pin Change Interrupt for A0, A1, A2
  PCMSK1 = 0b00000111; 

//Safety before MIDI takes over so the arduino can be reprogrammed
  delay(800); 
/* =====================================================*/
  setupMidi();
} 

/* =====================================================
==============MAIN LOOP=================================
======================================================*/  
void loop() { 
  checkencoder();                  //Check encoder and update values

  updateLED();                     //LED on for active state    
  midiRead();
  updateLEDValue(MIDI_CH[0], 0); //Update current midi chan for state 0
  clearLED();                      //LED off (dim light)

  updatewave();
}

/* =====================================================
==============Update LED's based on LED Values==========
======================================================*/ 
char getWaveSample(unsigned int PWMshape){
    switch (PWMshape) {
      case 1:
        PWMdata = tableStep;
        // Saw wave for shape 1
        break;
      case 2:
        PWMdata = 256-tableStep;
        // Inv Saw wave for shape 2
        break;
      case 3:
        //Triangle wave for shape 3
        PWMdata = tableStep * 2;
        if (tableStep > 127) PWMdata = 256-(2*(tableStep-127)); 
        break;
      case 4: // Step 4
        PWMdata = (tableStep&0b11000000)+64;
        break;
      case 5: // Step 8
        PWMdata = (tableStep&0b11100000)+32;
        break;
      case 6: // Step 16
        PWMdata = (tableStep&0b11110000)+16;
        break;
      case 7: // Step 32
        PWMdata = (tableStep&0b11111000)+8;
        break;
      case 8: // Step 4 + noise
        PWMdata = random(120) * (tableStep>>6);
        break;
      case 9: // Step 8 + noise
        PWMdata = random(60) * (tableStep>>5);
        break;
      case 10: // noise
        PWMdata = random(255);
        break;
      case 11: // noise
        PWMdata = random(128) + tableStep;
        break;
      case 12: // ramp and noise
        if (tableStep < 127 ) {
          PWMdata = 2 * tableStep;
        }
        else {
          PWMdata = random(255);
        }
        break;
      case 13: // half ramp
        if (tableStep < 127 ) {
          PWMdata = 2 * tableStep;
        }
        else {
          PWMdata = 0;
        }
        break;
      case 14: // 25% duty cycle
        if (tableStep < 63 ) {
          PWMdata = 255;
        }
        else {
          PWMdata = 0;
        }
        break;
      case 15: // pulse 6% duty cycle
        PWMdata = 0;
        if (tableStep < 64) {
          PWMdata = 255;
        }
        if (tableStep > 128) {
          if (tableStep < 192) {
            PWMdata = 255;
          }
        }
        break;
      default:
        // Sine wave for shape 0
        PWMdata = waveTable[tableStep];
        break;
    }
    return PWMdata;
}
/* =====================================================
==============Update Wave data pointer==================
======================================================*/ 
void updatewave(){
  if ( millis() >= (lastwaveupdate+delayTime) ){
    lastwaveupdate = millis();
    tableStep++;                                                // Jumps to the next step. 
                                                                /* tableStep is an 8-Bit unsigned integer, 
                                                                so it can only store a value between 0 and 255 and will 
                                                                automatically "overflow" and go back to 0 when it gets 
                                                                bigger than 255, which is the lenght of the lookup table. 
                                                                 */ 
  /*  Serial.print(F("Delay time: "));
    Serial.print(MIDI_CH[3]<<1);
    Serial.println(F(" "));*/
    delayTime =  32 - (MIDI_CH[3]<<1);                                 // values from 0 to 15 shifted up 1 
                                                                // multiplied by 2 as delay from sample to sample 
  /* ===========Update Square Output======================*/
    if(tableStep<128) {                                           // Turn LED on for first half of the cycle, indicate Tempo 
      digitalWrite(Square, HIGH); 
      digitalWrite(InvSquare, LOW); 
    } 
    else {                                                        // Turn it off for the second half 
      digitalWrite(Square, LOW); 
      digitalWrite(InvSquare, HIGH); 
    } 
  /* ===========Update PWM1 Output========================*/
    PWMdata = getWaveSample(MIDI_CH[2]);

  //  analogWrite(PWM1, waveTable[PWMshape1][tableStep]);              // Writes the value at the current step in the table to Pin 5 as PWM-Signal.  
    analogWrite(PWM1, PWMdata);
  
  /* ===========Update PWM2 Output========================*/
    PWMdata = getWaveSample(0b00001111&(MIDI_CH[2]+4));
  //  analogWrite(PWM2, waveTable[PWMshape2][tableStep]);              // Writes the value at the current step in the table to Pin 5 as PWM-Signal.  
     analogWrite(PWM2, PWMdata);
  }
}

/* =====================================================
==============Check encoder interrupt===================
======================================================*/ 
void checkencoder(){
  if (left){
    left = false;
    if (encoder[state] != 0) encoder[state]--; // Decrement if not 0
    if (state < 4) {
      if(((MIDI_CH[state]=encoder[state] >> 2) == 0) && state == 2) MIDI_CH[state]=16;
      updateLEDValue(MIDI_CH[state], state);
      EEPROM.write(state, MIDI_CH[state]);
    }
  }
  if (right){
    right = false;
    if (encoder[state] < 0x3f) encoder[state]++;
    if (state < 4) {
      MIDI_CH[state]=encoder[state] >> 2;
      updateLEDValue(MIDI_CH[state], state);
      EEPROM.write(state, MIDI_CH[state]);
    }
  }
  if (button){
    button = false;    
    buttoncnt++;
    buttoncnt = buttoncnt & B00000111;
    state = buttoncnt >> 1;
    }
}
/* =====================================================
==============Update LED Values based on input==========
======================================================*/ 
void updateLEDValue(unsigned int nibble,unsigned int updstate)
{
  if (nibble >= 0x10) nibble = 0x0f;
  else (nibble = nibble & 0x0f);
  if (nibble & 0x01){
//    LED1Data = true;
    LEDData[updstate][0] = true;
  }
  else {
//    LED1Data = false;
    LEDData[updstate][0] = false;
  }
  if (nibble & 0x02){
//    LED2Data = true;
    LEDData[updstate][1] = true;
  }
  else {
//    LED2Data = false;
    LEDData[updstate][1] = false;
  }
  if (nibble & 0x04){
//    LED3Data = true;
    LEDData[updstate][2] = true;
  }
  else {
//    LED3Data = false;
    LEDData[updstate][2] = false;
  }
  if (nibble & 0x08){
//    LED4Data = true;
    LEDData[updstate][3] = true;
  }
  else {
//    LED4Data = false;
    LEDData[updstate][3] = false;
  }

}
/* =====================================================
==============Update LED's based on LED Values==========
======================================================*/ 
void updateLED(){
  digitalWrite(LED1, LEDData[state][0]);
  digitalWrite(LED2, LEDData[state][1]);
  digitalWrite(LED3, LEDData[state][2]);
  digitalWrite(LED4, LEDData[state][3]);
  delay(1);
  digitalWrite(LED_MIDI, state & B00000001);
  digitalWrite(LED_CV1, state & B00000010);
  digitalWrite(LED_CV2, state & B00000001);
  digitalWrite(LED_LFO, state & B00000010);
}

void clearLED(){
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED_MIDI, LOW);
  digitalWrite(LED_CV1, LOW);
  digitalWrite(LED_CV2, LOW);
  digitalWrite(LED_LFO, LOW);
}

/* =====================================================
==============Encoder change interrupt==================
======================================================*/  

ISR (PCINT1_vect) {
// If interrupt is triggered by the button
  boolean SW_val = digitalRead(SWpin);
  if (SW_val != SW_old) {
    SW_old = SW_val;
    button = true;}

// Else if interrupt is triggered by encoder signals
  else {
    
    // Read A and B signals
    boolean A_val = digitalRead(Apin);
    if (A_val != A_old) {
      A_old = A_val;
      if (A_val^B_old) {
        right = true;
      }
      else {
        left = true;
      }
    }
    else {
      boolean B_val = digitalRead(Bpin);
      B_old = B_val;
      if (A_val^B_val) {
        left = true;
      }
      else {
        right = true;
      }
    }
  }
}
