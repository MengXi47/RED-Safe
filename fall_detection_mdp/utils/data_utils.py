# utils/data_utils.py
import cv2
import mediapipe as mp
import numpy as np
import os
from typing import List, Tuple
import config
import math

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

def extract_keypoints_and_angles_from_video(video_path: str, seq_len: int = 30) -> np.ndarray:
    """從影片萃取關鍵點與關節角度序列"""
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(static_image_mode=False, model_complexity=2, smooth_landmarks=True,
                        enable_segmentation=False, min_detection_confidence=0.7, min_tracking_confidence=0.7)
    cap = cv2.VideoCapture(video_path)
    sequences = []
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
            # 角度特徵
            angle_feats = []
            landmarks = result.pose_landmarks.landmark
            for a, b, c in config.ANGLE_PAIRS:
                idx_a = getattr(mp_pose.PoseLandmark, a).value
                idx_b = getattr(mp_pose.PoseLandmark, b).value
                idx_c = getattr(mp_pose.PoseLandmark, c).value
                angle = calculate_angle(landmarks[idx_a], landmarks[idx_b], landmarks[idx_c])
                angle_feats.append(angle)
            keypoints.extend(angle_feats)
            sequence.append(keypoints)
        if len(sequence) == seq_len:
            sequences.append(sequence)
            sequence = []
    cap.release()
    return np.array(sequences)

def batch_process_videos(video_label_list: List[Tuple[str, int]], seq_len: int = 30, n_jobs: int = os.cpu_count()):
    """多核批次處理影片，萃取序列與標籤"""
    from multiprocessing import Pool
    from functools import partial
    X, y = [], []
    with Pool(n_jobs) as pool:
        process_func = partial(extract_keypoints_and_angles_from_video, seq_len=seq_len)
        results = pool.map(process_func, [v[0] for v in video_label_list])
    for idx, seqs in enumerate(results):
        label = video_label_list[idx][1]
        for seq in seqs:
            X.append(seq)
            y.append(label)
    return np.array(X), np.array(y)

def collect_videos(raw_dir: str) -> List[Tuple[str, int]]:
    """收集所有fall/normal影片路徑與標籤"""
    video_label_pairs = []
    for label_name in ['fall', 'normal']:
        folder = os.path.join(raw_dir, label_name)
        label = 1 if label_name == 'fall' else 0
        if not os.path.exists(folder):
            continue
        for file in os.listdir(folder):
            if file.lower().endswith(('.mp4', '.avi', '.mov', '.mkv')):
                video_path = os.path.join(folder, file)
                video_label_pairs.append((video_path, label))
    return video_label_pairs

def load_dataset(data_dir: str):
    """
    載入已處理好的 numpy 資料集
    """
    X = np.load(os.path.join(data_dir, "X.npy"), allow_pickle=True)
    y = np.load(os.path.join(data_dir, "y.npy"), allow_pickle=True)
    return X, y
