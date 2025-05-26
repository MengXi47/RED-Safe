import cv2
import mediapipe as mp

# 初始化 MediaPipe Pose（高精度設定）
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(
    static_image_mode=False,
    model_complexity=2,              # 使用最高模型精度
    smooth_landmarks=True,
    enable_segmentation=False,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.7
)

#同時支援 靜態圖片推論 與 影片序列處理
# def init_pose(static_mode=False):
#     return mp_pose.Pose(
#         static_image_mode=static_mode,
#         model_complexity=2,
#         smooth_landmarks=True,
#         enable_segmentation=False,
#         min_detection_confidence=0.7,
#         min_tracking_confidence=0.7
#     )


# 全域變數
X, y = [], []

def process_video(video_path, label):
    global X, y
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
