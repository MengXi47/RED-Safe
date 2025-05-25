from yt_dlp import YoutubeDL
from pathlib import Path

def download_video(url, output_path):
    ydl_opts = {
        "format": "18",  # 格式代號18為常見mp4格式（360p/480p）
        "outtmpl": str(output_path)
    }

    with YoutubeDL(ydl_opts) as ydl:
        print(f"開始下載影片: {url}")
        ydl.download([url])
        print(f"已儲存至: {output_path}")

if __name__ == "__main__":
    output_dir = Path("D:/pytorch/data/raw")
    output_dir.mkdir(parents=True, exist_ok=True)

    video_url = "https://youtu.be/9BVWTdnYVIE?si=A2cqxVY-EptvKEqB"
    output_path = output_dir / "demo_fall().mp4"

    download_video(video_url, output_path)
