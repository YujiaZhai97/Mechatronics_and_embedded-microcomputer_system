#include <xc.h>
#include <pic.h>
#pragma config FOSC=HS, CP=OFF, DEBUG=OFF, BORV=20, BOREN=0, MCLRE=ON, PWRTE=ON, WDTE=OFF
#pragma config BORSEN=OFF, IESO=OFF, FCMEN=0
/* Note: the format for the CONFIG directive starts with a double underscore.
The above directive sets the oscillator to an external high speed clock,
sets the watchdog timer off, sets the power up timer on, sets the system
clear on (which enables the reset pin) and turns code protect off. */
/**************************************************************
StepperMotors.c
/**************************************************************
This program is programmed in C, which coontrols two step motors.
There are 6 modes with different starting positions and motions.
Hardware for this program is the Mechatronics microcomputer board.
The program converts the potentiometer value on RA0 and displays it as
an 8 bit binary value on Port D.
**************************************************************/
// Variable declarations
char Temp;                                                    // Variable for delay loop  
char i;                                                       // Variable for delay loop                                                          
unsigned long Timer;
unsigned char Count;

void switchdelay(void) // Waits for switch debounce
{
  for (i=20; i > 0; i--){} // 120 us delay
}

void RotationDelay(void)
{
	for (Timer = 1923; Timer > 0; Timer--){} //0.05 s delay	
}

#define PORTBIT(adr,bit)		((unsigned)(&adr)*8+(bit))
// The function PORTBIT is used to give a name to a bit on a port
// The variable RC0 could have equally been used
	static bit		greenButton 	@ 		PORTBIT(PORTC,1);
	static bit 		redButton 		@ 		PORTBIT(PORTC,0);
	static bit 		HBI				@		PORTBIT(PORTA,2);
	static bit 		VBI				@		PORTBIT(PORTA,3);
	static bit 		HUI				@		PORTBIT(PORTA,0);
	static bit 		VUI				@		PORTBIT(PORTA,1);

// Define pose
int HB = 0; //PortB, 7
int VB = 1; //PortB, 6
int HU = 2; //PortB, 5
int VU = 3; //PortB, 4
int Uniphase = 0;
int Biphase = 0; 
int k = 0;


//motorsychronize
void Motorsinitialization()
{
	PORTD = 0b01001100; //phase1
	RotationDelay();		//stepper motor delay	
	PORTD = 0b00000110;	//phase2
	RotationDelay();		//stepper motor delay
	PORTD = 0b00010011;	//phase3
	RotationDelay();		//stepper motor delay
	PORTD = 0b01011001;	//phase0
	RotationDelay();		//stepper motor delay
	Uniphase = 0;		//initialize phase recorder
	Biphase = 0;		//initialize phase recorder
}


void UCW_full(void)		//two phase
{
	Temp = PORTD >> 4;	//clear unipolar bits
	Temp = Temp << 4;	//setback bipolar bits
	switch((Uniphase++)%4) //increase the phase for CW rotation
	{
		case 0: 
		{
			PORTD = 0b00001001 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b00001100 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000110 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000011 | Temp;
			break;
		}
	}
}

void UCW_wave(void)		//one phase
{
	Temp = PORTD >> 4;	//clear unipolar
	Temp = Temp << 4;	//setback bipolar bits
	switch((Uniphase++)%4)	//increase the phase for CW rotation
	{
		case 0: 
		{
			PORTD = 0b00001000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b00000100 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000010 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000001 | Temp;
			break;
		}
	}
}

void UCCW_full(void)	//two phase
{
	Temp = PORTD >> 4;	//clear unipolar 
	Temp = Temp << 4;	//setback bipolar bits
	switch((Uniphase--)%4)	//decrease the phase for CCW rotation
	{
		case 0: 
		{
			PORTD = 0b00001001 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b00001100 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000110 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000011 | Temp;
			break;
		}
	}
}

void UCCW_wave(void) //one phase
{
	Temp = PORTD >> 4;	//clear unipolar bits
	Temp = Temp << 4;	//setback bipolar bits
	switch((Uniphase--)%4)	//decrease the phase for CCW rotation
	{
		case 0: 
		{
			PORTD = 0b00001000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b00000100 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000010 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000001 | Temp;
			break;
		}
	}
}

