import mediapipe as mp
import math

mp_pose = mp.solutions.pose
PoseLandmark = mp_pose.PoseLandmark

def calculate_angle(a, b, c):
    """
    計算三點 a, b, c 間的夾角，b 為頂點
    a, b, c 為有 x, y 屬性的物件
    """
    ang = math.degrees(
        math.atan2(c.y - b.y, c.x - b.x) -
        math.atan2(a.y - b.y, a.x - b.x)
    )
    if ang < 0:
        ang += 360
    if ang > 180:
        ang = 360 - ang
    return ang

def is_fall(landmarks):
    """
    根據 landmarks 判斷是否跌倒，回傳 True/False
    landmarks: mediapipe.pose 的 landmarks 列表
    """

    try:
        left_hip = landmarks[PoseLandmark.LEFT_HIP.value]
        right_hip = landmarks[PoseLandmark.RIGHT_HIP.value]
        left_shoulder = landmarks[PoseLandmark.LEFT_SHOULDER.value]
        right_shoulder = landmarks[PoseLandmark.RIGHT_SHOULDER.value]

        mid_hip_x = (left_hip.x + right_hip.x) / 2
        mid_hip_y = (left_hip.y + right_hip.y) / 2
        mid_shoulder_x = (left_shoulder.x + right_shoulder.x) / 2
        mid_shoulder_y = (left_shoulder.y + right_shoulder.y) / 2

        body_dx = mid_shoulder_x - mid_hip_x
        body_dy = mid_shoulder_y - mid_hip_y

        angle_body = abs(math.degrees(math.atan2(body_dy, body_dx)) - 90)

        if angle_body > 45:
            return True
        else:
            return False

    except Exception as e:
        print(f"[錯誤] 判斷跌倒時出現例外: {e}")
        return False
