"""
YOLO11 Pose + 追蹤 + 跌倒偵測（規則版基線，含視覺化與參數）

功能摘要：
  - 使用 Ultralytics YOLO11 Pose 模型偵測人體邊框與 17 點骨架。
  - 將偵測結果交由 ByteTrack/BOTSORT 追蹤器維持同一人 ID。
  - 針對每位受測者計算傾斜角、寬高比、相對身高與垂直速度，進行規則式跌倒判斷。
  - 支援即時顯示、文字警示與選擇性錄影輸出，方便部署與偵錯。

環境需求：
  pip install ultralytics opencv-python numpy
  （若在無 GUI 的環境請改裝 opencv-python-headless 或關閉 --show）

快速開始：
  1) 在程式內設定 RTSP_URL 為攝影機/串流來源（可含帳密）。
  2) 執行 python main.py --show                # 顯示視窗；伺服器模式可加上 --no-show。
  3) 如需保存推論影像，可加上 --save out.mp4。

重要參數說明：
  --weights        YOLO11 Pose 權重檔，依硬體可換成 s/m/l 版本。
  --tracker        追蹤器設定檔，預設 bytetrack.yaml，另可選 botsort.yaml。
  --win_size       每個 ID 的歷史視窗幀數，對應跌倒判斷的時間窗。
  --alert_frames   規則連續符合的最少幀數；數值越大越不易誤報。
  --cooldown       警報冷卻秒數，避免短時間多次重複通知。
  --imgsz/--conf   YOLO 推論解析度與置信度，依網路與硬體調整。

規則判斷邏輯：
  - 當連續 alert_frames 幀內，人體傾角 angle > 55 度且寬高比 ratio > 1.1。
  - 同時計算垂直位移平均值 v_y，大於 1.5 像素/幀才視為跌倒（避免靜躺誤判）。
  - 可依實際攝影機角度、距離調整門檻降低誤報或漏報。

部署建議：
  - 確保 RTSP 串流延遲穩定；若遇到卡頓可調整攝影機輸出解析度或 fps。
  - 伺服器模式可關閉顯示（--no-show）並透過日誌整合告警系統。
  - 若需更進階的準確率，可將歷史特徵改由 ML 模型判別。
"""

from __future__ import annotations
from ultralytics import YOLO
from collections import deque, defaultdict
import numpy as np
import argparse
import time
import math
import cv2
import os
import sys

# ===== 在程式內指定 RTSP 來源 =====
# 僅支援透過 RTSP 串流輸入影像，請於部署前更新實際位址。
RTSP_URL: str | None = "rtsp://admin:@192.168.47.150:554/stream1"  # e.g. "rtsp://user:pass@ip:554/Streaming/Channels/101"


# ==============================================================


def parse_args():
    p = argparse.ArgumentParser(description="YOLO11 Pose 跌倒偵測")
    p.epilog = "注意：來源已在程式內以 RTSP_URL 指定，不再從終端機傳入。"
    p.add_argument("--weights", default="yolo11n-pose.pt", help="YOLO11 Pose 權重")
    p.add_argument("--tracker", default="bytetrack.yaml", help="追蹤器：bytetrack.yaml 或 botsort.yaml")
    p.add_argument("--conf", type=float, default=0.25, help="置信度閾值")
    p.add_argument("--iou", type=float, default=0.45, help="NMS IOU 閾值")
    p.add_argument("--imgsz", type=int, default=640, help="推論輸入解析度")
    p.add_argument("--device", default=None, help="裝置，例如 0, cpu, cuda:0；留空由庫自動決定")
    p.add_argument("--win_size", type=int, default=30, help="每人歷史視窗幀數（約 1 秒，依 FPS 調整）")
    p.add_argument("--alert_frames", type=int, default=6, help="連續 N 幀符合規則才觸發警報")
    p.add_argument("--cooldown", type=float, default=10.0, help="警報冷卻秒數，避免重複觸發")
    p.add_argument("--show", dest="show", action="store_true", help="顯示視窗影像與標註")
    p.add_argument("--no-show", dest="show", action="store_false", help="不顯示視窗（伺服器/無頭模式）")
    p.set_defaults(show=True)
    p.add_argument("--save", default=None, help="輸出影片路徑（如 out.mp4），不指定則不存檔")
    return p.parse_args()


def body_tilt_angle_deg(kpts: np.ndarray) -> float:
    """以肩-臀向量與『垂直方向』的夾角估算身體傾斜（度數）。
    kpts: (17,2) ndarray（像素座標）。
    回傳值：0 表示垂直站立，越大越傾斜。
    """
    sh = kpts[[5, 6]].mean(axis=0)  # 雙肩中點
    hp = kpts[[11, 12]].mean(axis=0)  # 雙臀中點
    v = hp - sh
    # 與垂直方向的偏角：atan2(x, y)
    angle = abs(math.degrees(math.atan2(float(v[0]), float(v[1]) + 1e-9)))
    return angle


def height_ratio(kpts: np.ndarray, box_h: float) -> float:
    head = kpts[0]
    ankle = kpts[[15, 16]].mean(axis=0)
    head_ankle = float(np.linalg.norm(ankle - head))
    return head_ankle / max(float(box_h), 1e-6)


def compute_features(kpts: np.ndarray, xyxy: np.ndarray):
    x1, y1, x2, y2 = map(float, xyxy)
    w, h = x2 - x1, y2 - y1
    ratio = w / max(h, 1e-6)  # 寬高比，倒地時通常變扁（>1）
    angle = body_tilt_angle_deg(kpts)
    rel_h = height_ratio(kpts, h)
    cx, cy = (x1 + x2) / 2.0, (y1 + y2) / 2.0
    return dict(angle=angle, ratio=ratio, rel_h=rel_h, cx=cx, cy=cy)


