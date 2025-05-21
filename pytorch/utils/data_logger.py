import os
from datetime import datetime
import cv2

def log_event(event):
    os.makedirs("output", exist_ok=True)
    with open("D:/pytorch/output/logs.txt", "a") as f:
        f.write(f"{datetime.now()}: {event}\n")

def save_frame(frame, filename):
    cv2.imwrite(filename, frame)