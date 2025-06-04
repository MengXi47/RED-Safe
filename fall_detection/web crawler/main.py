from googleapiclient.discovery import build
from yt_dlp import YoutubeDL
from pathlib import Path

def search_youtube_videos(api_key, query, max_results=20):
    youtube = build('youtube', 'v3', developerKey=api_key)
    search_response = youtube.search().list(
        q=query,
        part='id',
        type='video',
        maxResults=max_results,
        videoDuration='short'  # 可選: 'any', 'short', 'medium', 'long'
    ).execute()
    video_urls = []
    for item in search_response.get('items', []):
        video_id = item['id']['videoId']
        video_urls.append(f"https://www.youtube.com/watch?v={video_id}")
    return video_urls

def download_videos(url_list, save_dir, prefix="video"):
    save_dir = Path(save_dir)
    save_dir.mkdir(parents=True, exist_ok=True)
    ydl_opts = {
        "format": "18",
        "outtmpl": str(save_dir / (prefix + "_%(autonumber)03d.%(ext)s")),
        "ignoreerrors": True,
        "quiet": False,
        "noplaylist": True,
    }
    with YoutubeDL(ydl_opts) as ydl:
        for url in url_list:
            try:
                print(f"開始下載: {url}")
                ydl.download([url])
            except Exception as e:
                print(f"❌ 下載失敗: {url}，原因: {e}")

if __name__ == "__main__":
    # 你的API金鑰
    API_KEY = "AIzaSyB59nhS4uyEPcMNzD17eiGVON2S2iTbXSw"  # ←請填入你自己的API Key

    # 關鍵字自動搜尋
    fall_urls = search_youtube_videos(API_KEY, "跌倒影片", max_results=20)
    walk_urls = search_youtube_videos(API_KEY, "模特兒走秀", max_results=20)

    # 跌倒影片下載
    if fall_urls:
        download_videos(fall_urls, "D:/fall_detection/data/raw/fall", prefix="fall")
    else:
        print("❌ 沒有搜尋到跌倒影片")

    # 行走影片下載
    if walk_urls:
        download_videos(walk_urls, "D:/fall_detection/data/raw/normal", prefix="walk")
    else:
        print("❌ 沒有搜尋到行走影片")
