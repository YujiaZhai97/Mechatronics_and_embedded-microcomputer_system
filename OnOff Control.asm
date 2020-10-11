 	list P=16F747
	title "Digital On/Off Control"
;***********************************************************************

;	This Program illustrates four different On/Off Control, on this board, 

;	Port A is connected to potentiometer for A/D value setting and reading.
;	Port B is connected to 4 LED showing for mode excecuting and Error type,
;	only pin 0, 1, 2, 3 pins are used.
;	Port C pin 6 and 7 are connected to two switches 
;	Port C pin 0 get sensor input, and pin 2 is an output of buzzer.
;	Port D is connected to 3 other LED, pin 2 connected to Green LED showing
;	On/Off condition of Mode3,
;	pin 6 and 7 connected to LED showing the Transistor condition, reduced or main.
;	Port E connected to octal switch, which control the mode we are going to excecute

;	MODE1 is the most simple one, just turn on or off the main transistor by pressing 
;	red button.

;	MODE2 reads A/D value for time. When turn on transistor, the program will counting down the time.
;	When it is zero, turn off main transistor. 
;	We can also reset time counting in the process by pressing red button.

;	MODE3 decides On/Off condition by A/D value used for comparation with 70h.
;	When A/D value is above this value, Main transistor is on, or it will off.
;	Comparation will be processed when red button is not pressed.
;	When red button is pressed, this Mode is stopped.

;	MODE4 reads A/D value for time. When Main transistor is 'on', program uses sensor to check if
;	the transistor is really on. If not, it is in Error.
;	If it really on, turn on reduced transistor to reduce power. Then trun off Main transistor.
; 	When time is counting, keep on checking if transistor is really on, if not 
;	for the first time, it restart from turn on main transistor, if the problem still
;	exist, the system is in Error.
;	If the main transistor is really on, wait untill time is over, then turn off transistor.
;	Use sensor to detect if the transistor is realy off.
;***********************************************************************

	#include <P16F747.INC>

 __config _CONFIG1, _FOSC_HS & _CP_OFF & _DEBUG_OFF & _VBOR_2_0 & _BOREN_0 & _MCLR_ON & _PWRTE_ON & _WDT_OFF
 __config _CONFIG2, _BORSEN_0 & _IESO_OFF & _FCMEN_OFF

; Variable declarations
	
Count	equ		20h		; the counter
Temp	equ		21h		; a temporary register
State	equ		22h		; the program state register
Mode	equ		23h		; the mode in operation
Timer2 	equ 	24h 	; timer storage variable
Timer1 	equ 	25h 	; timer storage variable
Timer0 	equ 	26h 	; timer storage variable
Etype	equ		27h		; define error type
Temp1	equ		28h		; a temporary register
Temp2	equ		29h		; a temporary register

		org		00h			; interrupter vector
 		goto	SwitchCheck	; jump to interrupt service routine (dummy)
		org		04h 		; interrupt vector
		goto 	isrService 	; jump to interrupt service routine (dummy)

		org		15h			; Beginning of program storage

SwitchCheck

		call	initPort	; initialize ports

AtoD
		call 	initAD 		; call to initialize A/D
		call 	SetupDelay 	; delay for Tad (see data sheet) prior to A/D start
		bsf 	ADCON0,GO 	; start A/D conversion
;
;&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
;

;		Before Mode Operation

waitPress		
		btfss	PORTC,7			; see if green button pressed(0), if not(1) excecute next sentence
		goto 	GreenPress		; green button is pressed - goto routine
		goto	waitPress 		; keep checking

GreenPress
		call 	SwitchDelay		; let switch debounce
		btfsc 	PORTC,7			; confirm green button pressed(0),skip goto, not pressed(1), goto waitPress
		goto	waitPress		; noise - not pressed - keep checking

GreenRelease
		bcf 	PORTD,7			; turn off main transistor 
		btfss 	PORTC,7 		; see if green button released(1)
		goto 	GreenRelease 	; no - keep waiting

; 		ReadModeSwitch
		movf	PORTE,W			; get value of the Octal
		andlw	B'11110111'		; get only last three digits E2, E1, E0
		movwf	Mode			; move the value to Mode register 
		movwf	Temp
		comf	Temp,W			; get true value of mode
					
		andlw	B'00000111'		; save mode in Temp
		movwf	Temp			
		movwf	Etype			; set error type
		bsf		Etype,3
		movf	Etype,W

		movlw	B'00000101'		; 5,6,7, fault
		subwf	Temp,W
		btfsc	STATUS,0
		goto	Errors

		movlw	B'00000001'		; 0, fault
		subwf	Temp,W
		btfss	STATUS,0
		goto	Errors
		
		movf	Temp,W
		movwf	PORTB			; output the Mode value to red LED
		
