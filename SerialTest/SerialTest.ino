#include <EEPROM.h>

void sayHello(String &);
void sparkLight(String &);
void setPassword(String &);
void getPassword(String &);

enum CommandStates {
  SayHello,
  SparkLight,
  SetPassword,
  GetPassword,
  CommandLength
};

typedef struct {
  uint8_t commandState;
  char command[6 + 1]; // 6 bytes available
  void (*pFunc) (String &command);
} CommandState;

const CommandState commands[] = {
  { SayHello,     "sayhel",    sayHello },
  { SparkLight,   "sparkl",    sparkLight },
  { SetPassword,   "setpwd",    setPassword },
  { GetPassword,   "getpwd",    getPassword }
};

const String passwords[4];

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  readEEPROM();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
  delay(990);
}

// 串口指令处理
void serialHandler(String &command) {
  for (char i = 0; i < CommandLength; i++) {
    const CommandState &cmd = commands[i];
    const String cmdStr = String(cmd.command);
    if (command.startsWith(cmdStr)) {
      // 获取参数
      command.replace(cmdStr, "");
      // 去除前后空格
      command.trim();
      // Serial.println("rest is " + command);
      cmd.pFunc(command);
      return ;
    }
  }

  Serial.println("unmatch command: " + command);
}

// 串口中断
void serialEvent() {
  static String serialString = "";
  // 预留字节用于字符串
  serialString.reserve(32);

  char ch;
  while (Serial.available() && (ch = (char)Serial.read())) {
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

  Serial.println("Read password done.");
}

/* 串口命令事件 */
void sayHello(String &rest) {
  Serial.println("Hello, world!");
  Serial.print("Build time: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
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
    Serial.println("Illegal argument.");
    return ;
  }

  char user = rest.charAt(0);
  String password = rest.substring(2);

  // 判断用户是否合法
  if (user < 'A' || user > 'D') {
    Serial.println("Illegal user. Select from A, B, C or D.");
    return ;
  }

  // 判断密码是否合法，最少需4位
  uint8_t length = password.length();
  if (length < 4 || length > 16) {
    Serial.println("Password's length must in 4~16 digits.");
    return ;
  }

  // 判断密码是否只包含数字
  for (uint8_t i = 0; i < length; i++) {
    char ch = password.charAt(i);

    if (ch < '0' || ch > '9') {
      Serial.println("Password can only contains number.");
      return ;
    }
  }


  // 每个用户预留的最大密码长度为16
  uint8_t userAddrFrom = (user - 'A') * 16;

  // 写入密码到EEPROM
  for (char i = 0; i < 16; i++) {
    EEPROM.write(userAddrFrom + i, i < length? password.charAt(i): '\0');
  }

  Serial.println("Password for " + String(user) + " has been changed.");
}

void getPassword(String &rest) {
  readEEPROM();
  for (uint8_t user = 0; user < 4; user++) {
    String p = passwords[user];
    Serial.print((char)(user + 'A'));
    if (!p.length()) {
      Serial.println(", [none]");
    } else {
      Serial.println(", " + p);
    }
  }
}