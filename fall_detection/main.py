import cv2
import mediapipe as mp
import torch
import numpy as np
from collections import deque
import threading
import time
import queue
import config
from models import FallLSTM

# ----------- 工具函式區 -----------

def calculate_angle(a, b, c):
    """
    計算三點（a, b, c）所形成的夾角（b為頂點）
    """
    a = np.array([a.x, a.y, a.z])
    b = np.array([b.x, b.y, b.z])
    c = np.array([c.x, c.y, c.z])
    ba = a - b
    bc = c - b
    cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc) + 1e-8)
    angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
    return np.degrees(angle)

def predict_fall(model, sequence, device):
    """
    輸入一段關鍵點＋角度序列，回傳是否跌倒
    """
    with torch.no_grad():
        inputs = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)
        outputs = model(inputs)
        probs = torch.softmax(outputs, dim=1)
        pred = torch.argmax(probs, dim=1).item()
        return pred == 1

def list_cameras(max_index=5):
    """
    自動偵測可用攝像頭編號
    """
    available = []
    for i in range(max_index):
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            available.append(i)
            cap.release()
    return available

def select_cameras(available_cams):
    """
    互動式攝像頭選擇介面
    """
    print("\n==============================")
    print("偵測到以下可用攝像頭：")
    for idx in available_cams:
        print(f"[{idx}] 攝像頭 {idx}")
    print("請選擇攝像頭：")
    print("  - 輸入單一數字（如 0）")
    print("  - 多個請用逗號分隔（如 0,1）")
    print("  - 全部請輸入 all")
    print("  - 離開請輸入 q")
    while True:
        choice = input("您的選擇：").strip().lower()
        if choice == 'q':
            exit()
        if choice == 'all':
            return available_cams
        if ',' in choice:
            selected = [int(x) for x in choice.split(',') if x.isdigit()]
        else:
            selected = [int(choice)] if choice.isdigit() else []
        valid = [x for x in selected if x in available_cams]
        if valid:
            return valid
        print("輸入無效，請重新選擇。")

# ----------- 多攝像頭即時推論主流程（含影像佇列） -----------

def camera_loop(camera_index, device, frame_queue):
    """
    每個攝像頭開一個執行緒，進行即時關鍵點＋角度序列蒐集與跌倒推論，結果放入影像佇列
    """
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(static_image_mode=False, model_complexity=2, smooth_landmarks=True,
                        enable_segmentation=False, min_detection_confidence=0.7, min_tracking_confidence=0.7)
    cap = cv2.VideoCapture(camera_index)
    local_deque = deque(maxlen=config.SEQ_LEN)
    model = FallLSTM(input_size=config.INPUT_SIZE).to(device)
    model.load_state_dict(torch.load(config.MODEL_PATH, map_location=device))
    model.eval()

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
            # 加入角度特徵
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

            # 滿一段序列就推論
            if len(local_deque) == config.SEQ_LEN:
                sequence = np.array(local_deque, dtype=np.float32)
                if predict_fall(model, sequence, device):
                    cv2.putText(frame, "Danger!", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 0, 255), 3)
                else:
                    cv2.putText(frame, "Detecting...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3)
        else:
            cv2.putText(frame, "偵測中...", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 0), 3)

        mp.solutions.drawing_utils.draw_landmarks(frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)

        # 放入影像佇列，避免阻塞
        if frame_queue.full():
            try:
                frame_queue.get_nowait()  # 舊畫面出列避免阻塞
            except queue.Empty:
                pass
        frame_queue.put(frame)

    cap.release()

# ----------- 主程式入口 -----------

if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"使用裝置: {device}")

    # 自動偵測所有可用攝像頭
    camera_indices = list_cameras(max_index=5)
    if not camera_indices:
        print("未偵測到可用攝像頭")
        exit()

    # 互動式選擇攝像頭
    selected_cams = select_cameras(camera_indices)
    print("已選擇攝像頭：", selected_cams)

    # 給每個攝像頭一個影像佇列
    frame_queues = {idx: queue.Queue(maxsize=1) for idx in selected_cams}

    threads = []
    try:
        # 啟動攝像頭執行緒
        for idx in selected_cams:
            t = threading.Thread(target=camera_loop, args=(idx, device, frame_queues[idx]))
            t.daemon = True
            t.start()
            threads.append(t)

        # 主執行緒負責顯示影像
        while True:
            for idx in selected_cams:
                if not frame_queues[idx].empty():
                    frame = frame_queues[idx].get()
                    cv2.imshow(f"攝像頭 {idx}", frame)

            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                print("中止所有攝像頭")
                break
            elif key == ord('x'):
                print("強制終止程式")
                exit(0)  # 強制終止不執行 finally

            if not any(t.is_alive() for t in threads):
                print("所有攝像頭執行緒已結束")
                break

            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n正在關閉所有攝像頭...")
    finally:
        cv2.destroyAllWindows()