waitPress1
		btfss	PORTC,7			; see if green button pressed(0), if not(1) excecute next sentence
		goto 	GreenPress1		; green button is pressed - goto routine
		goto 	waitTrans2		; wait for get into mode execution

GreenPress1
		call 	SwitchDelay		; let switch debounce
		btfsc 	PORTC,7			; confirm green button pressed(0),skip goto, not pressed(1), goto waitPress
		goto	waitPress1		; noise - not pressed - keep checking
		goto	GreenRelease

waitTrans2
		btfss	PORTC,6			; see if red button pressed(0)
		goto 	RedPress		; red button is pressed - goto routine
		goto	waitPress1		

RedPress
		call 	SwitchDelay		; let switch debounce
		btfsc 	PORTC,6			; confirm green button pressed(0),skip goto, not pressed(1), goto waitPress
		goto	waitPress1		; noise - not pressed - keep checking

RedRelease
		btfss 	PORTC,6 		; see if red button released(1)
		goto 	RedRelease 		; no - keep waiting
		
GoToMode
		movf	PORTB,W			; fetch Mode value
		btfsc	Mode,2			; check E2(1) in mode range 0-3, (0) out of range
		goto	Mode1			; define 1 2 3 in Mode1
		goto	Mode4			; just Mode 4

;
;111111111111111111111111111111111111111111111111111111111111111111111111111
;
; 		Mode 1

Mode1
		btfss	Mode,1			; ckeck if E1=1, stay and define 0/1
		goto	Mode2			; E1=0, goto Mode2
		
		movlw	B'10000000'
		addwf	PORTD,F			; toggle main transistor
		goto	waitPress1
;
;		end of Mode1
;11111111111111111111111111111111111111111111111111111111111111111111111
;
;22222222222222222222222222222222222222222222222222222222222222222222222
;
; 		Mode 2		
		
Mode2
		btfss	Mode,0			; check if E0=1, 101, Mode2
		goto	Mode3			; E0=0, goto 100, Mode3
		movlw	B'00001010'
		movwf	Etype			; error type 1010
		movwf	Temp1

		bsf 	ADCON0,GO 		; restart A/D conversion
		call	GetAD			; Get value, read A/D
		bcf		STATUS,0	
		rrf		Temp,F
		bcf		STATUS,0
		rrf		Temp,F			; devided by 4, A/D for time				
		bsf		PORTD,7			; turn on main transistor

Timepoint2
		bsf		STATUS,0
		call	TimePoint2		; minus 1 second
		btfss	PORTC,6			; see if red button pressed(0)
		goto	RedPress2	
		decfsz 	Temp,F 			; decrement time
		goto 	Timepoint2		; check slide switch	
		bcf		PORTD,7
		goto	waitPress1		; the end of Mode2		

RedPress2
		call 	SwitchDelay		; let switch debounce
		btfss 	PORTC,6			; confirm red button pressed(0),skip goto, not pressed(1), goto waitPress
		goto	RedRelease		; restart reading A/D value
	
;
;		end of Mode2
;2222222222222222222222222222222222222222222222222222222222222222222
;
;33333333333333333333333333333333333333333333333333333333333333333333
;		
;		Mode 3

Mode3		
		bsf		ADCON0,GO
		bsf		PORTD,2			; turn on LED
		movlw	B'00001011'
		movwf	Etype			; error type 1010
		movwf	Temp1	

		call    GetAD			; get A/D value
		movlw	70h				; check A/D > 70h
		subwf	Temp,F			; A/D - 70h
		btfss	STATUS,0		; check c = 0, or c = 1
		goto	Off3
		bsf		PORTD,7 
		goto	waitPress3

Off3
		bcf		PORTD,7

waitPress3
		btfss	PORTC,6			; see if red button pressed(0)
		goto	RedPress3
		goto	Mode3

RedPress3
		call 	SwitchDelay		; let switch debounce
		btfsc 	PORTC,6			; confirm green button pressed(0),skip goto, not pressed(1), goto waitPress
		goto	Mode3			; noise - not pressed - keep checking		
		
