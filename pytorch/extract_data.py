import os
import numpy as np
import cv2
import mediapipe as mp

DATA_DIR = "D:/pytorch/data/processed"
os.makedirs(DATA_DIR, exist_ok=True)

# 初始化 MediaPipe Pose（高精度）
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(
    static_image_mode=False,
    model_complexity=2,
    smooth_landmarks=True,
    enable_segmentation=False,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.7
)

# 資料集
X, y = [], []

def process_video(video_path, label):
    cap = cv2.VideoCapture(video_path)
    sequence = []
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        result = pose.process(rgb)
        if result.pose_landmarks:
            keypoints = []
            for lm in result.pose_landmarks.landmark:
                keypoints.extend([lm.x, lm.y, lm.z, lm.visibility])
            sequence.append(keypoints)
        if len(sequence) == 30:
            X.append(sequence)
            y.append(label)
            sequence = []
    cap.release()

# 呼叫影片
process_video("D:/pytorch/data/raw/demo_fall.mp4", label=1)
process_video("D:/pytorch/data/raw/normal_sample.mp4", label=0)

# 儲存
np.save(f"{DATA_DIR}/X.npy", np.array(X))
np.save(f"{DATA_DIR}/y.npy", np.array(y))

print(f"X shape: {np.array(X).shape}, y shape: {np.array(y).shape}")
print("訓練資料產生完成")
