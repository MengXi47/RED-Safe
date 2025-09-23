# predict.py
"""
單一序列預測跌倒
"""
import torch
import numpy as np
from models import FallLSTM
import config

def predict_fall(sequence: np.ndarray, device: str = "cpu") -> bool:
    """
    預測單一序列是否跌倒
    """
    model = FallLSTM(input_size=config.INPUT_SIZE)
    model.load_state_dict(torch.load(config.MODEL_PATH, map_location=device))
    model.eval()
    sequence = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)
    with torch.no_grad():
        output = model(sequence)
        pred = torch.argmax(torch.softmax(output, dim=1), dim=1).item()
        return pred == 1  # 1 = fall

# 範例用法
if __name__ == "__main__":
    # 載入一筆測試序列
    test_seq = np.load("D:/pytorch/data/processed/X.npy")[0]  # 取第一筆
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    is_fall = predict_fall(test_seq, device)
    print("跌倒" if is_fall else "正常")
