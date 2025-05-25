import os

def create_folders():
    folders = [
        "D:/pytorch/data/raw/fall",
        "D:/pytorch/data/raw/normal",
        "D:/pytorch/data/processed",
        "D:/pytorch/model",
        "D:/pytorch/output",
        "D:/pytorch/output/fall_frames"
    ]
    for folder in folders:
        os.makedirs(folder, exist_ok=True)
    print("資料夾建立完成")

def list_videos_in_folder(folder_path):
    videos = [f for f in os.listdir(folder_path) if f.endswith(".mp4")]
    if not videos:
        print(f"{folder_path} 目錄中沒有 mp4 影片，請手動放入影片檔")
    else:
        print(f"發現以下 {folder_path} 中的影片：")
        for v in videos:
            print(f" - {os.path.join(folder_path, v)}")

def main():
    create_folders()
    base_raw = "D:/pytorch/data/raw"
    list_videos_in_folder(os.path.join(base_raw, "fall"))
    list_videos_in_folder(os.path.join(base_raw, "normal"))

if __name__ == "__main__":
    main()
