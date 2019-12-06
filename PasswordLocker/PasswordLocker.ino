#include "./PasswordLocker.h"
char nowFunction = 0;//现在选择的功能

unsigned long lastPressTime = 0;
String inputStr = "";
byte inputStrIndex = 0;
bool pressed = false;
char nowUser = '\0';
// SoftwareSerial DLSerial(6,7);

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {

	// DLSerial.begin(9600); // init SoftSerial
	lcd.init();
	lcd.backlight();
	lcd.clear();
	customKeypad.begin();
	printMessage(NORMAL_MSG);

	//设置最左一列为中断按键
	// attachInterrupt(0,CISP,CHANGE);
	// doSleep();
}
void loop() {
	// wdt_reset();//记得喂狗
	if (millis() - lastPressTime > MAX_WAITTIME){
		if(pressed){
			printMessage(OVERTIME_MSG);
			_delay_ms(DELAYTIME);
			printMessage(NORMAL_MSG);
			_delay_ms(500);
		}
		lcd.noBacklight();
		inputStr = "";
		inputStrIndex = 0;
		pressed = false;
	}
	customKeypad.tick();
	while(customKeypad.available()){
		keypadEvent e = customKeypad.read();
		// DLSerial.print((char)e.bit.KEY);

		if(e.bit.EVENT == KEY_JUST_PRESSED){
			pressed = true;
			lcd.backlight();
			switch(e.bit.KEY){
				case 'A':
				case 'B':
				case 'C':
				case 'D':
					nowUser = (char)e.bit.KEY;
					inputStr = "";
					inputStrIndex = 0;
					printMessage(NORMAL_MSG);
					break;
				case '*': //功能键*
				case '#'://功能键#
					if (e.bit.KEY == '*'){
						nowFunction = UNLOCK;
					}else{
						nowFunction = LOCKINSIDE;
					}
					pressed = false;
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
			// DLSerial.println(" pressed");
			tone(BELLPIN,1000,50);
			lastPressTime = millis();
		}
			// else if(e.bit.EVENT == KEY_JUST_RELEASED) DLSerial.println(" released");
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
	delay(50);
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
	pressed = true;
}

void printMessage(byte index){
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(msg[index]);
	lcd.setCursor(0,1);
	lcd.print(msg[index + 1]);
}

// void CISP(){
// 	customKeypad.tick();
// 	if(customKeypad.available()){
// 		keypadEvent e = customKeypad.read();
// 		if(e.bit.KEY == '*' && isWokeup == false){
// 			digitalWrite(13,HIGH);
// 			doWakeUp();
// 		}
// 	}
// }
