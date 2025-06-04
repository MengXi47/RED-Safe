import time
import subprocess

# ====== 專案路徑設定 ======
BATH_EXTRACT_PATH = r"D:\fall_detection\bath_extract_new.py"
INCREMENTAL_PATH   = r"D:\fall_detection\train_incremental.py"

def run_script(script_path):
    print(f"執行中：{script_path}")
    result = subprocess.run(["python", script_path])
    if result.returncode != 0:
        print(f"⚠️ {script_path} 執行失敗（return code: {result.returncode}）")
    else:
        print(f"✅ 完成：{script_path}")

def main():
    repeat_times = 1  # 這裡設定你想要重複的次數
    train_times=5
    for i in range(repeat_times):
        print(f"\n==== 第 {i+1} 輪流程開始 ====")
        run_script(BATH_EXTRACT_PATH)
        for j in range(train_times):
            print(f"\n==== 第 {j+1} 訓練腳本 ====")
            run_script(INCREMENTAL_PATH)
            time.sleep(5)
        print(f"----- 完成第 {i+1} 輪流程，等待 10 秒進入下一輪 -----")
        time.sleep(10)
    print("=== 全部流程執行完畢 ===")

if __name__ == "__main__":
    main()
