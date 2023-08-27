/*
	File 		: PasswordLocker - Slave
	Last Modify : Sept 12,2022T01:21
	Author		: MeetinaXD
*/

// 门内芯片作为从机
/*
	指令有两条：
		O+user+pwd -> OPEN 开门
		L+user+pwd -> LOCK 反锁
	回复有三条：
		W -> Wrong Password
		O -> Opened.
		L -> Locked from inside.
*/

#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

void sayHello(String &);
void sparkLight(String &);
void setPassword(String &);
void getPassword(String &);
void reset(String &);
void doUnlock(String &);
void doLock(String &);
void doOpen(String &);

void (* resetFunc) (void) = 0;  // declare reset fuction at address 0

// 可用的指令
enum CommandStates {
  SayHello,
  SparkLight,
  SetPassword,
  GetPassword,
	Reset,
	DoUnlock,
	DoLock,
	Open,
  CommandLength
};

// 门锁的状态
enum LockStates {
	Locked,
	Unlocked
};

enum ResponseStates {
	WrongResponse,
	UnlockedResponse,
	LockedResponse
};

// 给密码键盘的回应值
const char *responses[] = {
	"Wrong!",
	"Ulcked'",
	"Locked"
};

typedef struct {
  uint8_t commandState;
  char command[6 + 1]; // 6 bytes available
  void (*pFunc) (String &command);
} CommandState;

// 注册指令
const CommandState commands[] = {
  { SayHello,     "sayhel",			sayHello },
  { SparkLight,   "sparkl",			sparkLight },
  { SetPassword,  "setpwd",			setPassword },
  { GetPassword,  "getpwd",			getPassword },
  { Reset,  			"reset",			reset },
	{ DoUnlock,  		"unlock",			doUnlock },
	{ DoLock,  			"lock",				doLock },
	{ Open,  				"open",				doOpen }
};

// 最大等待时间（解锁后等待多久上锁）
#define MAX_WAITTIME 2000	// Max input wait time (10 sec).
// 门外密码键盘的i2c地址
#define I2C_ADDR 0xFA
// 舵机使用的接口
#define SERVO_PIN 0x03
// 电池管理模块的key接口
#define BATTERY_MODULE_PIN 0x04
#define RX_PIN 0
#define TX_PIN 1
// 返回给键盘的结果
char responseState = WrongResponse;

// 上次解锁时间
unsigned long unlockTime = 0;

// 当前锁状态
bool lockState = Unlocked;

// 当前四个用户的密码
String passwords[4] = {
	"876318",
	"215404",
	"310316",
	"010810"
};

Servo servo;
SoftwareSerial mySerial(6, 7); // RX, TX
void setup() {
	// 配置并启用Timer 0中断
	configTimer2();
	mySerial.begin(9600);
	// Serial.begin(9600);

	servo.attach(3);
	doLock();
	// servo.write(0);

	Wire.begin(I2C_ADDR);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(BATTERY_MODULE_PIN, OUTPUT);
	pinMode(RX_PIN, INPUT);
	pinMode(TX_PIN, OUTPUT);

	activeBatteryModule();
	// 读取密码
	readEEPROM();
}

void loop() {
	static uint8_t counter = 0;
	digitalWrite(LED_BUILTIN, HIGH);
	delay(20);
	digitalWrite(LED_BUILTIN, LOW);
	delay(2000);

	if (counter++ == 5) {
		counter = 0;
		activeBatteryModule();
	}
}

