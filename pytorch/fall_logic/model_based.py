import torch
import numpy as np
from model.lstm_model import FallLSTM

def load_model(path):
    model = FallLSTM()
    model.load_state_dict(torch.load(path, map_location="cpu"))
    model.eval()
    return model

def predict_fall(model, sequence, device):
    sequence = torch.tensor(sequence, dtype=torch.float32).unsqueeze(0).to(device)
    with torch.no_grad():
        output = model(sequence)
        return output.item() > 0.5