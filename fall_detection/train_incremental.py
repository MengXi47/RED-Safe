# train_incremental.py
"""
增量學習（養AI）訓練腳本：每次有新資料時，載入舊模型，混合新舊資料訓練，防止遺忘
"""
import torch
from torch.utils.data import DataLoader, TensorDataset, random_split
import numpy as np
import torch.nn as nn
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
import os
import random

from models import FallLSTM
import config


def load_data_for_incremental(old_data_path, new_data_path, replay_ratio=0.5):
    """
    載入舊資料與新資料，並根據 replay_ratio 混合
    """
    X_old = np.load(os.path.join(old_data_path, "X.npy"), allow_pickle=True)
    y_old = np.load(os.path.join(old_data_path, "y.npy"), allow_pickle=True)
    X_new = np.load(os.path.join(new_data_path, "X_new.npy"), allow_pickle=True)
    y_new = np.load(os.path.join(new_data_path, "y_new.npy"), allow_pickle=True)

    # 經驗回放：隨機抽取部分舊資料
    n_replay = int(len(X_new) * replay_ratio)
    idxs = np.random.choice(len(X_old), min(n_replay, len(X_old)), replace=False)
    X_replay = X_old[idxs]
    y_replay = y_old[idxs]

    # 合併新舊資料
    X_total = np.concatenate([X_new, X_replay], axis=0)
    y_total = np.concatenate([y_new, y_replay], axis=0)
    return X_total, y_total


def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"🚀 使用裝置: {device}")

    # 1. 載入舊模型
    model = FallLSTM(input_size=config.INPUT_SIZE).to(device)
    if os.path.exists(config.MODEL_PATH):
        model.load_state_dict(torch.load(config.MODEL_PATH, map_location=device))
        print("🔄 載入已存在的模型，進行增量學習")
    else:
        print("⚠️ 未找到舊模型，將從頭開始訓練")

    # 2. 載入新資料與 replay 舊資料
    # 新資料請先經過資料前處理，存成 X_new.npy, y_new.npy
    X, y = load_data_for_incremental(config.DATA_DIR, config.DATA_DIR, replay_ratio=0.5)
    X_tensor = torch.tensor(X, dtype=torch.float32)
    y_tensor = torch.tensor(y, dtype=torch.long)
    dataset = TensorDataset(X_tensor, y_tensor)
    train_loader = DataLoader(dataset, batch_size=config.BATCH_SIZE, shuffle=True)

    # 3. 訓練參數
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.0001)  # 學習率較低，避免新資料洗掉舊知識
    writer = SummaryWriter(log_dir=config.LOG_DIR)

    # 4. 增量訓練
    EPOCHS = 20  # 增量訓練可設較少 epoch
    for epoch in range(EPOCHS):
        print(f"\n⏳ 增量訓練第 {epoch + 1}/{EPOCHS} 次...")
        model.train()
        train_loss = 0
        train_loop = tqdm(train_loader, desc=f"增量訓練 Epoch {epoch + 1}", leave=False)
        for inputs, labels in train_loop:
            inputs, labels = inputs.to(device), labels.to(device)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()
            train_loss += loss.item()
            train_loop.set_postfix(loss=loss.item())
        train_loss /= len(train_loader)
        writer.add_scalar("Loss/Incremental_Train", train_loss, epoch + 1)
        print(f"📊 增量訓練 Epoch {epoch + 1} | Train Loss: {train_loss:.4f}")

    # 5. 儲存新模型
    torch.save(model.state_dict(), config.MODEL_PATH)
    print("✅ 增量學習完成，模型已更新。")
    writer.close()


if __name__ == "__main__":
    main()
