import os
import cv2
import mediapipe as mp
import multiprocessing as mp_pool
from tqdm import tqdm
import numpy as np
import math

SEQ_LEN = 30  # 每段序列長度

# 角度特徵設定（6組重要關節）
ANGLE_PAIRS = [
    ("LEFT_HIP", "LEFT_KNEE", "LEFT_ANKLE"),
    ("RIGHT_HIP", "RIGHT_KNEE", "RIGHT_ANKLE"),
    ("LEFT_SHOULDER", "LEFT_HIP", "LEFT_KNEE"),
    ("RIGHT_SHOULDER", "RIGHT_HIP", "RIGHT_KNEE"),
    ("LEFT_SHOULDER", "RIGHT_SHOULDER", "RIGHT_HIP"),
    ("RIGHT_SHOULDER", "LEFT_SHOULDER", "LEFT_HIP"),
]

# 資料夾設定
BASE_DIR = r"D:\fall_detection\data\raw"
FALL_DIR = os.path.join(BASE_DIR, "fall")
NORMAL_DIR = os.path.join(BASE_DIR, "normal")
PROCESSED_DIR = r"D:\fall_detection\data\processed"
os.makedirs(FALL_DIR, exist_ok=True)
os.makedirs(NORMAL_DIR, exist_ok=True)
os.makedirs(PROCESSED_DIR, exist_ok=True)

def calculate_angle(a, b, c):
    """計算三點夾角（b為頂點）"""
    a = np.array([a.x, a.y, a.z])
    b = np.array([b.x, b.y, b.z])
    c = np.array([c.x, c.y, c.z])
    ba = a - b
    bc = c - b
    cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc) + 1e-8)
    angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
    return np.degrees(angle)

def process_video(video_path_label):
    """
    處理單一影片：萃取每段SEQ_LEN幀的關鍵點＋角度特徵序列
    """
    try:
        import mediapipe as mp
        mp_pose = mp.solutions.pose
        pose = mp_pose.Pose(
            static_image_mode=False,
            model_complexity=2,
            smooth_landmarks=True,
            enable_segmentation=False,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.7
        )
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
                # 加入6個關節角度特徵
                angle_feats = []
                landmarks = result.pose_landmarks.landmark
                for a, b, c in ANGLE_PAIRS:
                    idx_a = getattr(mp_pose.PoseLandmark, a).value
                    idx_b = getattr(mp_pose.PoseLandmark, b).value
                    idx_c = getattr(mp_pose.PoseLandmark, c).value
                    angle = calculate_angle(landmarks[idx_a], landmarks[idx_b], landmarks[idx_c])
                    angle_feats.append(angle)
                keypoints.extend(angle_feats)
                sequence.append(keypoints)
            if len(sequence) == SEQ_LEN:
                sequence_data.append((sequence, label))
                sequence = []
        cap.release()
        pose.close()
        return sequence_data
    except Exception as e:
        print(f"❌ 影片處理失敗：{video_path_label[0]}，錯誤：{e}")
        return []

def collect_videos():
    """
    收集所有fall/normal影片路徑與標籤
    """
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

    print("🧠 開始多核處理影片並提取關鍵點＋角度序列...")
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
