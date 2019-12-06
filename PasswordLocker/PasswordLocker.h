#ifndef __PWDLK__HEADER__
#define __PWDLK__HEADER__
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "Adafruit_Keypad.h"
#include "LiquidCrystal_I2C.h"
#include <SoftwareSerial.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

	#define F_CPU   16000000UL

#define NORMAL_MSG 0
#define UNLOCKED_MSG 2
#define WRONGPWD_MSG 4
#define OVERTIME_MSG 6
#define UNSELECTED_USR_MSG 8
#define LOCKINSIDE_MSG 10
#define FUCK_MSG 12


#define MAX_WAITTIME 10000 //10 sec
#define DELAYTIME 2000 //提示信息 屏幕等待时间
#define UNLOCK 'O'
#define LOCKINSIDE 'L'

/* PINs defination. */
#define BELLPIN A0

#define ROWS 4 // rows
#define COLS 4 // columns
#define KEYPAD_ADDR 0xFA
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};
const char *password[] = {
	"876318",
	"215404",
	"310316",
	"3119000679"
};
// bool isWokeup = false;
byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {0, 1, 2, 3}; //connect to the column pinouts of the keypad
const char *msg[] = {	" Press password ",
						"                ",
						"    Unlocked    ",
						" Please come in ",
						" WRONG PASSWORD ",
						"    TRY AGAIN   ",
						" Input Overtime ",
						"   Terminated   ",
						" UNSELECTD USER ",
						"PRESS USER FIRST",
						"   LOCKED FROM  ",
						"     INSIDE     ",
						" FUCK YOU BITCH ",
						"GET OUT OF HERE!"
};


#endif /* __PWDLK__HEADER__ */