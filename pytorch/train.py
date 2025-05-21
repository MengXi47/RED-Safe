import torch
from torch.utils.data import DataLoader, TensorDataset
import numpy as np
import torch.nn as nn
import torch.optim as optim

# 假設你已經有 X.npy 和 y.npy
def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"使用裝置: {device}")

    # 載入資料
    X = np.load("D:/pytorch/data/processed/X.npy", allow_pickle=True)
    y = np.load("D:/pytorch/data/processed/y.npy", allow_pickle=True)

    # 把 numpy 轉成 tensor
    X_tensor = torch.tensor(X, dtype=torch.float32).to(device)
    y_tensor = torch.tensor(y, dtype=torch.long).to(device)

    dataset = TensorDataset(X_tensor, y_tensor)
    loader = DataLoader(dataset, batch_size=32, shuffle=True)

    # 簡單 LSTM 模型範例（請根據你模型做調整）
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
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    model.train()
    for epoch in range(10):
        total_loss = 0
        for inputs, labels in loader:
            inputs, labels = inputs.to(device), labels.to(device)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()
        print(f"Epoch {epoch+1}, Loss: {total_loss/len(loader):.4f}")

    torch.save(model.state_dict(), "D:/pytorch/model/trained_model.pth")

if __name__ == "__main__":
    main()