RedRelease3
		btfss 	PORTC,6 		; see if red button released(1)
		goto 	RedRelease3		; no - keep waiting

		bcf		PORTD,7			; turn off Main Traansistor
		bcf		PORTD,2			; turn off LED

		goto 	waitPress1
;
;		end of Mode3
;33333333333333333333333333333333333333333333333333333333333
;
;444444444444444444444444444444444444444444444444444444444444
;
;		Mode 4

Mode4
		movlw	B'00000000'
		movwf	Count			; count = 0 initially
		movlw	B'00001100'
		movwf	Etype			; error type 1100
		movlw	B'00001100'		
		movwf	Temp1			; sub fault,zero fault
		
		call	GetAD			; get A/D value
		bcf		STATUS,0	
		rrf		Temp,F
		bcf		STATUS,0
		rrf		Temp,F			; devided by 4, timer from A/D

Restart4
		bsf		PORTD,7			; turn on main transistor
		movlw	B'00001010'		; 10 second
		movwf	Temp1			; timer for 10 seconds

Sensorhigh
		btfsc	PORTC,0			; sensor high or low?
		goto	Continue4		; sensor high
		
		call 	TimePoint2
		decf	Temp
		decfsz 	Temp1,F 		; decrement time
		goto 	Sensorhigh 		; check slide switch

		movlw	B'00000010'	
		movwf	Temp1			; sub fault, engage fault/ retract fault		
		goto	Errors

Continue4		
		bsf		PORTD,6			; turn on reduced transistor (yellow and amber both on)
		call	SwitchDelay		; wait a little
		bcf		PORTD,7			; turn off main transistor

Timedone4			
		call	TimePoint2		
		decfsz	Temp,F
		goto	Sensorhigh2	
		
		bcf		PORTD,6			; turn off reduced transistor
		movlw	B'00000000'
		movwf	Count			; reset restart counter
		movlw	B'00001010'		; 10 second
		movwf	Temp1			; timer for 10 seconds					
								;CheckOff4
CheckOff4
		btfss	PORTC,0			; sensor low?
		goto	waitPress1		; the end of Mode4
		
		call	TimePoint2		; 1 second
		decfsz	Temp1
		goto	CheckOff4
		
		movlw	B'00000100'		; sub fault, extend fault
		movwf	Temp1
		goto	Errors

Sensorhigh2		 
		movlw	B'00000011'		; sub fault, disengage fault
		movwf	Temp1
		btfsc	PORTC,0			; sensor high£¿
		goto	Timedone4		; sensor high
		movlw	B'00000000'
		subwf	Count,F			; Count - 0=?
		
		btfss	STATUS,2
		goto	Errors			; z != 1
		incf	Count,F			; count = 0+1
		bcf		PORTD,6			; turn off reduced transistor
		goto	Restart4
		
;
; 		end of Mode 4
;44444444444444444444444444444444444444444444444444444444444444444444444444444

;Public used Subroutine

; 		Get A/D value
GetAD
		btfsc 	ADCON0,GO 		; check if A/D is finished
		goto 	GetAD   		; loop right here until A/D finished

		btfsc 	ADCON0,GO 		; make sure A/D finished
		goto 	GetAD	 		; A/D not finished, continue to loop
		
		movf 	ADRESH,W 		; get A/D value
		movwf	Temp
		movlw	B'00000001'		; check A/D = 0?
		subwf	Temp,F	
		btfss	STATUS,0		; c = o, negative
		goto 	Errors	 		; if A/D = 0, fault
		incf	Temp,F
		return

Mode67
		movlw	B'00001111'
		movwf	Etype			; error type 1111 = 7
		movwf	Temp1
		btfss	Mode,0			; if E0=1, 011, Mode4
		goto	Errors			; 000, Mode7
		movlw	B'00001110'
		movwf	Etype			; error type 1110 = 6
		movwf	Temp1
		goto	Errors			; 001, Mode6

Errors
		bcf		PORTD,7			; turn off main transistor
		bcf		PORTD,6			; turn off reduced transistor
		movf	Etype,W			; get Error Type
		movwf	PORTB
		goto	isrService		; loop

SwitchDelay
		movlw 	D'20' 			; load Temp with decimal 20
		movwf 	Temp2

delay1
		decfsz	Temp2, F 		; 60 usec delay loop
		goto 	delay1 			; loop until count equals zero
		return 					; return to calling routine