void BCW_full(void)		//two phase
{
	Temp = PORTD << 4;	//clear bipolar bits
	Temp = Temp >> 4;	//setback unipolar bits
	switch((Biphase--)%4) // decrease for CW rotation
	{
		case 0: 
		{
			PORTD = 0b01010000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b01000000 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000000 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00010000 | Temp;
			break;
		}
	}
}

void BCW_wave(void)		//one phase
{
	Temp = PORTD << 4;	//clear bipolar bits
	Temp = Temp >> 4;	//setback unipolar bits
	switch((Biphase--)%4)	//decrease for CW rotation
	{
		case 0: 
		{
			PORTD = 0b00010000 | Temp;
			PORTD = 0b11010000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b01010000 | Temp;
			PORTD = 0b01100000 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b01000000 | Temp;
			PORTD = 0b10000000 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000000 | Temp;
			PORTD = 0b00110000 | Temp;
			break;
		}
	}
}

void BCCW_full(void)	//two phase
{
	Temp = PORTD << 4;	//clear bipolar bits
	Temp = Temp >> 4;	//setback unipolar bits
	switch((Biphase++)%4)	// increase for CCW rotation
	{
		case 0: 
		{
			PORTD = 0b01010000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b01000000 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b00000000 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00010000 | Temp;
			break;
		}
	}
}

