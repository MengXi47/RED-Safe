# config.py
DATA_DIR = "D:/fall_detection/data/processed"
RAW_DIR = "D:/fall_detection/data/raw"
MODEL_PATH = "D:/fall_detection/model/best_model.pth"
LOG_DIR = "D:/fall_detection/logs"
LOG_FILE = "D:/fall_detection/output/logs.txt"
SEQ_LEN = 30
INPUT_SIZE = 138
BATCH_SIZE = 32
EPOCHS = 200
ANGLE_PAIRS = [
    # (a, b, c)三點計算夾角，b為頂點
    ("LEFT_HIP", "LEFT_KNEE", "LEFT_ANKLE"),
    ("RIGHT_HIP", "RIGHT_KNEE", "RIGHT_ANKLE"),
    ("LEFT_SHOULDER", "LEFT_HIP", "LEFT_KNEE"),
    ("RIGHT_SHOULDER", "RIGHT_HIP", "RIGHT_KNEE"),
    ("LEFT_SHOULDER", "RIGHT_SHOULDER", "RIGHT_HIP"),
    ("RIGHT_SHOULDER", "LEFT_SHOULDER", "LEFT_HIP"),
]
