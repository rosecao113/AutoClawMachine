#include <TimerOne.h>
#include <Servo.h>
// 左限位开关，用变量记住总行程脉冲，可不用右限位
const int pinLim[2] = { 12, 13 }; 

// 步进电机驱动，[:,0]PUL+，[:,1]DIR+
const int pinStep[2][2] = { { 4, 5   },  // 上滑台
                            { 6, 7 } };  // 下滑台
enum { PUL = 0, DIR = 1 };  // 对应列号
enum { StepperTop = 0, StepperBot = 1 }; // 上、下滑台，对应pinLim的列、pinStep的行

// 推杆，直流电机
const int pinRelay[4] = { 8, 9, 10, 11 };                     // DO端口，控制四路固态继电器
const bool controlSignal[3][4] = { { HIGH, LOW, LOW, HIGH },  // 推杆伸长
                                   { LOW, HIGH, HIGH, LOW },  // 推杆缩短
                                   { LOW, LOW, LOW, LOW } };  // 停止
enum { LONG = 0, SHORT = 1, STOP = 2 };  // 依次为伸长、缩短、停止，对应controlSignal的行

// 抓手
const int pinPwm = 3; // 抓手舵机pwm
Servo claw;
enum { OPEN = 140, CLOSE = 40 };  // 抓手张开、闭合，对应舵机角度

// 全局变量
const int totalPulse[2] = {7530, 11243};  // 滑台总行程脉冲数，上滑台x轴，下滑台y轴
int currentPulse[2] = { 0, 0 }; // 上、下滑台当前脉冲
float coord_x = 0, coord_y = 0;    // 目标坐标
int curPos = OPEN;               // 记录舵机当前角度变量
bool hasTarget = false;       // 是否有目标坐标传入
#define leftDir HIGH
#define rightDir LOW

void setup() {
  Serial.begin(9600);  // 初始化串口, 默认波特率为 9600
  while (!Serial) {};  // 等待串口初始化完成

  // 初始化滑台复位
  for (int i = 0; i < 2; i++) {
    pinMode(pinLim[i], INPUT_PULLUP);
    pinMode(pinStep[i][PUL], OUTPUT);
    pinMode(pinStep[i][DIR], OUTPUT);
  }
  reset();  

  // 初始化推杆缩短
  for (int i = 0; i < 4; i++) {
    pinMode(pinRelay[i], OUTPUT);
  }
  control(SHORT, 6);

  // 初始化抓手张开
  claw.attach(pinPwm);
  for (int i = 40; i<= 140; i++) { 
    claw.write(i);           
    delay(15);                      
  }
}

void control(int state, int dt){
  // 控制推杆
  // 推杆速度15mm/s，最大行程100mm
  // 利用延迟时间dt控制伸缩距离，伸缩距离(15*dt)mm
  for (int i = 0; i < 4; i++) digitalWrite(pinRelay[i], controlSignal[state][i]); 
  delay(dt * 1000);
  for (int i = 0; i < 4; i++) digitalWrite(pinRelay[i], controlSignal[STOP][i]); 
  delay(500); 
}

void clawWrite(int tarPos){
  // 控制抓手
  // 舵机开合角度限制在40~140
  int minPos = tarPos > curPos ? 0 : tarPos;
  int maxPos = tarPos > curPos ? tarPos : 180;
  int posDir = tarPos > curPos ? 1 : -1;
  for (; curPos >= minPos  && curPos <= maxPos; curPos += posDir) { 
    claw.write(curPos);          
    delay(15);                      
  }
}

void step(int stepper, int dir) {
  if ((!digitalRead(pinLim[stepper]) && dir == leftDir) || ( currentPulse[stepper] >= totalPulse[stepper] && dir == rightDir))
    return;
  digitalWrite(pinStep[stepper][DIR], dir);   // 方向
  digitalWrite(pinStep[stepper][PUL], HIGH);  // 脉冲
  digitalWrite(pinStep[stepper][PUL], LOW);
  delayMicroseconds(1000);  // 延时
}

void reset(){
  while(digitalRead(pinLim[StepperTop])) step(StepperTop, leftDir);
  while(!digitalRead(pinLim[StepperTop])) step(StepperTop, rightDir);
  currentPulse[StepperTop] = 0;
  while(digitalRead(pinLim[StepperBot])) step(StepperBot, leftDir);
  while(!digitalRead(pinLim[StepperBot])) step(StepperBot, rightDir);
  currentPulse[StepperBot] = 0;
  delay(15);
}

void sliderMove(int stepper, int targetPulse, int& currentPulse){
  while(1){
    if(targetPulse > currentPulse){
      step(stepper, rightDir);
      currentPulse++;
    }else if(targetPulse < currentPulse){
      step(stepper, leftDir);
      currentPulse--;
    }else{
      break;
    }
  }
}

void serialEvent() {
  // 读取串口传来的坐标
  coord_x = Serial.parseFloat();
  delay(15);
  coord_y = Serial.parseFloat();
  delay(15);
  hasTarget = true;
}

void loop() {
  if(!hasTarget) return;

  // 滑台移动到指定坐标位置
  sliderMove(StepperBot , int(totalPulse[StepperBot]*coord_x), currentPulse[StepperBot]);
  sliderMove(StepperTop , int(totalPulse[StepperTop]*coord_y), currentPulse[StepperTop]);
  delay(500);
  control(LONG, 6.7);    // 推杆伸长
  clawWrite(CLOSE); // 抓手闭合
  delay(500);
  control(SHORT, 6.7);   // 推杆缩短

  reset();          // 滑台复位
  clawWrite(OPEN);  // 抓手张开
  Serial.println("finish!");
  hasTarget = false;
}
