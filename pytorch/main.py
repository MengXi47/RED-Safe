import cv2
import mediapipe as mp
import torch
import numpy as np
from collections import deque
import threading

# === 控制模式 ===
# 0 = 使用單個攝像頭（手動選擇）
# 1 = 同時開啟所有可用攝像頭
mode = 0

# 初始化 MediaPipe Pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(static_image_mode=False,
                    min_detection_confidence=0.5,
                    min_tracking_confidence=0.5)

# 模型架構
class SimpleLSTM(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.lstm = torch.nn.LSTM(input_size=132, hidden_size=64, batch_first=True)
        self.fc = torch.nn.Linear(64, 2)

    def forward(self, x):
        _, (hn, _) = self.lstm(x)
        out = self.fc(hn[-1])
        return out

# 模型與裝置初始化
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"使用裝置: {device}")

model = SimpleLSTM().to(device)
model.load_state_dict(torch.load("D:/pytorch/model/trained_model.pth", map_location=device))
model.eval()

# 預測函數
def predict_fall(model, sequence, device):
    with torch.no_grad():
        inputs = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)  # (1, 30, 132)
        outputs = model(inputs)
        probs = torch.softmax(outputs, dim=1)
        pred = torch.argmax(probs, dim=1).item()
        return pred == 1  # 1 表示跌倒

# 列出可用攝像頭
def list_cameras(max_index=5):
    available = []
    for i in range(max_index):
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            available.append(i)
            cap.release()
    return available

# 每個攝像頭的處理函數
def camera_loop(camera_index):
    cap = cv2.VideoCapture(camera_index)
    local_deque = deque(maxlen=30)

    while True:
        ret, frame = cap.read()
        if not ret:
            print(f"攝像頭 {camera_index} 無法讀取影像")
            break

        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = pose.process(rgb)

        if results.pose_landmarks:
            kpts = []
            for lm in results.pose_landmarks.landmark:
                kpts.extend([lm.x, lm.y, lm.z, lm.visibility])
            local_deque.append(kpts)

            if len(local_deque) == 30:
                sequence = np.array(local_deque, dtype=np.float32)
                if predict_fall(model, sequence, device):
                    cv2.putText(frame, "Danger!", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 0, 255), 3)
                else:
                    cv2.putText(frame, "Detecting...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3)
        else:
            cv2.putText(frame, "偵測中...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 0), 3)

        mp.solutions.drawing_utils.draw_landmarks(frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
        cv2.imshow(f"攝像頭 {camera_index}", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            print(f"攝像頭 {camera_index} 中止")
            break

    cap.release()
    cv2.destroyWindow(f"攝像頭 {camera_index}")

# 主程式
if __name__ == "__main__":
    print("正在尋找可用攝像頭...")
    camera_indices = list_cameras(max_index=5)
    if not camera_indices:
        print("未偵測到可用攝像頭")
        exit()

    if mode == 0:
        # 單攝像頭模式
        print("可用攝像頭：", camera_indices)
        selected = int(input("請輸入要使用的攝像頭編號："))
        if selected not in camera_indices:
            print("選擇的攝像頭無效")
            exit()
        camera_loop(selected)

    elif mode == 1:
        # 多攝像頭模式
        print("啟動所有攝像頭：", camera_indices)
        threads = []
        for idx in camera_indices:
            t = threading.Thread(target=camera_loop, args=(idx,))
            t.start()
            threads.append(t)

        # 等待所有執行緒完成
        for t in threads:
            t.join()

    cv2.destroyAllWindows()
