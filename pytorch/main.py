import cv2
import mediapipe as mp
import torch
import numpy as np
from collections import deque

mp_pose = mp.solutions.pose
pose = mp_pose.Pose(static_image_mode=False,
                    min_detection_confidence=0.5,
                    min_tracking_confidence=0.5)

deque_kpts = deque(maxlen=30)

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"使用裝置: {device}")

# 載入模型（跟訓練時模型結構要一樣）
class SimpleLSTM(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.lstm = torch.nn.LSTM(input_size=132, hidden_size=64, batch_first=True)
        self.fc = torch.nn.Linear(64, 2)

    def forward(self, x):
        _, (hn, _) = self.lstm(x)
        out = self.fc(hn[-1])
        return out

model = SimpleLSTM().to(device)
model.load_state_dict(torch.load("D:/pytorch/model/trained_model.pth", map_location=device))
model.eval()

def predict_fall(model, sequence, device):
    with torch.no_grad():
        inputs = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)  # (1, 30, 132)
        outputs = model(inputs)
        probs = torch.softmax(outputs, dim=1)
        pred = torch.argmax(probs, dim=1).item()
        return pred == 1  # 1表示跌倒

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: 無法讀取攝影機")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: 無法讀取影像")
        break

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(rgb)

    if results.pose_landmarks:
        kpts = []
        for lm in results.pose_landmarks.landmark:
            kpts.extend([lm.x, lm.y, lm.z, lm.visibility])
        deque_kpts.append(kpts)

        if len(deque_kpts) == 30:
            sequence = np.array(deque_kpts, dtype=np.float32)
            if predict_fall(model, sequence, device):
                cv2.putText(frame, "跌倒偵測!", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 0, 255), 3)
            else:
                cv2.putText(frame, "偵測中...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3)
    else:
        cv2.putText(frame, "偵測中...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3)

    mp.solutions.drawing_utils.draw_landmarks(frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
    cv2.imshow("Fall Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
