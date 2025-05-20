import cv2
import mediapipe as mp
from fall_logic.rule_based import is_fall
from utils.data_logger import log_event, save_frame

def main():
    # 初始化 MediaPipe Pose
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(static_image_mode=False,
                        min_detection_confidence=0.5,
                        min_tracking_confidence=0.5)

    # 開啟攝影機
    cap = cv2.VideoCapture(0)  # 0 是預設攝影機
    if not cap.isOpened():
        print("Error: 無法讀取攝影機")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: 無法讀取影像")
            break

        # 轉成 RGB 供 Mediapipe 使用
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # 偵測姿勢
        results = pose.process(frame_rgb)

        if results.pose_landmarks:
            try:
                landmarks = results.pose_landmarks.landmark  # Mediapipe 骨架點列表
                if is_fall(landmarks):
                    print("⚠️ 跌倒偵測！")
                    log_event("Fall detected")
                    save_frame(frame, "output/fall_frame.jpg")
            except Exception as e:
                print(f"[錯誤] 判斷跌倒時出現例外: {e}")

        # 畫出骨架點在影像上
        mp.solutions.drawing_utils.draw_landmarks(
            frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)

        # 顯示影像
        cv2.imshow("Fall Detection", frame)

        # 按 q 鍵離開
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
