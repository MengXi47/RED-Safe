import torch.nn as nn

class FallLSTM(nn.Module):
    def __init__(self):
        super(FallLSTM, self).__init__()
        self.lstm = nn.LSTM(input_size=132, hidden_size=64, num_layers=2, batch_first=True)
        self.fc = nn.Linear(64, 1)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        out, _ = self.lstm(x)
        out = self.fc(out[:, -1, :])
        return self.sigmoid(out)