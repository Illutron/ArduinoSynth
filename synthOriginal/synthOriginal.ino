
#include "the_synth.h"
  unsigned int counter=0;
  unsigned char bar;
  unsigned int  demo;
  unsigned char midi_state=0;
  unsigned char midi_cmd=0;
  unsigned char midi_1st=0;
  unsigned char midi_2nd=0;
  unsigned char MFLAG=0;

void setup()
{
  Serial.begin(9600);
  initSynth();
  demo = 0;
  bar = 0;


  setup_voice(0,(unsigned int)SinTable,200.0,(unsigned int)Env1,1.0,300);
  setup_voice(1,(unsigned int)RampTable,100.0,(unsigned int)Env1,1.0,512);
  setup_voice(2,(unsigned int)TriangleTable,100.0,(unsigned int)Env2 ,.5,1000);
  setup_voice(3,(unsigned int)NoiseTable,1200.0,(unsigned int)Env3,.02,500);
}


unsigned char pattern[4][32]=
{
	{1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0},
	{48,0,0,41,0,0,44,0,44,0,0,46,0,0,41,0,48,0,0,41,0,0,44,0,0,44,0,46,0,0,51,0},
	{0,0,0,50,0,0,0,0,0,0,0,50,0,0,0,0,0,0,0,50,0,0,0,0,0,0,0,50,0,0,0,0},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0}
};

unsigned char song[4][32]=
{
	{0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1}
};



void loop()
{
  //************************************************
  // External midi
  //************************************************
  /*
    if(CHK(UCSR0A,RXC0))
    {
      unsigned char c=UDR0;

      UDR0=c;
      switch(midi_state)
      {
      case 0:
        {
          switch(c&0xF0)
          {
          case 0x80:
          case 0x90:
          case 0xB0:
            {
              midi_state=1;
              midi_cmd=c;
            };
            break;
          }
        };
        break;

      case 1:		//-Get 1
        {
          if(c&0x80)
          {
            midi_state=0;
            break;
          }
          midi_1st=c;
          midi_state=2;
        };
        break;

      case 2:		//-Get 2
        {
          if(c&0x80)
          {
            midi_state=0;
            break;
          }
          midi_2nd=c;
          midi_state=0;

          //---------------------------------
          //	Parse MIDI here
          //---------------------------------

          TOG(PORTB,5);

          switch(midi_cmd)
          {
            //	Notes
          case 0x91:
            {
              if(midi_2nd)
                mtrigger(0,midi_1st);
            };
            break;
          case 0x92:
            {
              if(midi_2nd)
                mtrigger(1,midi_1st);
            };
            break;
          case 0x93:
            {
              if(midi_2nd)
                mtrigger(2,midi_1st);
            };
            break;
          case 0x94:
            {
              if(midi_2nd)
                mtrigger(3,midi_1st);
            };
            break;
            //	Controllers

          case 0xb1:
            {
              switch(midi_1st)
              {
              case 0x0d: 
                setup_wave(0,midi_2nd);
                break; //
              case 0x0c: 
                setup_env(0,midi_2nd);
                break;  //12
              case 0x0a: 
                setup_length(0,midi_2nd);
                break;//10
              case 0x07: 
                setup_mod(0,midi_2nd);
                break;//7
              };
              break;
            };
            break;


          case 0xb2:
            {
              switch(midi_1st)
              {
              case 0x0d: 
                setup_wave(1,midi_2nd);
                break; //
              case 0x0c: 
                setup_env(1,midi_2nd);
                break;  //12
              case 0x0a: 
                setup_length(1,midi_2nd);
                break;//10
              case 0x07: 
                setup_mod(1,midi_2nd);
                break;//7
              };
              break;
            };
            break;


          case 0xb3:
            {
              switch(midi_1st)
              {
              case 0x0d: 
                setup_wave(2,midi_2nd);
                break; //
              case 0x0c: 
                setup_env(2,midi_2nd);
                break;  //12
              case 0x0a: 
                setup_length(2,midi_2nd);
                break;//10
              case 0x07: 
                setup_mod(2,midi_2nd);
                break;//7
              };
              break;
            };
            break;


          case 0xb4:
            {
              switch(midi_1st)
              {
              case 0x0d: 
                setup_wave(3,midi_2nd);
                break; //
              case 0x0c: 
                setup_env(3,midi_2nd);
                break;  //12
              case 0x0a: 
                setup_length(3,midi_2nd);
                break;//10
              case 0x07: 
                setup_mod(3,midi_2nd);
                break;//7
              };
              break;
            };
            break;
          }
        };
        break;
      }
    }
    
    */
    //************************************************
    // End midi 
    //************************************************
    
    if(synthTick()) 
    {
      //************************************************
      // Demo mode
      //************************************************
      bar=counter&0x1f;
      demo=counter>>5;

      switch(demo)
      {
        case 4:
        {
          setup_voice(3,(unsigned int)TriangleTable,1500.0,(unsigned int)Env3,.03,100);
        };break;      
        
        case 8:
        {
          setup_voice(3,(unsigned int)NoiseTable,1500.0,(unsigned int)Env3,.03,300);
        };break;      
      
        case 12:
        {
          setup_voice(1,(unsigned int)TriangleTable,100.0,(unsigned int)Env1,2.0,512);
        };break;      
      
        case 16:
        {
          setup_voice(1,(unsigned int)RampTable,100.0,(unsigned int)Env1,1.0,512);
          counter=0;
        };break;      
      }

   //   if(song[0][demo])
        if(pattern[0][bar])
          trigger(0);
          
     // if(song[1][demo])
      if(pattern[1][bar])      
        mtrigger(1,pattern[1][bar]);
        
     // if(song[2][demo])
      if(pattern[2][bar])
        mtrigger(2,pattern[2][bar]);
        
     // if(song[3][demo])
      if(pattern[3][bar])
        trigger(3);
        
      Serial.println(counter);
      //************************************************
      // End demo mode
      //************************************************
      tim=0;
      counter++;
     // counter&=0x001f;
    }
}

