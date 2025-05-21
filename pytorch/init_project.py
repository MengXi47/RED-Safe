import os

def create_folders():
    folders = [
        "data/raw",
        "data/processed",
        "model",
        "output",
        "output/fall_frames"
    ]
    for folder in folders:
        os.makedirs(folder, exist_ok=True)
    print("資料夾建立完成")

def list_raw_videos():
    raw_dir = "data/raw"
    videos = [f for f in os.listdir(raw_dir) if f.endswith(".mp4")]
    if not videos:
        print("data/raw 目錄中沒有 mp4 影片，請手動放入影片檔")
    else:
        print("發現以下影片：")
        for v in videos:
            print(f" - {os.path.join(raw_dir, v)}")

def main():
    create_folders()
    list_raw_videos()

if __name__ == "__main__":
    main()
