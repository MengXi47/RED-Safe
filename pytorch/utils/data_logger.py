import os

def log_event(message):
    base_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.abspath(os.path.join(base_dir, "..", "output"))
    os.makedirs(output_dir, exist_ok=True)
    log_path = os.path.join(output_dir, "logs.txt")
    with open(log_path, "a", encoding="utf-8") as f:
        f.write(message + "\n")

def save_frame(frame, filename):
    import cv2
    # 確保資料夾存在
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    # 儲存影像檔
    cv2.imwrite(filename, frame)
