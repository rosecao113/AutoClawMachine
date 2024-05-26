# AutoClawMachine
## dir tree

```
AutoClawMachine/
│
├─ arduino.py        定义串口类用于通信
│  
├─ main.py              
│      
├─ vision.py         用传统/深度学习进行目标检测 
│
├─ mech              Arduino程序
│  ├─ duo            使用两块uno
│  │  ├─ claw
│  │  │   └─ claw.ino    控制推杆伸缩和抓手开合
│  │  │    
│  │  └─ slider
│  │      └─ slider.ino  控制滑台移动到指定坐标和复位
│  │
│  └─ solo           使用一块uno
│     └─ ClawMachine
│         └─ ClawMachine.ino  执行完整流程
│  
├─ YOLOv3             网络参数，需要自行另外下载
│  ├─ coco.names      数据集类名
│  ├─ yolov3.cfg      超参数config
│  └─ yolov3.weights  预训练网络权重
│
├─ .gitignore         Git忽略文件
├─ README.md          项目说明
└─ requirements.txt   项目依赖列表
```

<br/>

## run

装环境

```bash
pip install -r requirements.txt
```

跑完整检测+抓取

```bash
python main.py
```

只跑检测

```bash
python vision.py
```