def rule_fall_decision(recent_feats: list[dict]) -> bool:
    """簡單規則：傾角大且框變扁（取連續幀穩定）。"""
    if not recent_feats:
        return False
    return all(f["angle"] > 55 and f["ratio"] > 1.1 for f in recent_feats)


def main():
    args = parse_args()

    # 顯示視窗前置檢查（無頭/環境變數）
    if args.show and sys.platform.startswith("linux") and os.environ.get("DISPLAY") is None:
        print("[WARN] 未偵測到 DISPLAY，可能是無頭環境（SSH/WSL）。視窗可能無法顯示。")
    if args.show:
        try:
            cv2.namedWindow("Fall Detection", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Fall Detection", 1280, 720)
        except cv2.error as e:
            print("[ERROR] 無法建立顯示視窗（可能是 headless 套件或缺少 GUI 後端）：", e)
            args.show = False

    # 視訊輸出設定
    writer = None

    model = YOLO(args.weights)

    # 紀錄每個追蹤 ID 的歷史特徵
    history: dict[int, deque] = defaultdict(lambda: deque(maxlen=args.win_size))

    cooldown_until = 0.0
    last_time = time.time()
    fps = 0.0

    if not RTSP_URL:
        # 來源若未設定，直接中止以避免 YOLO track 內部再度拋錯。
        print("[ERROR] RTSP_URL 未設定，請在程式內填入串流位址。")
        sys.exit(1)

    # track: 產生器逐幀輸出
    generator = model.track(
        source=RTSP_URL,
        stream=True,
        tracker=args.tracker,
        conf=args.conf,
        iou=args.iou,
        imgsz=args.imgsz,
        device=args.device,
        verbose=False,
    )

    for res in generator:
        # 影像視覺化（含框與骨架）
        frame = res.plot()

        kps = res.keypoints  # 關鍵點 Results.Keypoints
        boxes = res.boxes  # 邊框與 .id 追蹤 ID
        if kps is None or boxes is None:
            # 更新 FPS 顯示
            now = time.time()
            dt = now - last_time
            fps = 0.9 * fps + 0.1 * (1.0 / dt) if dt > 0 else fps
            last_time = now
            if args.show:
                cv2.putText(frame, f"FPS: {fps:.1f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                try:
                    cv2.imshow("Fall Detection", frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
                except cv2.error as e:
                    print("[ERROR] 無法顯示視窗，可能是 headless 安裝或缺少 GUI 後端：", e)
                    print("提示：若在 Linux/WSL 請確認 DISPLAY；若誤裝 opencv-python-headless 請改裝 opencv-python。")
                    args.show = False
            if writer is not None:
                writer.write(frame)
            continue

        xyxy_all = boxes.xyxy.cpu().numpy() if hasattr(boxes, 'xyxy') else []
        ids = boxes.id.cpu().numpy().astype(int) if (hasattr(boxes, 'id') and boxes.id is not None) else np.arange(
            len(xyxy_all))
        kpts_all = kps.xy.cpu().numpy() if hasattr(kps, 'xy') else []  # (N,17,2)

        # 對每個人計算特徵與決策
        fall_flags = {}
        for i, pid in enumerate(ids):
            xyxy = xyxy_all[i]
            kpt = kpts_all[i]
            feats = compute_features(kpt, xyxy)

            # 垂直速度（像素/幀）
            prev = history[pid][-1] if history[pid] else None
            v_y = 0.0 if prev is None else (feats["cy"] - prev["cy"])  # 往下為正
            feats["v_y"] = float(v_y)
            history[pid].append(feats)

            # 連續 N 幀規則成立 + 略需有向下移動跡象（避免靜態躺姿誤判）
            recent = list(history[pid])[-args.alert_frames:]
            fall_like = rule_fall_decision(recent) and (np.mean([r["v_y"] for r in recent]) > 1.5)
            fall_flags[pid] = bool(fall_like)

        # 警報（全域冷卻）
        now = time.time()
        if now > cooldown_until and any(fall_flags.values()):
            print(
                f"[ALERT] 可能跌倒：IDs={[pid for pid, f in fall_flags.items() if f]}  @ {time.strftime('%Y-%m-%d %H:%M:%S')}")
            cooldown_until = now + float(args.cooldown)

        # 視覺化標註（在已繪製的 frame 上疊字）
        for i, pid in enumerate(ids):
            x1, y1, x2, y2 = map(int, xyxy_all[i])
            label = f"ID {int(pid)}"
            if fall_flags.get(pid, False):
                label += " | FALL"
                color = (0, 0, 255)  # 紅
            else:
                color = (0, 255, 0)  # 綠
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, label, (x1, max(y1 - 10, 0)), cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)

        # FPS 顯示
        dt = now - last_time
        fps = 0.9 * fps + 0.1 * (1.0 / dt) if dt > 0 else fps
        last_time = now
        cv2.putText(frame, f"FPS: {fps:.1f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

        # 初始化與寫入影片
        if writer is None and args.save:
            fourcc = cv2.VideoWriter_fourcc(*"mp4v")
            h, w = frame.shape[:2]
            writer = cv2.VideoWriter(args.save, fourcc, 30.0, (w, h))
        if writer is not None:
            writer.write(frame)

        if args.show:
            try:
                cv2.imshow("Fall Detection", frame)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
            except cv2.error as e:
                print("[ERROR] 無法顯示視窗，可能是 headless 安裝或缺少 GUI 後端：", e)
                print("提示：若在 Linux/WSL 請確認 DISPLAY；若誤裝 opencv-python-headless 請改裝 opencv-python。")
                args.show = False

    if writer is not None:
        writer.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
