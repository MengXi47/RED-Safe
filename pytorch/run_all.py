import os
import sys
import subprocess

# 1. 清空 X, y 並提取關鍵點 (整合 extract_keypoints.py 內容)
from utils.extract_keypoints import process_video, X, y

def extract_keypoints_and_save():
    DATA_RAW = "D:/pytorch/data/raw"
    DATA_PROCESSED = "D:/pytorch/data/processed"
    os.makedirs(DATA_PROCESSED, exist_ok=True)

    # 清空舊資料
    X.clear()
    y.clear()

    # 影片與標籤，這裡可以自己加影片路徑
    videos_labels = [
        (f"{DATA_RAW}/demo_fall.mp4", 1),
        (f"{DATA_RAW}/normal_sample.mp4", 0),
    ]

    for video_path, label in videos_labels:
        print(f"處理影片: {video_path} 標籤: {label}")
        process_video(video_path, label)

    # 存 npy 檔案
    import numpy as np
    np.save(f"{DATA_PROCESSED}/X.npy", np.array(X))
    np.save(f"{DATA_PROCESSED}/y.npy", np.array(y))
    print(f"X shape: {np.array(X).shape}, y shape: {np.array(y).shape}")
    print("關鍵點資料處理完成，已儲存成 npy 檔")

# 2. 執行 extract_data.py （如果你 extract_data.py 是用來轉換格式，這步可以跳過，因為上面已完成）
# 這裡不呼叫 extract_data.py ，因為前面已經做了 npy 儲存

# 3. 執行 train.py 來訓練模型
def train_model():
    train_script = "D:/pytorch/train.py"
    print("開始訓練模型...")
    result = subprocess.run([sys.executable, train_script], capture_output=True, text=True)
    print(result.stdout)
    if result.returncode != 0:
        print("訓練過程發生錯誤:", result.stderr)
    else:
        print("模型訓練完成")

if __name__ == "__main__":
    extract_keypoints_and_save()
    train_model()
