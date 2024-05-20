import os
import time
import numpy as np
import serial
from arduino import Arduino
from vision import con_component_detect, yolo_detect, getImg

def sendCoord(coord, timeout=3000):
    miner = Arduino()
    miner.sendTargets(coord)
    start = time.time()
    while 1:
        time.sleep(1)
        if not miner.getCurrentStatus():
            print("Finished!")
            break
        elif time.time() - start > timeout:
            print("Timeout! Failed")
            break
    miner.closeSerial()


def main(object_detect=yolo_detect, **kwargs):
    try:
        img = getImg()      
        num, coords = object_detect(img, **kwargs)
        if num > 0:
            sendCoord(coords[0])
        else:
            print("Clean! Congrats!")
        return num
    except Exception as e:
        print("Error: ", e)



if __name__ == "__main__":
    while 1:
        ret = main(confidence_thre = 0.3)
        # ret = main(con_component_detect)
        if not ret:
            break
