import os
import cv2
import mediapipe as mp
import multiprocessing as mp_pool
from tqdm import tqdm
import numpy as np

# 高精度設定
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(
    static_image_mode=False,
    model_complexity=2,
    smooth_landmarks=True,
    enable_segmentation=False,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.7
)

# 資料夾設定
BASE_DIR = r"D:\pytorch\data\raw"
FALL_DIR = os.path.join(BASE_DIR, "fall")
NORMAL_DIR = os.path.join(BASE_DIR, "normal")
PROCESSED_DIR = r"D:\pytorch\data\processed"
os.makedirs(FALL_DIR, exist_ok=True)
os.makedirs(NORMAL_DIR, exist_ok=True)
os.makedirs(PROCESSED_DIR, exist_ok=True)

def process_video(video_path_label):
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
    video_label_pairs = []
    for label_name in ['fall', 'normal']:
        folder = os.path.join(BASE_DIR, label_name)
        label = 1 if label_name == 'fall' else 0
        if not os.path.exists(folder):
            print(f"⚠️ 資料夾不存在：{folder}")
            continue
        for file in os.listdir(folder):
            if file.lower().endswith(('.mp4', '.avi', '.mov', '.mkv')):
                video_path = os.path.join(folder, file)
                video_label_pairs.append((video_path, label))
    return video_label_pairs

if __name__ == "__main__":
    print("📦 收集影片清單中...")
    video_list = collect_videos()
    print(f"🎞️ 總共找到 {len(video_list)} 部影片")

    X, y = [], []

    print("🧠 開始處理影片並提取關鍵點序列...")
    with mp_pool.Pool(processes=os.cpu_count()) as pool:
        for result in tqdm(pool.imap_unordered(process_video, video_list), total=len(video_list), desc="🚀 處理進度"):
            for sequence, label in result:
                X.append(sequence)
                y.append(label)

    print(f"\n✅ 完成！總共處理 {len(X)} 筆序列，標籤數：{len(y)}")

    X = np.array(X)
    y = np.array(y)
    np.save(os.path.join(PROCESSED_DIR, "X.npy"), X)
    np.save(os.path.join(PROCESSED_DIR, "y.npy"), y)

    print(f"💾 成功儲存 X.npy {X.shape}，y.npy {y.shape} 至：{PROCESSED_DIR}")
