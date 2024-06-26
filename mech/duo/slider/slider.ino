#include <TimerOne.h>
const int pinRet = 2;  // 抓手的报告信号，DI
const int pinCom = 8;  // 给抓手的任务信号，DO

// 限位开关，[:,0]左限位，[:,1]右限位
const int pinLim[2][2] = { {  9, 10  },  // 上滑台
                           { 11, 12} };  // 下滑台
enum { LEFT = 0, RIGHT = 1 }; // 对应列号

// 步进电机驱动，[:,0]PUL+，[:,1]DIR+
const int pinStep[2][2] = { { 4, 5   },  // 上滑台
                            { 6, 7 } };  // 下滑台
enum { PUL = 0, DIR = 1 };  // 对应列号

enum { StepperTop = 0, StepperBot = 1 }; // 上、下滑台，对应pinLim和pinStep的行号

// 全局变量
const int total_pulse_top = 7530;  // 上滑台总行程脉冲数
const int total_pulse_bot = 11243;  // 下滑台总行程脉冲数
int currentPulseTop, currentPulseBot; // 上、下滑台当前脉冲
float coord_x = 0, coord_y = 0;    // 目标坐标
bool clawFinish = false;      // 抓手是否完成，用于复位
bool hasTarget = false;       // 是否有目标坐标传入
#define leftDir HIGH
#define rightDir LOW

void RetClaw() {
  // 通过外部中断得知抓手状态
  clawFinish = !clawFinish;
}

void setup() {
  Serial.begin(9600);  // 初始化串口, 默认波特率为 9600
  while (!Serial) {};  // 等待串口初始化完成

  // 初始化
  pinMode(pinRet, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinRet), RetClaw, CHANGE);
  pinMode(pinCom, OUTPUT);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++){
      pinMode(pinLim[i][j], INPUT_PULLUP);
      pinMode(pinStep[i][j], OUTPUT);
    }
  }

  // 滑台初始化复位
  reset();

  // 计算总行程
  // int pulse_cnt = 0;
  // while(digitalRead(pinLim[StepperTop][RIGHT])){
  //   step(StepperTop, rightDir);
  //   pulse_cnt++;
  // }
  // Serial.print("top pulse number: ");
  // Serial.println(pulse_cnt);
  // int pulse_cnt_b = 0;
  // while(digitalRead(pinLim[StepperBot][RIGHT])){
  //   step(StepperBot, rightDir);
  //   pulse_cnt_b++;
  // }
  // Serial.print("bot pulse number: ");
  // Serial.println(pulse_cnt_b);
}

void step(int stepper, int dir) {
  if ((!digitalRead(pinLim[stepper][LEFT]) && dir == leftDir) || (!digitalRead(pinLim[stepper][RIGHT]) && dir == rightDir))
    return;
  digitalWrite(pinStep[stepper][DIR], dir);   // 方向
  digitalWrite(pinStep[stepper][PUL], HIGH);  // 脉冲
  digitalWrite(pinStep[stepper][PUL], LOW);
  delayMicroseconds(1000);  // 延时
}

void reset(){
  while(digitalRead(pinLim[StepperTop][LEFT])) step(StepperTop, leftDir);
  while(!digitalRead(pinLim[StepperTop][LEFT])) step(StepperTop, rightDir);
  currentPulseTop = 0;
  while(digitalRead(pinLim[StepperBot][LEFT])) step(StepperBot, leftDir);
  while(!digitalRead(pinLim[StepperBot][LEFT])) step(StepperBot, rightDir);
  currentPulseBot = 0;
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
  delay(100);
  coord_y = Serial.parseFloat();
  delay(100);
  hasTarget = true;
}

void loop() {
  if(!hasTarget) return;

  // 滑台移动到指定坐标位置
  sliderMove(StepperBot , int(total_pulse_bot*coord_x), currentPulseBot);
  sliderMove(StepperTop , int(total_pulse_top*coord_y), currentPulseTop);

  digitalWrite(pinCom, LOW);  // 通知抓取
  while(!clawFinish); // 等待抓手抓好

  reset();  // 复位
  digitalWrite(pinCom, HIGH);  // 通知抓手张开
  delay(1500);
  Serial.println("finish!");
}