TimePoint2
		movlw 	06h 			; get most significant hex value + 1
		movwf 	Timer2 			; store it in count register
		movlw 	16h 			; get next most significant hex value
		movwf 	Timer1 			; store it in count register
		movlw 	15h 			; get least significant hex value
		movwf 	Timer0 			; store it in count register

delay3		
		decfsz 	Timer0, F 		; Delay loop Timer0
		goto 	delay3
		decfsz 	Timer1, F 		; Delay loop Timer1
		goto 	delay3
		decfsz 	Timer2, F 		; Delay loop Timer2
		goto 	delay3
		return

Buzzerset
		clrf	TMR2			; TMR2 is cleared
		bsf		PORTC,2			; CCP1 is set
		movf	CCPR1L,W		; PWM ducy cycle is latched from CCPR1L to 
		movwf	CCPR1H			; to CCPR1H
		return
		
;
;&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
;

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
;
; InitializeAD - initializes and sets up the A/D hardware.
; Select AN0 to AN3 as analog inputs, proper clock period, and read AN0.
initAD
		bsf 	STATUS,RP0		; select register bank 1
		movlw	B'00001110' 	; RA0 analog input, all other digital
		movwf 	ADCON1 			; move to special function A/D register
		bcf 	STATUS,RP0 		; select register bank 0
		movlw 	B'01000001' 	; select 8 * oscillator, analog input 0, turn on
		movwf 	ADCON0 			; move to special function A/D register
		return

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
;
; This routine is a software delay of 10uS required for the A/D setup.
; At a 4Mhz clock, the loop takes 3uS, so initialize the register Temp with
; a value of 3 to give 9uS, plus the move etc. should result in
; a total time of > 10uS.
SetupDelay
		movlw 	03h 			; load Temp with hex 3
		movwf 	Temp
delay2
		decfsz 	Temp, F 		; Delay loop
		goto 	delay2
		return


;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
;
;		Port Initialization Subroutine

initPort

		clrf	PORTA		; Clear Port A output latches
		clrf	PORTB		; Clear Port B output latches
		clrf	PORTC		; Clear Port C output latches
		clrf	PORTD		; Clear Port D output latches
		clrf	CCP1CON		; Clear CCP1 PWM output latches

		bsf 	STATUS,RP0 	; Set bit in STATUS register for bank 1
		movlw 	B'11111111' ; move hex value FF into W register
		movwf 	TRISA 		; Configure Port A as all inputs		
		clrf 	TRISB 		; Configure Port B as all outputs
		movlw 	B'11111011' ; move hex value FF into W register
		movwf 	TRISC 		; Configure Port C as all inputs
		clrf 	TRISD 		; Configure Port D as all outputs?
		bsf		TRISE,0
		bsf		TRISE,1
		bsf		TRISE,2
		movlw 	B'00001110' ; RA0 analog input, all other digital
		movwf 	ADCON1 		; move to special function A/D register
		bcf 	STATUS,RP0 	; Clear bit in STATUS register for bank 0
		return
;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
;

;		Buzzer Initialization Subroutine
iniBuzzer
		bcf 	STATUS,RP0 	; Clear bit in STATUS register for bank 0
		movlw	B'00000001'	; initial buzzer register
		movwf	T2CON
		movlw	B'00001111'
		movwf	CCP1CON
		movlw	B'00110100'
		movwf	CCPR1L
		bsf		CCP1CON,5
		bsf		CCP1CON,4
		movlw	B'01100111'
		movwf	PR2
		return

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;
; 		This routine safely hangs up the microcomputer in an infinite loop

isrService
		call	iniBuzzer
		call	Buzzerset
		movlw	00h
		movwf	PORTB
		call	TimePoint2
		movf	Etype,W			; get Error Type
		movwf	PORTB
		call	TimePoint2

		movlw	00h
		movwf	PORTB
		call	TimePoint2

		movf	Temp1,W			; sub Error	
		movwf	PORTB
		call    TimePoint2
		
Timeadd	
		incf	TMR2
		movf	CCPR1H,W
		subwf	TMR2,W			; TMR2 - CCPR1H(duty)
		btfsc	STATUS,0
		bcf		PORTC,2			; duty time reached
		movf	PR2,W
		subwf	TMR2,W			; TMR2 - PR2(period)
		btfss	STATUS,0
		goto	Timeadd
		goto	isrService		; error - - stay here

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		end
