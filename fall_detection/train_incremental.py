import torch
from torch.utils.data import DataLoader, TensorDataset, random_split
import numpy as np
import torch.nn as nn
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
import os
import gc
import sys

from models import FallLSTM
import config

def validate_config():
    assert os.path.exists(config.OLD_DATA_DIR), "舊資料路徑不存在"
    assert os.path.exists(config.NEW_DATA_DIR), "新資料路徑不存在"
    assert config.BATCH_SIZE > 0, "批次大小需為正整數"
    assert config.EPOCHS > 0, "訓練週期需為正整數"

def balance_data(X, y):
    """資料平衡：混合過採樣與欠採樣"""
    from collections import Counter
    from sklearn.utils import resample

    class_counts = Counter(y)
    max_count = max(class_counts.values())
    X_balanced, y_balanced = [], []
    for cls in class_counts:
        idxs = np.where(y == cls)[0]
        X_cls = X[idxs]
        y_cls = y[idxs]
        X_res, y_res = resample(X_cls, y_cls, replace=True, n_samples=max_count, random_state=42)
        X_balanced.append(X_res)
        y_balanced.append(y_res)
    return np.concatenate(X_balanced), np.concatenate(y_balanced)

def load_data_for_incremental(old_data_path, new_data_path, replay_ratio=0.5, balance=True):
    """載入並混合新舊資料"""
    X_old = np.load(os.path.join(old_data_path, "X.npy"), allow_pickle=True)
    y_old = np.load(os.path.join(old_data_path, "y.npy"), allow_pickle=True)
    X_new = np.load(os.path.join(new_data_path, "X.npy"), allow_pickle=True)
    y_new = np.load(os.path.join(new_data_path, "y.npy"), allow_pickle=True)

    n_replay = int(len(X_new) * replay_ratio)
    idxs = np.random.choice(len(X_old), min(n_replay, len(X_old)), replace=False)
    X_replay, y_replay = X_old[idxs], y_old[idxs]

    X_total = np.concatenate([X_new, X_replay])
    y_total = np.concatenate([y_new, y_replay])

    return balance_data(X_total, y_total) if balance else (X_total, y_total)

def evaluate(model, dataloader, criterion, device):
    """驗證集評估"""
    model.eval()
    val_loss, correct, total = 0, 0, 0
    with torch.no_grad():
        for inputs, labels in dataloader:
            inputs, labels = inputs.to(device), labels.to(device)
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            val_loss += loss.item() * inputs.size(0)
            _, predicted = torch.max(outputs, 1)
            correct += (predicted == labels).sum().item()
            total += labels.size(0)
            del inputs, labels, outputs, loss, predicted
    val_loss /= total
    return val_loss, correct / total

def main():
    validate_config()
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"🚀 使用裝置: {device}")

    # 模型初始化與安全載入
    model = FallLSTM(config.INPUT_SIZE).to(device)
    if os.path.exists(config.MODEL_PATH):
        print("🔄 載入既有模型進行增量學習")
        try:
            model.load_state_dict(torch.load(config.MODEL_PATH, map_location=device))
        except Exception as e:
            print(f"⚠️ 模型載入失敗: {e}\n將以新模型訓練")
    else:
        print("⚠️ 建立新模型")

    # 資料準備（修正路徑）
    try:
        X, y = load_data_for_incremental(config.OLD_DATA_DIR, config.NEW_DATA_DIR)
    except FileNotFoundError as e:
        print(f"❌ 資料載入失敗: {e}")
        sys.exit(1)

    dataset = TensorDataset(torch.tensor(X, dtype=torch.float32), torch.tensor(y, dtype=torch.long))

    # 資料切分
    train_size = int(0.8 * len(dataset))
    val_size = len(dataset) - train_size
    train_dataset, val_dataset = random_split(dataset, [train_size, val_size])

    train_loader = DataLoader(train_dataset, batch_size=config.BATCH_SIZE, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=config.BATCH_SIZE)

    # 訓練設定
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.0001, weight_decay=1e-4)
    writer = SummaryWriter(log_dir=config.LOG_DIR)

    # 訓練循環
    for epoch in range(config.EPOCHS):
        print(f"\n⏳ Epoch {epoch + 1}/{config.EPOCHS}")
        model.train()
        train_loss, train_correct, total = 0, 0, 0

        for inputs, labels in tqdm(train_loader, desc="Training"):
            inputs, labels = inputs.to(device), labels.to(device)

            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            train_loss += loss.item() * inputs.size(0)
            _, predicted = torch.max(outputs, 1)
            train_correct += (predicted == labels).sum().item()
            total += labels.size(0)

            del inputs, labels, outputs, loss, predicted

        torch.cuda.empty_cache()
        gc.collect()

        train_loss /= total
        train_acc = train_correct / total

        val_loss, val_acc = evaluate(model, val_loader, criterion, device)

        writer.add_scalars('Loss', {'train': train_loss, 'val': val_loss}, epoch)
        writer.add_scalars('Accuracy', {'train': train_acc, 'val': val_acc}, epoch)

        print(f"📊 Train Loss: {train_loss:.4f} | Val Loss: {val_loss:.4f}")
        print(f"🎯 Train Acc: {train_acc:.4f} | Val Acc: {val_acc:.4f}")

    torch.save(model.state_dict(), config.MODEL_PATH)
    print(f"✅ 模型已儲存至 {config.MODEL_PATH}")

    writer.close()
    del model, dataset, train_loader, val_loader
    torch.cuda.empty_cache()

if __name__ == "__main__":
    main()
