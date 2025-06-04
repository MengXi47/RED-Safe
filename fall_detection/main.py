import cv2
import mediapipe as mp
import torch
import numpy as np
from collections import deque
import config
from models import FallLSTM

def calculate_angle(a, b, c):
    a = np.array([a.x, a.y, a.z])
    b = np.array([b.x, b.y, b.z])
    c = np.array([c.x, c.y, c.z])
    ba = a - b
    bc = c - b
    cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc) + 1e-8)
    angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
    return np.degrees(angle)

def predict_fall(model, sequence, device):
    with torch.no_grad():
        inputs = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)
        outputs = model(inputs)
        probs = torch.softmax(outputs, dim=1)
        pred = torch.argmax(probs, dim=1).item()
        return pred == 1

def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"使用裝置: {device}")

    cap = cv2.VideoCapture(0)
    local_deque = deque(maxlen=config.SEQ_LEN)

    # 保持你的模型載入方式完全不變
    model = FallLSTM(input_size=config.INPUT_SIZE).to(device)
    model.load_state_dict(torch.load(config.MODEL_PATH, map_location=device))
    model.eval()

    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(
        static_image_mode=False,
        model_complexity=1,
        smooth_landmarks=True,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.7
    )

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("攝影機無法讀取")
                break

            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = pose.process(rgb)

            if results.pose_landmarks:
                kpts = []
                for lm in results.pose_landmarks.landmark:
                    kpts.extend([lm.x, lm.y, lm.z, lm.visibility])

                angle_feats = []
                landmarks = results.pose_landmarks.landmark
                for a, b, c in config.ANGLE_PAIRS:
                    idx_a = getattr(mp_pose.PoseLandmark, a).value
                    idx_b = getattr(mp_pose.PoseLandmark, b).value
                    idx_c = getattr(mp_pose.PoseLandmark, c).value
                    angle = calculate_angle(landmarks[idx_a], landmarks[idx_b], landmarks[idx_c])
                    angle_feats.append(angle)

                kpts.extend(angle_feats)
                local_deque.append(kpts)

                if len(local_deque) == config.SEQ_LEN:
                    sequence = np.array(local_deque, dtype=np.float32)
                    is_fall = predict_fall(model, sequence, device)
                    status_text = "Danger!" if is_fall else "Detecting..."
                    color = (0, 0, 255) if is_fall else (0, 255, 0)
                    cv2.putText(frame, status_text, (30, 50),
                                cv2.FONT_HERSHEY_SIMPLEX, 1.5, color, 3)

                mp.solutions.drawing_utils.draw_landmarks(
                    frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
            else:
                cv2.putText(frame, "偵測中...", (30, 50),
                            cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 0), 3)

            cv2.imshow("Camera", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()
        pose.close()

if __name__ == "__main__":
    main()
