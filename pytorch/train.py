import torch
from torch.utils.data import DataLoader, TensorDataset, random_split
import numpy as np
import torch.nn as nn
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
import os

def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"🚀 使用裝置: {device}")

    # 載入資料 (請確認路徑與格式正確)
    print("📂 載入資料中...")
    X = np.load("D:/pytorch/data/processed/X.npy", allow_pickle=True)
    y = np.load("D:/pytorch/data/processed/y.npy", allow_pickle=True)

    X_tensor = torch.tensor(X, dtype=torch.float32)
    y_tensor = torch.tensor(y, dtype=torch.long)

    dataset = TensorDataset(X_tensor, y_tensor)

    train_size = int(0.8 * len(dataset))
    val_size = len(dataset) - train_size
    train_dataset, val_dataset = random_split(dataset, [train_size, val_size])

    train_loader = DataLoader(train_dataset, batch_size=32, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=32, shuffle=False)

    # 定義模型
    class SimpleLSTM(nn.Module):
        def __init__(self):
            super().__init__()
            self.lstm = nn.LSTM(input_size=132, hidden_size=64, batch_first=True)
            self.fc = nn.Linear(64, 2)

        def forward(self, x):
            _, (hn, _) = self.lstm(x)
            out = self.fc(hn[-1])
            return out

    model = SimpleLSTM().to(device)

    # 模型檔案路徑
    model_path = "D:/pytorch/model/best_model.pth"

    # 載入已有模型
    if os.path.exists(model_path):
        model.load_state_dict(torch.load(model_path, map_location=device))
        print("🔄 載入已存在的模型，繼續訓練")
    else:
        print("✨ 無現有模型，從頭開始訓練")

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    log_dir = "D:/pytorch/logs"
    os.makedirs(log_dir, exist_ok=True)
    writer = SummaryWriter(log_dir=log_dir)

    best_val_loss = float('inf')

    for epoch in range(200):
        print(f"⏳ 執行第 {epoch+1}/200 次訓練...")
        model.train()
        train_loss = 0
        for inputs, labels in train_loader:
            inputs, labels = inputs.to(device), labels.to(device)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()
            train_loss += loss.item()

        train_loss /= len(train_loader)

        model.eval()
        val_loss = 0
        correct = 0
        total = 0
        with torch.no_grad():
            for inputs, labels in val_loader:
                inputs, labels = inputs.to(device), labels.to(device)
                outputs = model(inputs)
                loss = criterion(outputs, labels)
                val_loss += loss.item()

                _, predicted = torch.max(outputs, 1)
                total += labels.size(0)
                correct += (predicted == labels).sum().item()

        val_loss /= len(val_loader)
        val_acc = correct / total

        print(f" Epoch {epoch+1}, Train Loss: {train_loss:.4f}, Val Loss: {val_loss:.4f}, Val Acc: {val_acc:.4f}")

        writer.add_scalar("Loss/Train", train_loss, epoch+1)
        writer.add_scalar("Loss/Val", val_loss, epoch+1)
        writer.add_scalar("Accuracy/Val", val_acc, epoch+1)

        # 儲存最佳模型
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save(model.state_dict(), model_path)
            print(f"💾 Epoch {epoch+1} - 儲存最佳模型，Val Loss: {val_loss:.4f}")

    writer.close()
    print("✅ 訓練完成，模型已儲存。")

if __name__ == "__main__":
    main()