void configTimer2() {
	// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
	// compare match register = [ 16,000,000Hz/ (prescaler * desired interrupt frequency) ] - 1
	// interrupt frequency (Hz) = (Arduino clock speed 16,000,000Hz) / (prescaler * (compare match register + 1))
	// stop interrupts
	cli();

	TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 61.035hz increments
  OCR2A = 255;// = (16*10^6) / (1024 * 61.035) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22 and CS20 bits for 1024 prescaler
  TCCR2B |= (1 << CS22) | (1 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

	// allow interrupts
	sei();
}


// Timer 2's Interrupt
ISR (TIMER2_COMPA_vect) {
	detectAndLock();
	serialEvent();
	// static bool state = false;
	// static uint8_t counter = 0;
	// counter++;
	// if (counter == 21)
	// 	digitalWrite(LED_BUILTIN, HIGH);
	// else if (counter == 31)
	// 	digitalWrite(LED_BUILTIN, LOW);
}

void detectAndLock() {
	// 解锁后指定时间自动上锁
	if (lockState == Unlocked && (millis() - unlockTime > MAX_WAITTIME)) {
		mySerial.println("Time's up, lockup the door");
		doLock();
	}
}

void lockInside() {
	mySerial.println("Lock inside...");
}

void doLock(String &rest) {
	doLock();
}
void doLock(){
	lockState = Locked;
	servo.write(140);
	mySerial.println("locked");
}

void doUnlock(String &rest) {
	doUnlock();
}
void doUnlock(){
	servo.write(65);
	// 记录解锁时间
	unlockTime = millis();
	lockState = Unlocked;
	mySerial.println("unlocked");
}

// 当接收到来自键盘的指令时触发
void receiveEvent(int length){
	String command = "";
	command.reserve(length);

	while (length --> 0) {
		command += (char)Wire.read();
	}

	mySerial.println("Get Command: " + command);

	char operation = command.charAt(0); //操作命令
	char user = command.charAt(1); //选择的用户

	mySerial.print("selected user: ");
	mySerial.println(user);

	// 判断选择的用户是否在A B C D之中
	if (user < 'A' || user > 'D') {
		mySerial.println("Illegal user.");
		return;
	}

	String password = command.substring(2);

	// 判断密码是否一致
	if (passwords[user - 'A'] != password) {
		mySerial.println("Wrong password.");
		responseState = WrongResponse;
		return ;
	}

	mySerial.println("Pass");

	// 根据指令操作
	switch (operation) {
		case 'O':
			responseState = UnlockedResponse;
			doUnlock();
			break;
		case 'L':
			responseState = LockedResponse;
			lockInside();
			break;
	}

	return ;
}


// 当receiveEvent执行完毕后自动触发
void requestEvent() {
	Wire.write(responses[responseState]);
}

String getArgument(String command, uint8_t position) {
	uint8_t lp = 0, p = 0, i = 0;
	command.trim();
	while (i < position + 1) {
		lp = p;
		command = command.substring(lp);
		command.trim();
		p = command.indexOf(" ");

		if (p == -1) return "";
		i++;
	}

	return command.substring(0, p);
}

// 串口指令处理
void serialHandler(String &command) {
	mySerial.println("> " + command);
  for (char i = 0; i < CommandLength; i++) {
    const CommandState &cmd = commands[i];
    const String cmdStr = String(cmd.command);
    if (command.startsWith(cmdStr)) {
      // 获取参数
      command.replace(cmdStr, "");
      // 去除前后空格
      command.trim();
      // mySerial.println("rest is " + command);
      cmd.pFunc(command);
      return ;
    }
  }

  mySerial.println("unmatch command: " + command);
}

// 串口中断
void serialEvent() {
  static String serialString = "";
  // 预留字节用于字符串
  serialString.reserve(32);

  char ch;
  while (mySerial.available() && (ch = (char)mySerial.read())) {
    if (ch == '\n') {
      serialHandler(serialString);
      serialString = "";
    } else {
      serialString += ch;
    }
  }
}

void readEEPROM() {
  for (uint8_t user = 0; user < 4; user++) {
    String p = "";
    for (uint8_t i = 0; i < 16; i++) {
      char ch = EEPROM.read(user * 16 + i);
      if (ch < '0' || ch > '9') {
        break;
      }

      p += ch;
    }
    passwords[user] = p;
  }

  mySerial.println("Read password done.");
}

/* 串口命令事件 */
void sayHello(String &rest) {
  mySerial.println("Hello, world!");
  mySerial.print("Build time: ");
  mySerial.print(__DATE__);
  mySerial.print(" ");
  mySerial.println(__TIME__);
}

void sparkLight(String &rest) {
  for (char i = 0; i < 8; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void setPassword(String &rest) {
  /**
   * 命令格式如:
   * setpwd A 123456
   * setpwd B 34512312312
   */

  if (rest.length() < 3) {
    mySerial.println("Illegal argument.");
    return ;
  }

  char user = getArgument(rest, 0).charAt(0);
  String password = getArgument(rest, 1);

  // 判断用户是否合法
  if (user < 'A' || user > 'D') {
    mySerial.println("Illegal user. Select from A, B, C or D.");
    return ;
  }

  // 判断密码是否合法，最少需4位
  uint8_t length = password.length();
  if (length < 4 || length > 16) {
    mySerial.println("Password's length must in 4~16 digits.");
    return ;
  }

  // 判断密码是否只包含数字
  for (uint8_t i = 0; i < length; i++) {
    char ch = password.charAt(i);

    if (ch < '0' || ch > '9') {
      mySerial.println("Password can only contains number.");
      return ;
    }
  }


  // 每个用户预留的最大密码长度为16
  uint8_t userAddrFrom = (user - 'A') * 16;

  // 写入密码到EEPROM
  for (char i = 0; i < 16; i++) {
    EEPROM.write(userAddrFrom + i, i < length? password.charAt(i): '\0');
  }

  mySerial.println("Password for " + String(user) + " has been changed.");
}

void getPassword(String &rest) {
  readEEPROM();
  for (uint8_t user = 0; user < 4; user++) {
    String p = passwords[user];
    mySerial.print((char)(user + 'A'));
    if (!p.length()) {
      mySerial.println(", [none]");
    } else {
      mySerial.println(", " + p);
    }
  }
}

void reset(String &rest) {
	mySerial.println("Resetting...");
	delay(500);
	resetFunc();
}

void doOpen(String &rest) {
	char user = getArgument(rest, 0).charAt(0);
  String password = getArgument(rest, 1);

	// 判断密码是否一致
	if (passwords[user - 'A'] != password) {
		mySerial.println("Wrong password.");
		return ;
	}

	mySerial.println("Pass");
	doUnlock();
}


// 拉低电池模块的key引脚，重置计数器
void activeBatteryModule() {
	digitalWrite(BATTERY_MODULE_PIN, HIGH);
	delay(500);
	digitalWrite(BATTERY_MODULE_PIN, LOW);
}