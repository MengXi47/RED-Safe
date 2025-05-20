import mediapipe as mp
import cv2

pose = mp.solutions.pose.Pose()

def get_pose(frame, bbox=None):
    image_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(image_rgb)
    if results.pose_landmarks:
        return results.pose_landmarks.landmark
    return None
