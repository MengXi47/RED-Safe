import os
import zipfile

def package_project():
    zip_name = "fall_detection_project.zip"
    with zipfile.ZipFile(zip_name, 'w', zipfile.ZIP_DEFLATED) as zf:
        for root, _, files in os.walk("."):
            for file in files:
                if file.endswith((".py", ".txt", ".pth", ".npy", ".mp4")):
                    filepath = os.path.join(root, file)
                    # 把相對路徑寫進zip，保持資料夾結構
                    arcname = os.path.relpath(filepath, ".")
                    zf.write(filepath, arcname)
    print(f"專案已成功打包成 {zip_name}")

if __name__ == "__main__":
    package_project()
