//**************************************************************************
//*   Simple AVR wavetable synthesizer V1.0                                *
//*                                                                        *
//*   Implements 4 voices using selectable waveform and envelope tables    *
//*   Uses 8-bit PWM @ 62.5 kHz for audio output                           *
//*                                                                        *
//*   (C) DZL 2008                                                         *
//**************************************************************************

#include <math.h>

#include "avr/interrupt.h"
#include "avr/pgmspace.h"
#include "sin256.h"
#include "ramp256.h"
#include "saw256.h"
#include "square256.h"
#include "noise256.h"
#include "tria256.h"
#include "env0.h"
#include "env1.h"
#include "env2.h"
#include "env3.h"

#define SET(x,y) (x |=(1<<y))		        					//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       						// |
#define CHK(x,y) (x & (1<<y))           						// |
#define TOG(x,y) (x^=(1<<y))            						//-+
//*********************************************************************
//	Audio interrupt
//*********************************************************************

volatile unsigned int PCW[4]={0,0,0,0};							//-Wave phase accumolators
volatile unsigned int FTW[4]={1000,200,300,400};              	//-Wave frequency tuning words
volatile unsigned char AMP[4]={255,255,255,255};                //-Wave amplitudes [0-255]
volatile unsigned int PITCH[4]={500,500,500,500};               //-Voice pitch
volatile int MOD[4]={20,0,64,127};                             	//-Voice envelope modulation [0-1023 512=no mod. <512 pitch down >512 pitch up]
volatile unsigned int wavs[4];                                  //-Wave table selector [address of wave in memory]
volatile unsigned int envs[4];                                  //-Envelopte selector [address of envelope in memory]
volatile unsigned int EPCW[4]={0x8000,0x8000,0x8000,0x8000};    //-Envelope phase accumolator
volatile unsigned int EFTW[4]={10,10,10,10};                    //-Envelope speed tuning word
volatile unsigned int tim=0;                                    //-Sample counter eg. for sequencer
volatile unsigned int tim2=0;                                    //-Sample counter eg. for sequencer

volatile unsigned char divider=4;                               //-Sample rate decimator for envelope
volatile unsigned char tick=0;
volatile unsigned char envg=0;

unsigned char synthTick(void)
{
  if(tick)
  {
    tick=0;
    return 1;
  }
  return 0;
}



#define FS 16000.0                                              //-Sample rate

SIGNAL(TIMER1_COMPA_vect)
{

	OCR1A+=2000000/FS;			                				//-Auto sample rate

	if(divider)
	{
		divider--;
    }
    else
    {

  	//-------------------------------
  	// Volume envelope generator
  	//-------------------------------

	divider=4;
    if(!(EPCW[0]&0x8000))
    {
    	AMP[0]=pgm_read_byte(envs[0]+ ((EPCW[0]+=EFTW[0])>>7) );
     	if(EPCW[0]&0x8000)
          AMP[0]=0;
    }
    else
    	AMP[0]=0;

      if(!(EPCW[1]&0x8000))
      {
        AMP[1]=pgm_read_byte(envs[1]+ ((EPCW[1]+=EFTW[1])>>7) );
        if(EPCW[1]&0x8000)
          AMP[1]=0;
      }
      else
        AMP[1]=0;

      if(!(EPCW[2]&0x8000))
      {
        AMP[2]=pgm_read_byte(envs[2]+ ((EPCW[2]+=EFTW[2])>>7) );
        if(EPCW[2]&0x8000)
          AMP[2]=0;
      }
      else
        AMP[2]=0;

      if(!(EPCW[3]&0x8000))
      {
        AMP[3]=pgm_read_byte(envs[3]+ ((EPCW[3]+=EFTW[3])>>7) );
        if(EPCW[3]&0x8000)
          AMP[3]=0;
      }
      else
        AMP[3]=0;
    }
  //-------------------------------
  //  Synthesizer/audio mixer
  //-------------------------------
    
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Arduino MEGA(2560) boards
    OCR2A=127+
    #else //Arduino ATmega168/328 based boards
    OCR0A=127+
    #endif
  	((
  	(((signed char)pgm_read_byte(wavs[0]+((PCW[0]+=FTW[0])>>8))*AMP[0])>>8)+
  	(((signed char)pgm_read_byte(wavs[1]+((PCW[1]+=FTW[1])>>8))*AMP[1])>>8)+
  	(((signed char)pgm_read_byte(wavs[2]+((PCW[2]+=FTW[2])>>8))*AMP[2])>>8)+
  	(((signed char)pgm_read_byte(wavs[3]+((PCW[3]+=FTW[3])>>8))*AMP[3])>>8)
  	)>>2);

     tim++;
     tim2++;

   //************************************************
   //  Modulation engine
   //************************************************

   if(tim>FS/20)
    {
      switch(envg)
      {
        case 0:
        {
          FTW[0]=PITCH[0]+(PITCH[0]*(EPCW[0]/(32767.5*128.0  ))*((int)MOD[0]-512));
          envg++;
        };break;
        case 1:
        {
          FTW[1]=PITCH[1]+(PITCH[1]*(EPCW[1]/(32767.5*128.0  ))*((int)MOD[1]-512));
          envg++;
        };break;
        case 2:
        {
          FTW[2]=PITCH[2]+(PITCH[2]*(EPCW[2]/(32767.5*128.0  ))*((int)MOD[2]-512));
          envg++;
        };break;
        case 3:
        {
          FTW[3]=PITCH[3]+(PITCH[3]*(EPCW[3]/(32767.5*128.0  ))*((int)MOD[3]-512));
          envg++;
        };break;
        case 4:
        {
          tim=0;
          envg=0;
          tick=1;
        };break;
      }
    }
}

