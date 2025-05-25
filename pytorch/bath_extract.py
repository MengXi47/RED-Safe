import os
import cv2
import mediapipe as mp
import multiprocessing as mp_pool
from tqdm import tqdm
import numpy as np

# 初始化 MediaPipe Pose 模組
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(static_image_mode=False)

# 資料夾路徑設定
BASE_DIR = r"D:\pytorch\data\raw"
FALL_DIR = os.path.join(BASE_DIR, "fall")
NORMAL_DIR = os.path.join(BASE_DIR, "normal")
PROCESSED_DIR = r"D:\pytorch\data\processed"
os.makedirs(FALL_DIR, exist_ok=True)
os.makedirs(NORMAL_DIR, exist_ok=True)
os.makedirs(PROCESSED_DIR, exist_ok=True)

def process_video(video_path_label):
    """
    處理單支影片，擷取每30幀一組的關鍵點序列。
    回傳格式為 list of (sequence, label)，
    其中 sequence 是30幀的關鍵點list，label是該影片標籤（0或1）
    """
    video_path, label = video_path_label
    cap = cv2.VideoCapture(video_path)
    sequence_data = []
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
            sequence_data.append((sequence, label))
            sequence = []

    cap.release()
    return sequence_data

def collect_videos():
    """
    掃描 fall 與 normal 資料夾，回傳 [(影片路徑, 標籤), ...]
    fall 標籤1，normal 標籤0
    """
    video_label_pairs = []
    for label_name in ['fall', 'normal']:
        folder = os.path.join(BASE_DIR, label_name)
        label = 1 if label_name == 'fall' else 0
        if not os.path.exists(folder):
            print(f"警告：資料夾不存在 {folder}")
            continue
        for file in os.listdir(folder):
            if file.lower().endswith(('.mp4', '.avi', '.mov', '.mkv')):
                video_path = os.path.join(folder, file)
                video_label_pairs.append((video_path, label))
    return video_label_pairs

if __name__ == "__main__":
    video_list = collect_videos()

    X, y = [], []

    # 使用多進程加速處理影片
    with mp_pool.Pool(processes=os.cpu_count()) as pool:
        results = list(tqdm(pool.imap_unordered(process_video, video_list), total=len(video_list)))

    # 將多進程結果合併
    for result in results:
        for sequence, label in result:
            X.append(sequence)
            y.append(label)

    print(f"總共處理序列數量：{len(X)}，標籤數量：{len(y)}")

    # 轉成 numpy 陣列並儲存成 .npy 檔
    X = np.array(X)
    y = np.array(y)
    np.save(os.path.join(PROCESSED_DIR, "X.npy"), X)
    np.save(os.path.join(PROCESSED_DIR, "y.npy"), y)

    print(f"成功儲存 X.npy ({X.shape}), y.npy ({y.shape}) 至 {PROCESSED_DIR}")
