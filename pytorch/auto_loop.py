import time
import subprocess

# 兩個腳本的完整路徑
run_all_path = r"D:\pytorch\run_all.py"
train_path = r"D:\pytorch\train.py"

def run_script(script_path):
    """執行一個 .py 檔案，等它完成後再繼續"""
    print(f"執行中：{script_path}")
    result = subprocess.run(["python", script_path])
    if result.returncode != 0:
        print(f"⚠️ {script_path} 執行失敗（return code: {result.returncode}）")
    else:
        print(f"✅ 完成：{script_path}")

def main():
    while True:
        # 執行 run_all.py
        run_script(run_all_path)

        # 執行 train.py 十次
        for i in range(5):
            print(f"執行第 {i+1}/5 次 train.py...")
            run_script(train_path)

        print("----- 完成一輪流程，繼續下一輪 -----")
        time.sleep(1)

if __name__ == "__main__":
    main()
