import os
import numpy as np
from utils.extract_keypoints import process_video, X, y

DATA_DIR = "data/processed"
os.makedirs(DATA_DIR, exist_ok=True)

# 執行前清空X,y（如果你多次跑這支程式）
X.clear()
y.clear()

# 呼叫兩個影片路徑跟標籤
process_video("data/raw/fall.mp4", label=1) #跌倒
process_video("data/raw/normal_sample.mp4", label=0) #正常

# 儲存 numpy 陣列
np.save(f"{DATA_DIR}/X.npy", np.array(X))
np.save(f"{DATA_DIR}/y.npy", np.array(y))

print(f"X shape: {np.array(X).shape}, y shape: {np.array(y).shape}")
print("訓練資料產生完成")
