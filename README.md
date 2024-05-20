# AutoClawMachine
Arduino PJ -- Automatic Claw Machine

AutoClawMachine
│  
├─arduino.py                定义串口类用于通信
│  
├─main.py                    
│  
├─vision.py                   用传统/深度学习进行目标检测
│  
├─mech                         Arduino程序
│  ├─claw
│  │   └─claw.ino          控制推杆伸缩和抓手开合
│  │      
│  └─slider
│        └─slider.ino        控制滑台移动到指定坐标和复位
│          
└─YOLOv3                     网络参数，需要自行下载
     ├─coco.names         数据集类名
     ├─yolov3.cfg            超参数config
     └─yolov3.weights	预训练网络权重



跑完整检测+抓取
$ python main.py

只跑检测
$ python vision.py