void BCCW_wave(void)	//one phase 
{
	Temp = PORTD << 4;	//clear bipolar bits
	Temp = Temp >> 4;	//setback unipolar bits
	switch((Biphase++)%4)	//increase for CW rotation
	{
		case 0: 
		{
			PORTD = 0b00010000 | Temp;
			PORTD = 0b11010000 | Temp;
			break;
		}
		case 1: 
		{
			PORTD = 0b01010000 | Temp;
			PORTD = 0b01100000 | Temp;
			break;
		}
		case 2: 
		{
			PORTD = 0b01000000 | Temp;
			PORTD = 0b10000000 | Temp;
			break;
		}
		case 3: 
		{
			PORTD = 0b00000000 | Temp;
			PORTD = 0b00110000 | Temp;
			break;
		}
	}
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void mode1(void)
{
	PORTB = 0b00000001;//int uni_position {0}; // 1: at HUI 0: at VUI
  
  while (1 != 2)
  {
    if (HUI == 0 | VUI == 0) // motor at home, i.e. light blocked
    {
      
      if (greenButton == 0) 
      {
        //break the inf loop of this mode go back to the inf 
        //loop in main fumction 
        break;
      }
    }
    if (redButton == 0)
    {
      while (redButton == 0){} //Check if redbutton is released
      switchdelay();
      // move the rotor
      // Check where is the motor
      if (HUI == 1)
      {
        // To HUI
        while (HUI == 1)
        {
		      UCCW_full();
					RotationDelay();        
        }
        while (HBI == 1)
        {
          BCW_full(); 
					RotationDelay();  
        }
      }
      
      else if (HUI == 0)
      {
        // From HUI to VUI
        while (VUI == 1)
        {
		      UCW_full();
					RotationDelay();       
        }
        while (VBI == 1)
        {
          BCCW_full();
					RotationDelay();   
        }
      }
    }
  }
}

void mode2(void)
{
	PORTB = 0b00000010;
	while (HUI == 1)
  {
		UCCW_full();
		RotationDelay();        
	}
	while (HBI == 1)
	{
		BCW_full();
		RotationDelay();
	}
	while (1 != 2)
	{	
		k = 0;
		if (redButton == 0)
	    {
	      while (redButton == 0){} //Check if redbutton is released
	      switchdelay();
				while (1 != 2)
				{ 
					if (HUI == 0)
					{
				    while (VUI != 0 | VBI != 0)
				    {
				      if (VUI == 1)
				      {
						    UCW_full(); 
								RotationDelay();       
					    }
					    if (VBI == 1)
					    {
						    BCCW_full(); 
								RotationDelay();  
					    }
						}
			          if (redButton == 0) break;
					  
					}
					else if (VUI == 0)
					{
					     while (HUI != 0 | HBI != 0)
					     {
					     	if (HUI == 1)
					      {
							    UCCW_full(); 
									RotationDelay();       
						    }
						    if (HBI == 1)
						    {
							    BCW_full(); 
									RotationDelay();  
						    }
								
						  }
	          			if (redButton == 0) break;
					}
				}
				while (redButton == 0){}
				switchdelay();
				while (redButton == 1)
				{
					switchdelay();
					if (greenButton == 0)
					{
						k += 1;
						break;
					
					}
				}
				if (k >= 1) break;
				
	   	}
	}
}

void mode3(void)
{
	PORTB = 0b00000011;
	while (VUI == 1)
  {
		UCCW_full();
		RotationDelay();        
	}
	while (HBI == 1)
	{
		BCCW_full();
		RotationDelay();
	}
	while (1 != 2)
	{	
		k = 0;
		if (redButton == 0)
    {
      while (redButton == 0){} //Check if redbutton is released
      switchdelay();
			while (1 != 2)
			{ 
				if (VUI == 0)
				{
			    while (HUI != 0 | VBI != 0)
			    {
			      if (HUI == 1)
			      {
					    UCCW_full(); 
							RotationDelay();       
				    }
				    if (VBI == 1)
				    {
					    BCCW_full(); 
							RotationDelay();  
				    }
					}
          if (redButton == 0)
          {
							break;
          }
				  
				}
				else if (HUI == 0)
				{
			     while (VUI != 0 | HBI != 0)
			     {
			     	if (VUI == 1)
			      {
					    UCW_full(); 
							RotationDelay();       
				    }
				    if (HBI == 1)
				    {
					    BCW_full(); 
							RotationDelay();  
				    }
						
				  }
          if (redButton == 0)
          {
							break;
          }

				}
			}
			while (redButton == 0){}
			switchdelay();
			while (redButton == 1)
			{
				switchdelay();
				if (greenButton == 0)
				{
					k += 1;
					break;
				
				}
			}
			if (k >= 1)
			{
				break;
			}
   	}
	}
}

void mode4(void)
{
	PORTB = 0b00000100;
	while (VUI == 1)
  {
		UCW_full();
		RotationDelay();        
	}
	while (HBI == 1)
	{
		BCW_full();
		RotationDelay();
	}
	while (1 != 2)
	{	
		k = 0;
		if (redButton == 0)
    {
      while (redButton == 0){} //Check if redbutton is released
      switchdelay();
			while (1 != 2)
			{ 
				if (VUI == 0)
				{
			    while (HUI != 0 | VBI != 0)
			    {
			      if (HUI == 1)
			      {
					    UCCW_full(); 
							RotationDelay();       
				    }
				    if (VBI == 1)
				    {
					    BCCW_full(); 
							RotationDelay();  
				    }
					}
          if (redButton == 0)break;
				}
				else if (HUI == 0)
				{
			     while (VUI != 0 | HBI != 0)
			     {
			     	if (VUI == 1)
			      {
					    UCW_full(); 
							RotationDelay();       
				    }
				    if (HBI == 1)
				    {
					    BCW_full(); 
							RotationDelay();  
				    }
						
				  }
          if (redButton == 0) break;


				}
			}
			while (redButton == 0){}
			switchdelay();
			while (redButton == 1)
			{
				switchdelay();
				if (greenButton == 0)
				{
					k += 1;
					break;
				
				}
			}
			if (k >= 1)	break;

   	}
	}
}

void mode5(void)
{
	PORTB = 0b00000010;
	while (HUI == 1)
  {
		UCCW_wave();
		RotationDelay();        
	}
	while (HBI == 1)
	{
		BCW_wave();
		RotationDelay();
	}
	while (1 != 2)
	{	
		k = 0;
		if (redButton == 0)
    {
      while (redButton == 0){} //Check if redbutton is released
      switchdelay();
			while (1 != 2)
			{ 
				if (HUI == 0)
				{
			    while (VUI != 0 | VBI != 0)
			    {
			      if (VUI == 1)
			      {
					    UCW_wave(); 
							RotationDelay();       
				    }
				    if (VBI == 1)
				    {
					    BCCW_wave(); 
							RotationDelay();  
				    }
					}
          if (redButton == 0) break;

				  
				}
				else if (VUI == 0)
				{
			     while (HUI != 0 | HBI != 0)
			     {
			     	if (HUI == 1)
			      {
					    UCCW_wave(); 
							RotationDelay();       
				    }
				    if (HBI == 1)
				    {
					    BCW_wave(); 
							RotationDelay();  
				    }
						
				  }
          if (redButton == 0)break;

				}
			}
			while (redButton == 0){}
			switchdelay();
			while (redButton == 1)
			{
				switchdelay();
				if (greenButton == 0)
				{
					k += 1;
					break;
				
				}
			}
			if (k >= 1)	break;

   	}
	}
}

void mode6(void)
{
	PORTB = 0b00000011;
	while (VUI == 1)
  {
		UCCW_wave();
		RotationDelay();        
	}
	while (HBI == 1)
	{
		BCCW_wave();
		RotationDelay();
	}
	while (1 != 2)
	{	
		k = 0;
		if (redButton == 0)
    {
      while (redButton == 0){} //Check if redbutton is released
      switchdelay();
			while (1 != 2)
			{ 
				if (VUI == 0)
				{
			    while (HUI != 0 | VBI != 0)
			    {
			      if (HUI == 1)
			      {
					    UCCW_wave(); 
							RotationDelay();       
				    }
				    if (VBI == 1)
				    {
					    BCCW_wave(); 
							RotationDelay();  
				    }
					}
       		       if (redButton == 0) break;

				  
				}
				else if (HUI == 0)
				{
				     while (VUI != 0 | HBI != 0)
				     {
				     	if (VUI == 1)
					      {
							    UCW_wave(); 
								RotationDelay();       
						  }
					    if (HBI == 1)
						    {
							    BCW_wave(); 
								RotationDelay();  
						    }
							
					  }
	          		  if (redButton == 0)	break;
				}
			}
			while (redButton == 0){}
			switchdelay();
			while (redButton == 1)
			{
				switchdelay();
				if (greenButton == 0)
				{
					k += 1;
					break;
				
				}
			}
			if (k >= 1)	break;

   	}
	}
}

void mode0(void){	PORTB = 0b00001000;
}

void mode7(void){	PORTB = 0b00001111;
}


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void SetupDelay(void)               // Delay loop
{
  for (Temp=1; Temp > 0; Temp--){} // 17 us delay
}

void initAtoD(void) // Initialize A/D
{
  ADCON1 = 0b00001111; // RA0,RA1,RA3 analog inputs, rest digita
  ADCON0 = 0b01000001; // Select 8* oscillator, analog input 0, turn on
  SetupDelay(); // Delay a bit before starting A/D conversion
  GO = 1; // Start A/D
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void main(void)
{
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Initialization of ports and A/D
  PORTB = 0b00000000; 	// Set Port B low
  TRISB = 0b00000000; 	// Configure Port B,LED

  PORTC = 0b00000000; 	// Set Port C low
  TRISC = 0b11111111; 	// Configure Port C, pressbutton

  PORTD = 0b00000000; 	// Set Port D low
  TRISD = 0b00000000; 	// Configure Port D as all output 
  TRISE = 0b00000111; 	// Configure Port E
	
  initAtoD();
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  
  Motorsinitialization(); //initialize phase 

  while (1 != 2)          //infinite loop
  {
    while (greenButton == 0)
    {	
    	// greenbutton pressed
			switchdelay();
      while (greenButton == 0){} //mode

	    if (PORTE == 0b00000000) mode7();

	    else if (PORTE == 0b00000001) mode6();

	    else if (PORTE == 0b00000010) mode5();

	    else if (PORTE == 0b00000011)mode4();

	    else if (PORTE == 0b00000100) mode3();

	    else if (PORTE == 0b00000101) mode2();

	    else if (PORTE == 0b00000110) mode1();

	    else mode0();
		
	  }
  }
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
}
