import os
import sys
import subprocess

# ========== 路徑設定 ==========
BATCH_SCRIPT = "D:/pytorch/bath_extract.py"  # 這個處理所有影片並輸出 X.npy, y.npy
TRAIN_SCRIPT = "D:/pytorch/train.py"          # 模型訓練腳本

# ========== 執行腳本 ==========
def run_script(script_path, name):
    print(f"\n====== 開始執行：{name} ======")
    result = subprocess.run([sys.executable, script_path], capture_output=True, text=True)
    print(result.stdout)
    if result.returncode != 0:
        print(f"{name} 發生錯誤：\n{result.stderr}")
    else:
        print(f"{name} 執行完成！")

# ========== 主流程 ==========
if __name__ == "__main__":
    run_script(BATCH_SCRIPT, "批次關鍵點擷取 (bath_extract.py)")
    run_script(TRAIN_SCRIPT, "模型訓練 (train.py)")
