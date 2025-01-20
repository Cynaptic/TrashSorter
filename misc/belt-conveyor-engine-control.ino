#include <Servo.h>

// 定義伺服馬達和直流馬達的引腳
Servo myServo;
int servoPin = 9;  // 伺服馬達控制引腳
int in1 = 4;       // 直流馬達D1方向1
int in2 = 5;       // 直流馬達D1方向2
int in3 = 6;       // 直流馬達D2方向1
int in4 = 7;       // 直流馬達D2方向2
int enableD1 = 3;  // 直流馬達D1的使能引腳
int enableD2 = 10; // 直流馬達D2的使能引腳

void setup() {
  // 初始化伺服馬達
  myServo.attach(servoPin);
  myServo.write(90);  // 初始位置設定為90度

  // 設置直流馬達引腳為輸出模式
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enableD1, OUTPUT);
  pinMode(enableD2, OUTPUT);
  
  // 初始化串口通訊
  Serial.begin(9600);
}

void loop() {
  // 檢查是否有可讀取的字元
  if (Serial.available() > 0) {
    // 讀取傳入的字元
    char receivedChar = Serial.read();

    // 判斷接收到的字符並控制伺服馬達及直流馬達
    if (receivedChar == 'K') {
      rotateServo(0, 30000);  // 順時針旋轉30秒
    } else if (receivedChar == 'H') {
      rotateServo(0, 30000);  // 順時針旋轉30秒
    } else if (receivedChar == 'J') {
      rotateServo(0, 15000);  // 順時針旋轉15秒
      driveMotorD1(15000);    // 直流馬達D1運行15秒
    } else if (receivedChar == 'L') {
      rotateServo(180, 15000);  // 逆時針旋轉15秒
      driveMotorD2(15000);      // 直流馬達D2運行15秒
    }
  }
}

// 控制伺服馬達旋轉
void rotateServo(int angle, int duration) {
  unsigned long startTime = millis();
  myServo.write(angle);  // 設置伺服馬達角度

  // 伺服持續保持指定角度一段時間
  while (millis() - startTime < duration) {
    // 這裡可以執行其他任務或保持等待
  }

  myServo.write(90);  // 恢復伺服馬達到初始位置
}

// 控制直流馬達D1運行
void driveMotorD1(int duration) {
  digitalWrite(in1, HIGH);  // 設置D1正轉
  digitalWrite(in2, LOW);
  analogWrite(enableD1, 255);  // 啟動馬達（全速）

  delay(duration);  // 持續運行指定時間

  analogWrite(enableD1, 0);  // 停止馬達
}

// 控制直流馬達D2運行
void driveMotorD2(int duration) {
  digitalWrite(in3, LOW);   // 設置D2反轉
  digitalWrite(in4, HIGH);
  analogWrite(enableD2, 255);  // 啟動馬達（全速）

  delay(duration);  // 持續運行指定時間

  analogWrite(enableD2, 0);  // 停止馬達
}
