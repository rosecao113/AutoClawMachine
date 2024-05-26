#include <Servo.h>
const int pinRelay[4] = { 3, 4, 5, 6 };                       // DO端口，控制四路固态继电器
const bool controlSignal[3][4] = { { HIGH, LOW, LOW, HIGH },  // 推杆伸长
                                   { LOW, HIGH, HIGH, LOW },  // 推杆缩短
                                   { LOW, LOW, LOW, LOW } };  // 停止
enum { LONG = 0, SHORT = 1, STOP = 2 };  // 依次为伸长、缩短、停止，对应controlSignal的行号
const int pinCom = 2; // 滑台给抓手的任务信号 DI
const int pinPwm = 9; // 抓手舵机pwm
const int pinRet = 8;// 抓手给滑台的报告信号 DO
Servo claw;
enum { OPEN = 140, CLOSE = 40 };  // 抓手张开、闭合，对应舵机角度
int curPos = OPEN;               // 记录舵机当前角度变量
bool hasTarget = false;           // 是否需要抓

void ComSlider() {
  // 通过外部中断得知是否要抓
  hasTarget = !hasTarget;
}

void setup() {
  Serial.begin(9600);  // 初始化串口, 默认波特率为 9600
  while (!Serial) {};  // 等待串口初始化完成
  
  // 初始化端口
  pinMode(pinCom, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinCom), ComSlider, CHANGE);
  pinMode(pinRet, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(pinRelay[i], OUTPUT);
    digitalWrite(pinRelay[i], controlSignal[STOP][i]);  // 发送停止信号
  }
  claw.attach(pinPwm);
  
  // 初始状态：推杆缩短，抓手张开
  control(SHORT);
  for (int i = 40; i<= 140; i++) { 
    claw.write(i);           
    delay(15);                      
  }
}

void control(int state){
  // 控制推杆
  for (int i = 0; i < 4; i++) digitalWrite(pinRelay[i], controlSignal[state][i]); 
  delay(11000);
}

void clawWrite(int tarPos){
  // 控制抓手
  // 舵机开合角度限制在40~140
  int minPos = tarPos > curPos ? 40 : tarPos;
  int maxPos = tarPos > curPos ? tarPos : 140;
  int posDir = tarPos > curPos ? 1 : -1;
  for (; curPos >= minPos  && curPos <= maxPos; curPos += posDir) { 
    claw.write(curPos);           
    delay(15);                      
  }
}

void loop() {
  if(!hasTarget) return;
  
  // 滑台移动到位，要开抓了
  control(LONG);
  clawWrite(CLOSE);
  control(SHORT);
  digitalWrite(pinRet, LOW);   // 抓完，通过pinRet告诉滑台
  delay(100);
  digitalWrite(pinRet, HIGH);
  while(hasTarget);  // 等待滑台复位
  
  // 滑台已复位，抓手张开
  clawWrite(OPEN);
}




