import os
import cv2
import mediapipe as mp
import multiprocessing as mp_pool
from tqdm import tqdm
import numpy as np
import gc

SEQ_LEN = 30  # 每段序列長度
MAX_RETRIES = 3  # 影片讀取重試次數

ANGLE_PAIRS = [
    ("LEFT_HIP", "LEFT_KNEE", "LEFT_ANKLE"),
    ("RIGHT_HIP", "RIGHT_KNEE", "RIGHT_ANKLE"),
    ("LEFT_SHOULDER", "LEFT_HIP", "LEFT_KNEE"),
    ("RIGHT_SHOULDER", "RIGHT_HIP", "RIGHT_KNEE"),
    ("LEFT_SHOULDER", "RIGHT_SHOULDER", "RIGHT_HIP"),
    ("RIGHT_SHOULDER", "LEFT_SHOULDER", "LEFT_HIP"),
]

BASE_DIR = r"D:\fall_detection\data\raw"
PROCESSED_DIR = r"D:\fall_detection\data\processed"
os.makedirs(PROCESSED_DIR, exist_ok=True)

# 全域 pose 物件，每個進程只會初始化一次
pose = None

def calculate_angle(a, b, c):
    a = np.array([a.x, a.y, a.z])
    b = np.array([b.x, b.y, b.z])
    c = np.array([c.x, c.y, c.z])
    ba = a - b
    bc = c - b
    cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc) + 1e-8)
    angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
    return np.degrees(angle)

def init_worker():
    global pose
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(
        static_image_mode=False,
        model_complexity=1,
        smooth_landmarks=True,
        enable_segmentation=False,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5
    )

def process_video(video_path_label):
    global pose
    video_path, label = video_path_label
    sequence_data = []
    for attempt in range(MAX_RETRIES):
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            cap.release()
            continue
        sequence = []
        frame_count = 0
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
            frame_count += 1
            if frame_count % 2 != 0:
                continue
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            result = pose.process(rgb)
            if result.pose_landmarks:
                keypoints = np.zeros(33*4 + 6, dtype=np.float32)
                for i, lm in enumerate(result.pose_landmarks.landmark):
                    keypoints[i*4] = lm.x
                    keypoints[i*4+1] = lm.y
                    keypoints[i*4+2] = lm.z
                    keypoints[i*4+3] = lm.visibility
                landmarks = result.pose_landmarks.landmark
                for idx, (a, b, c) in enumerate(ANGLE_PAIRS):
                    idx_a = getattr(mp.solutions.pose.PoseLandmark, a).value
                    idx_b = getattr(mp.solutions.pose.PoseLandmark, b).value
                    idx_c = getattr(mp.solutions.pose.PoseLandmark, c).value
                    angle = calculate_angle(landmarks[idx_a], landmarks[idx_b], landmarks[idx_c])
                    keypoints[33*4 + idx] = angle
                sequence.append(keypoints)
                if len(sequence) == SEQ_LEN:
                    sequence_data.append((np.array(sequence, dtype=np.float32), label))
                    sequence = []
            # 主動釋放 frame 記憶體
            del frame, rgb, result
        cap.release()
        del cap
        gc.collect()  # 強制垃圾回收
        if len(sequence_data) > 0:
            break
    return sequence_data

def collect_videos():
    video_label_pairs = []
    extensions = ('.mp4', '.avi', '.mov', '.mkv', '.flv')
    for label_name in ['fall', 'normal']:
        folder = os.path.join(BASE_DIR, label_name)
        label = 1 if label_name == 'fall' else 0
        if not os.path.exists(folder):
            continue
        for file in os.listdir(folder):
            if file.lower().endswith(extensions):
                video_path = os.path.join(folder, file)
                video_label_pairs.append((video_path, label))
    return video_label_pairs

if __name__ == "__main__":
    print("📦 收集影片清單中...")
    video_list = collect_videos()
    print(f"🎞️ 總共找到 {len(video_list)} 部影片")

    X, y = [], []

    print("🧠 開始多核處理影片並提取關鍵點＋角度序列...")
    with mp_pool.Pool(
        processes=os.cpu_count(),
        initializer=init_worker,
        maxtasksperchild=10  # 每個子進程處理10支影片後自動重啟，防止記憶體累積[3]
    ) as pool:
        chunksize = max(1, len(video_list) // (os.cpu_count() * 4))
        results = pool.imap_unordered(process_video, video_list, chunksize=chunksize)
        with tqdm(total=len(video_list), desc="🚀 處理進度", unit="video") as pbar:
            for result in results:
                for sequence, label in result:
                    X.append(sequence)
                    y.append(label)
                pbar.update(1)

    print(f"\n✅ 完成！總共處理 {len(X)} 筆序列，標籤數：{len(y)}")

    X = np.stack(X, axis=0).astype(np.float32)
    y = np.array(y, dtype=np.int32)
    np.save(os.path.join(PROCESSED_DIR, "X_new.npy"), X)
    np.save(os.path.join(PROCESSED_DIR, "y_new.npy"), y)
    print(f"💾 成功儲存 X.npy {X.shape}，y.npy {y.shape} 至：{PROCESSED_DIR}")
