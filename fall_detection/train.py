# train.py
import sys
import os

# 加入專案根目錄到 Python 路徑（解決 utils 導入問題）
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import torch
from torch.utils.data import DataLoader, TensorDataset, random_split
import torch.nn as nn
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
import numpy as np

from models import FallLSTM
from utils.data_utils import load_dataset
import config

def main():
    # 確保模型目錄存在
    model_dir = os.path.dirname(config.MODEL_PATH)
    if not os.path.exists(model_dir):
        os.makedirs(model_dir)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"🚀 使用裝置: {device}")

    # 載入資料
    print("📂 載入資料中...")
    X, y = load_dataset(config.OLD_DATA_DIR)
    X_tensor = torch.tensor(X, dtype=torch.float32)
    y_tensor = torch.tensor(y, dtype=torch.long)

    dataset = TensorDataset(X_tensor, y_tensor)
    train_size = int(0.8 * len(dataset))
    val_size = len(dataset) - train_size
    train_dataset, val_dataset = random_split(dataset, [train_size, val_size])

    train_loader = DataLoader(train_dataset, batch_size=config.BATCH_SIZE, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=config.BATCH_SIZE, shuffle=False)

    # 初始化模型
    model = FallLSTM(input_size=config.INPUT_SIZE).to(device)

    if os.path.exists(config.MODEL_PATH):
        checkpoint = torch.load(config.MODEL_PATH, map_location=device)
        current_lstm_input_size = model.lstm.input_size
        saved_lstm_input_size = checkpoint['lstm.weight_ih_l0'].shape[1]
        assert current_lstm_input_size == saved_lstm_input_size, \
            f"模型輸入維度不匹配！當前：{current_lstm_input_size}，權重檔：{saved_lstm_input_size}"
        model.load_state_dict(checkpoint)
        print("🔄 載入已存在的模型，繼續訓練")
    else:
        print("✨ 無現有模型，從頭開始訓練")

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    writer = SummaryWriter(log_dir=config.LOG_DIR)

    best_val_loss = float('inf')

    for epoch in range(config.EPOCHS):
        print(f"\n⏳ 執行第 {epoch+1}/{config.EPOCHS} 次訓練...")

        # === 訓練階段 ===
        model.train()
        train_loss = 0
        train_loop = tqdm(train_loader, desc=f"訓練 Epoch {epoch+1}", leave=False)
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

        # === 驗證階段 ===
        model.eval()
        val_loss = 0
        correct = 0
        total = 0
        val_loop = tqdm(val_loader, desc=f"驗證 Epoch {epoch+1}", leave=False)
        with torch.no_grad():
            for inputs, labels in val_loop:
                inputs, labels = inputs.to(device), labels.to(device)
                outputs = model(inputs)
                loss = criterion(outputs, labels)
                val_loss += loss.item()

                _, predicted = torch.max(outputs, 1)
                total += labels.size(0)
                correct += (predicted == labels).sum().item()

                val_loop.set_postfix(loss=loss.item())

        val_loss /= len(val_loader)
        val_acc = correct / total

        print(f"📊 Epoch {epoch+1} | Train Loss: {train_loss:.4f} | Val Loss: {val_loss:.4f} | Val Acc: {val_acc:.4f}")

        writer.add_scalar("Loss/Train", train_loss, epoch+1)
        writer.add_scalar("Loss/Val", val_loss, epoch+1)
        writer.add_scalar("Accuracy/Val", val_acc, epoch+1)

        # 儲存最佳模型
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save(model.state_dict(), config.MODEL_PATH)
            print(f"💾 儲存最佳模型 @ Epoch {epoch+1} - Val Loss: {val_loss:.4f}")

    writer.close()
    print("\n✅ 訓練完成，模型已儲存。")

if __name__ == "__main__":
    main()
