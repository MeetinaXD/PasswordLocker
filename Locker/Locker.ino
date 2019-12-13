/*
	File 		: PasswordLocker - Slave
	Last Modify : Dec 13,2019T10:52
	Author		: MeetinaXD 
*/

//门内芯片作为从机
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
#include <stdlib.h>
#include <Servo.h>
#include <string.h>
#include <SoftwareSerial.h>
#define KEYPAD_ADDR 0xFA	// Master I2C address

#define MAX_WAITTIME 10000	// Max input wait time (10 sec).
//States defination.
#define WRONG  0
#define OPENED 1
#define LOCKED 2
const char *res[] = {"Wrong!","Ulcked'","Locked"}; // Slave response commands.
char state = 0;
String resState;
unsigned long lastPressTime = 0;
const char *password[] = {	// all the passwords of Users.
	"876318",
	"215404",
	"310316",
	"3119000679"
};
Servo myservo;
bool ledState = false;	// Led in pin13.
bool isUnlocked = false;
SoftwareSerial mySerial(10, 11); // RX, TX
void setup(){
	mySerial.begin(9600);
	myservo.attach(3);
	myservo.write(0);
	Wire.begin(KEYPAD_ADDR);
	Wire.onReceive(receiveEvent);
  	Wire.onRequest(requestEvent); 
  	pinMode(13,OUTPUT);
  	doLock();
}
void loop(){
	ledState = !ledState;
	digitalWrite(13,ledState);
	delay(1000);
	if(isUnlocked)
		if (millis() - lastPressTime > MAX_WAITTIME){
			doLock();
			isUnlocked = false;
		}
}
void receiveEvent(int length){
	mySerial.println(length);
	char *chArr = (char*)malloc(length + 1);
	int i = 0;
	chArr[length] = '\0';
	while (length-- > 0){
		chArr[i++] = Wire.read();
		mySerial.print(chArr[i - 1]);
	}
	
	char operate = chArr[0]; //操作命令
	char user = chArr[1] - 'A'; //选择的用户
	if (user < 0 || user > 3) return; //防止越界
	mySerial.print("selected user = ");
	mySerial.println(chArr[1]);
	mySerial.print("input password = ")
	if (!comparePassword(user,chArr + 2)){ //如果密码和用户不匹配
		mySerial.println(",password pass.");
		state = WRONG;
	}else{
		mySerial.println(",password wrong.");
		if (operate == 'O')
			doUnlock();
		else
			lockInside();
	}
	free(chArr);
	return;
}
int getArrLength(char *p){
	int len = 0;
	while(*(p++) != '\0')
		len++;
	return len;
}
bool comparePassword(char user, char *pwd){
	int la = strlen(password[user]);
	int lb = strlen(pwd);
	if (la != lb) return false;
	int i = 0;
	while(la-->0){
		mySerial.print(pwd[i]);
		if (pwd[i] != password[user][i]) return false;
		i++;
	}
	return true;
}
void lockInside(){
	state = LOCKED;
}
void doLock(){
	myservo.write(140);
}
void doUnlock(){
	state = OPENED;
	myservo.write(65);
	lastPressTime = millis();
	isUnlocked = true;
}
void requestEvent(){ // when Master order to request
	Wire.write(res[state]);
}
