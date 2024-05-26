import serial
import time
import struct

class Arduino:
    def  __init__(self, 
                  serialPort = "COM14", 
                  baudRate = 9600,
                  timeout = 0.01
                  ):
        self.serialPort = serialPort
        self.baudRate = baudRate
        self.timeout = timeout
        self.ser = serial.Serial(self.serialPort, self.baudRate, timeout=self.timeout)
        time.sleep(2.5)
        print("Serial open: ", self.ser.is_open)
        print("Settings: Port = %s, BaudRate = %d"%(self.serialPort, self.baudRate))
        # self.ser.set_buffer_size(rx_size=1, tx_size=1)
        self.hasTargets = False
        
    def get_latest_info(self):
        string = ""
        while len(string)==0:
            str_lines = self.ser.readlines()
            string = self.ser.readline().decode('utf-8')        
            try:
                if len(string)==0:
                    string = str_lines[-1].decode('utf-8')
            except:
                pass
        return string.strip()
    
    def sendTargets(self, coordinate):
        self.hasTargets = True
        x = str(round(1-coordinate[0],2))
        y = str(round(1-coordinate[1],2))
        self.ser.write(x.encode('utf-8'))
        time.sleep(2)
        self.ser.write(y.encode('utf-8'))
        time.sleep(2)   
        return
    
    def getCurrentStatus(self):
        if b'f' in self.ser.read_all():
            self.hasTargets = False
        return self.hasTargets
    
    def closeSerial(self):
        self.ser.close()
        return

    




