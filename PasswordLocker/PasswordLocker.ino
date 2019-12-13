/*
	File 		: PasswordLocker - Master
	Last Modify : Dec 13,2019T10:52
	Author		: MeetinaXD 
*/

#include "./PasswordLocker.h"
char nowFunction = 0;//现在选择的功能

unsigned long lastPressTime = 0;
String inputStr = "";
byte inputStrIndex = 0;
bool isPressed = false;
char nowUser;

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27,16,2); // components : I2C addr, row, col

void setup() {
	lcd.init();
	lcd.backlight(); // enable the light of the Display
	lcd.clear();
	customKeypad.begin();
	printMessage(NORMAL_MSG);
}
void loop() {
	// wdt_reset();//记得喂狗
	if (millis() - lastPressTime > MAX_WAITTIME){
		if(isPressed){
			printMessage(OVERTIME_MSG);
			_delay_ms(DELAYTIME);
			printMessage(NORMAL_MSG);
			_delay_ms(500);
		}
		lcd.noBacklight();
		inputStr = "";
		inputStrIndex = 0;
		isPressed = false;
	}
	customKeypad.tick();
	while(customKeypad.available()){
		keypadEvent e = customKeypad.read();
		if(e.bit.EVENT == KEY_JUST_PRESSED){
			isPressed = true;
			lcd.backlight();
			switch(e.bit.KEY){
				case 'A':	//user select key 'A'
				case 'B':	//user select key 'B'
				case 'C':	//user select key 'C'
				case 'D':	//user select key 'D'
					nowUser = (char)e.bit.KEY;
					inputStr = "";
					inputStrIndex = 0;
					printMessage(NORMAL_MSG);
					break;
				case '*':	//function key '*'
				case '#':	//function ket '#'
					nowFunction = (e.bit.KEY == '*')? UNLOCK: LOCKINSIDE;
					isPressed = false;
					passwordEvent(inputStr);
					inputStr = "";
					inputStrIndex = 0;
					break;
				default:
					inputStr += (char)e.bit.KEY;
					if (inputStrIndex > 99){
						printMessage(FUCK_MSG);
						_delay_ms(DELAYTIME);
						printMessage(NORMAL_MSG);
						inputStr = "";
						inputStrIndex = 0;
					}
					if (inputStrIndex > 16){
						char number[3];
						itoa(inputStrIndex,number,10);
						lcd.setCursor(13,1);
						lcd.write(' ');
						lcd.write(number[0]);
						lcd.write(number[1]);
					}else{
						lcd.setCursor(inputStrIndex,1);
						lcd.write('*');
					}
					inputStrIndex++;
					break;
			}
			tone(BELLPIN,1000,50);
			lastPressTime = millis();
		}
	}
	_delay_ms(10);
}

String sendCommand(char operate,char user,String command){
	char char_array[command.length() + 1];
	int len = command.length();
	command.toCharArray(char_array, command.length() + 1);
	Wire.beginTransmission(KEYPAD_ADDR);
	Wire.write(operate);
	Wire.write(user);
	int i = 0;
	while (len-- > 0)
		Wire.write(char_array[i++]);
	Wire.endTransmission();
	//等待从机开锁程序回复
	_delay_ms(50);
	Wire.requestFrom(KEYPAD_ADDR, 6);
	String s;
	while (Wire.available() > 0){
		s += char(Wire.read());
	}
	return s;
}

void passwordEvent(String str){
	if (nowUser == 0){
		printMessage(UNSELECTED_USR_MSG);
		_delay_ms(DELAYTIME);
		printMessage(NORMAL_MSG);
		return;
	}
	String response = sendCommand(nowFunction,nowUser,str);
	if(response == "Wrong!"){
		printMessage(WRONGPWD_MSG);
		_delay_ms(DELAYTIME);
		printMessage(NORMAL_MSG);
	}
	if(response == "Ulcked"){
		printMessage(UNLOCKED_MSG);
		_delay_ms(DELAYTIME);
		printMessage(NORMAL_MSG);
	}
	if(response == "Locked"){
		printMessage(LOCKINSIDE_MSG);
		_delay_ms(DELAYTIME);
		printMessage(NORMAL_MSG);
	}
	isPressed = true;
}
void printMessage(byte index){
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(msg[index]);
	lcd.setCursor(0,1);
	lcd.print(msg[index + 1]);
}