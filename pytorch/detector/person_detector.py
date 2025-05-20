from ultralytics import YOLO
import cv2

# 載入預訓練 YOLOv8 模型（最小模型）
model = YOLO("yolov8n.pt")  # 你也可以用 yolov8s.pt、m.pt、l.pt 看你電腦效能


def detect_person(frame):
    results = model(frame, verbose=False)[0]

    for box in results.boxes:
        cls_id = int(box.cls[0])
        if cls_id == 0:  # 人類 class (0)
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            return [x1, y1, x2, y2]

    return None