//*********************************************************************
//  Setup all voice parameters
//*********************************************************************

void setup_voice(unsigned char voice,unsigned int waveform, float pitch, unsigned int envelope, float length, unsigned int mod)
{
  wavs[voice]=waveform;//[address in program memory]
  envs[voice]=envelope;//[address in program memory]
  EFTW[voice]=(1.0/length)/(FS/(32767.5*10.0));//[s];
  PITCH[voice]=pitch/(FS/65535.0); //[Hz]
  MOD[voice]=mod;//0-1023 512=no mod
}

//*********************************************************************
//  Setup wave
//*********************************************************************

void setup_wave(unsigned char voice,unsigned char wave)
{
	switch(wave/16)
	{
		case 1:	wavs[voice]=(unsigned int)SinTable;break;
		case 2:	wavs[voice]=(unsigned int)TriangleTable;break;
		case 3:	wavs[voice]=(unsigned int)SquareTable;break;
		case 4:	wavs[voice]=(unsigned int)SawTable;break;
		case 5:	wavs[voice]=(unsigned int)RampTable;break;
		case 6:	wavs[voice]=(unsigned int)NoiseTable;break;
		default:	wavs[voice]=(unsigned int)SinTable;break;
	}
}

//*********************************************************************
//  Setup Envelope
//*********************************************************************

void setup_env(unsigned char voice,unsigned char env)
{
	switch(env/16)
	{
		case 1:	envs[voice]=(unsigned int)Env0;break;
		case 2:	envs[voice]=(unsigned int)Env1;break;
		case 3:	envs[voice]=(unsigned int)Env2;break;
		case 4:	envs[voice]=(unsigned int)Env3;break;
		default:	envs[voice]=(unsigned int)Env0;break;
	}
}


unsigned int EFTWS[128];

//*********************************************************************
//  Setup Length
//*********************************************************************

void setup_length(unsigned char voice,unsigned char length)
{

	EFTW[voice]=EFTWS[length];

/*  EFTW[voice]=(1.0/exp(.057762265 * (length - 69.)))/(FS/(32767.5*10.0));//[s];
	TOG(PORTB,5);
*/
}

//*********************************************************************
//  Setup mod
//*********************************************************************

void setup_mod(unsigned char voice,unsigned char mod)
{
  MOD[voice]=mod*8;//0-1023 512=no mod
//TOG(PORTB,4);

}

unsigned int PITCHS[128];
unsigned int FTWS[128];

//*********************************************************************
//  Midi trigger
//*********************************************************************

void mtrigger(unsigned char voice,unsigned char note)
{
//  PITCH[voice]=(440. * exp(.057762265 * (note - 69.)))/(FS/65535.0); //[MIDI note]
  PITCH[voice]=PITCHS[note];
  EPCW[voice]=0;
  FTW[voice]=PITCH[voice]+(PITCH[voice]*(EPCW[voice]/(32767.5*128.0  ))*((int)MOD[voice]-512));
}

//*********************************************************************
//  Simple trigger
//*********************************************************************

void trigger(unsigned char voice)
{
  EPCW[voice]=0;
}

//*********************************************************************
//  Init synth
//*********************************************************************

void initSynth()
{
  for(unsigned char i=0;i<128;i++)
  {
    EFTWS[i]=(1.0/exp(.057762265 * (i - 69.)))/(FS/(32767.5*10.0));//[s];
    PITCHS[i]=(440. * exp(.057762265 * (i - 69.)))/(FS/65535.0);
    //  		FTWS[voice]=PITCH[voice]+(PITCH[voice]*(EPCW[voice]/(32767.5*128.0  ))*((int)MOD[voice]-512));

  }
  TCCR1B=0x02;                                    //-Start audio interrupt
  SET(TIMSK1,OCIE1A);                             // |
  sei();                                          //-+

  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Arduino MEGA(2560) boards
  TCCR2A=0x83;                                    //-8 bit audio PWM
  TCCR2B=0x01;                                    // |
  OCR2A=127;                                      //-+
  SET(DDRB,4);							        //-PWM pin
  #else //Arduino ATmega328/168 based boards
  TCCR0A=0x83;                                    //-8 bit audio PWM
  TCCR0B=0x01;                                    // |
  OCR0A=127;                                      //-+
  SET(DDRD,6);							        //-PWM pin
  #endif
/*
  UCSR0A=0x02;                        //-Set up serial port 31250
  UCSR0B=0x18;						// |
  UCSR0C=0x06;						// |
  UBRR0=63;
  SET(DDRB,5);
  SET(DDRB,4);
*/							//-+
}

