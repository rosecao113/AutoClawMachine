import cv2
import numpy as np	
import time	
import os
import win32api, win32con, win32gui
from PIL import Image, ImageGrab

'''
VISION -- get Target Coordinates:
    1) Conventional DIP using morphological operations and connected component detection
or  2) Inference process using YOLOv3 pretrained on the COCO dataset(80 classes)

getImg -- get image from camera
'''

def con_component_detect(img):	
    # filter
    img = cv2.medianBlur(img, 5)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # thresholding
    # _, binary = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)
    _, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)

    # Morphological opening (erosion -> dilation )
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    bin_op = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel, iterations=2)

    # connected component detection
    num_labels, _, _, centroids = cv2.connectedComponentsWithStats(bin_op, connectivity=8)
    if num_labels > 1:
        centroids = np.delete(centroids, [0], axis=0)
        centroids[:, 0] /=  bin_op.shape[1]
        centroids[:, 1] /=  bin_op.shape[0]
        # centroids = [(int(np.round(x * 255)), int(np.round(y * 255))) for x, y in centroids]
    return num_labels-1, centroids
    

def yolo_detect(img,	
                pathOut = None,	
                label_path = './YOLOv3/coco.names',	
                config_path = './YOLOv3/yolov3.cfg',	
                weights_path = './YOLOv3/yolov3.weights',	
                confidence_thre = 0.5,	
                nms_thre = 0.3,	
                jpg_quality = 80):	
    '''	
    pathIn: img path	
    pathOut: save path	
    confidence_thre: confidence threshold(0,1)	
    nms_thre: NMS threshold
    '''	
    # load labels
    LABELS = open(label_path).read().strip().split("\n")	
    nclass = len(LABELS)		
    np.random.seed(42)	
    COLORS = np.random.randint(0, 255, size=(nclass, 3), dtype='uint8')		
    (H, W) = img.shape[:2]	
    # load model
    net = cv2.dnn.readNetFromDarknet(config_path, weights_path)	
    layernames = net.getLayerNames()	
    ln = [layernames[i - 1] for i in net.getUnconnectedOutLayers()]	
    # forward process	
    blob = cv2.dnn.blobFromImage(img, 1 / 255.0, (416, 416), swapRB=True, crop=False)	
    net.setInput(blob)	
    start = time.time()	
    layerOutputs = net.forward(ln)	
    end = time.time()	
    # time cost	
    print('single detection spends {:.2f} s'.format(end - start))	
    # bounding box, confidence score, class	
    boxes = []	
    confidences = []	
    classIDs = []	
    for output in layerOutputs:	
        for detection in output:		
            scores = detection[5:]	
            classID = np.argmax(scores)	
            confidence = scores[classID]	
            if confidence > confidence_thre:	
                box = detection[0:4] * np.array([W, H, W, H])	
                (centerX, centerY, width, height) = box.astype("int")	
                # origin	
                x = int(centerX - (width / 2))	
                y = int(centerY - (height / 2))		
                boxes.append([x, y, int(width), int(height)])	
                confidences.append(float(confidence))	
                classIDs.append(classID)	
    # NMS algorithm to reduce overlapping bounding boxes
    idxs = cv2.dnn.NMSBoxes(boxes, confidences, confidence_thre, nms_thre)	
    # plot
    coord_list = []
    if len(idxs) > 0:		
        for i in idxs.flatten():	
            (x, y) = (boxes[i][0], boxes[i][1])	
            (w, h) = (boxes[i][2], boxes[i][3])	
            coord_list.append(((x+w/2)/W, (y+h/2)/H))	
            color = [int(c) for c in COLORS[classIDs[i]]]	
            cv2.rectangle(img, (x, y), (x + w, y + h), color, 2)	
            text = '{}: {:.3f}'.format(LABELS[classIDs[i]], confidences[i])	
            (text_w, text_h), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 2)	
            cv2.rectangle(img, (x, y-text_h-baseline), (x + text_w, y), color, -1)	
            cv2.putText(img, text, (x, y-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2)	
    if pathOut :	
        cv2.imwrite(pathOut, img, [int(cv2.IMWRITE_JPEG_QUALITY), jpg_quality])
    cv2.namedWindow('YOLO Detection', cv2.WINDOW_AUTOSIZE)
    cv2.setWindowProperty('YOLO Detection', cv2.WND_PROP_TOPMOST, 1)
    cv2.imshow('YOLO Detection', img)
    cv2.waitKey()
    cv2.destroyAllWindows()
    return len(idxs), coord_list


def getImg(GoPro=True, save_path=None, device_id=0):
    if GoPro:
        handle = win32gui.FindWindow(0, '预览')
        # restore minimized window
        win32gui.SendMessage(handle, win32con.WM_SYSCOMMAND, win32con.SC_RESTORE, 0) 
        win32gui.SetForegroundWindow(handle)
        time.sleep(0.8)
        boundingbox = win32gui.GetWindowRect(handle)
        img = ImageGrab.grab(boundingbox)
        if save_path:
            img.save(os.path.join(save_path, 'camera_shot.png'))
        return cv2.cvtColor(np.asarray(img), cv2.COLOR_RGB2BGR)
    else:
        cap = cv2.VideoCapture(device_id)
        time.sleep(0.5)
        _, img = cap.read()
        if save_path:
            cv2.imwrite(os.path.join(save_path, 'camera_shot.png'), img)
        cv2.namedWindow('camera', cv2.WINDOW_AUTOSIZE)
        cv2.imshow('camera', img)
        cv2.waitKey()
        cap.release() 
        return img


if __name__ == "__main__":
    try:
        img = getImg()      
        num, coords = yolo_detect(img, confidence_thre = 0.3)
        print("Number of targets: ", num)
    except Exception as e:
        print("Error: ", e)



