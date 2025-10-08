#!/usr/bin/env python3
import json
import os
import requests

BASE_URL = os.environ.get("RED_API_BASE_URL", "https://api.redsafe-tw.com").rstrip("/")
# BASE_URL = os.environ.get("RED_API_BASE_URL", "https://127.0.0.1").rstrip("/")
TOKEN_FILE = ".tokens.json"


def save_tokens(access_token, refresh_token):
    data = {
        "access_token": access_token,
        "refresh_token": refresh_token
    }
    with open(TOKEN_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"✅ 已儲存 token 至 {TOKEN_FILE}")


def load_tokens():
    if not os.path.exists(TOKEN_FILE):
        return None, None
    with open(TOKEN_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
    return data.get("access_token"), data.get("refresh_token")


def request_json(method, path, token=None, payload=None):
    url = f"{BASE_URL}{path}"
    headers = {}
    if payload is not None:
        payload = {k: v for k, v in payload.items() if v not in (None, "")}
        headers["Content-Type"] = "application/json"
    if token:
        headers["Authorization"] = f"Bearer {token}"

    print(f"\n➡️ {method.upper()} {url}")
    if headers:
        print("Headers:", headers)
    if payload is not None:
        print("Body:", json.dumps(payload, ensure_ascii=False))

    try:
        resp = requests.request(
            method=method,
            url=url,
            headers=headers or None,
            json=payload,
            verify=False,
            timeout=15,
        )
    except requests.RequestException as exc:
        print(f"❌ 網路連線失敗: {exc}")
        return None

    try:
        data = resp.json()
    except ValueError:
        print(f"❌ 非 JSON 回應，HTTP {resp.status_code}: {resp.text[:300]}")
        return None

    if not resp.ok:
        print(f"❌ API 錯誤 (HTTP {resp.status_code}):", data)
        return None

    print("✅ 成功:", data)
    return data


def api_post(path, payload, token=None):
    return request_json("post", path, token=token, payload=payload)


def api_get(path, token=None):
    return request_json("get", path, token=token)


def do_signup():
    email = input("Email: ")
    user_name = input("User Name: ")
    password = input("Password: ")
    payload = {"email": email, "user_name": user_name, "password": password}
    data = api_post("/auth/signup", payload)
    if data:
        print("✅ Signup 成功:", data)


def do_signin():
    email = input("Email: ")
    password = input("Password: ")
    payload = {"email": email, "password": password}
    data = api_post("/auth/signin", payload)
    if data:
        if isinstance(data, dict) and data.get("message") == "150":
            print("⚠️ 此帳號已啟用 OTP，請使用「signin with OTP」選項並輸入動態密碼。")
        else:
            print("✅ Signin 成功:", data)
            save_tokens(data.get("access_token"), data.get("refresh_token"))


def do_refresh():
    _, rt = load_tokens()
    if not rt:
        rt = input("Refresh Token: ")
    payload = {"refresh_token": rt}
    data = api_post("/auth/refresh", payload)
    if data:
        print("✅ Refresh 成功:", data)
        # access_token 更新，refresh token 保留舊的或後端若回傳新的則用新的
        save_tokens(data.get("access_token"), rt)


def do_register_edge():
    edge_id = input("Edge ID: ")
    version = input("Version: ")
    edge_password = input("Initial Edge Password: ")
    payload = {"edge_id": edge_id, "edge_password": edge_password, "version": version}
    data = api_post("/edge/reg", payload)
    if data:
        print("✅ Edge 註冊成功:", data)


def do_send_edge_command():
    edge_id = input("Edge ID: ")
    code = input("Command Code: ")
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    payload = {"edge_id": edge_id, "code": code}
    api_post("/user/edge/command", payload, token=token)


def do_bind_edge():
    edge_id = input("Edge ID to bind: ")
    edge_name = input("Edge Name: ")
    edge_password = input("Edge Password: ")
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    payload = {
        "edge_id": edge_id,
        "edge_name": edge_name,
        "edge_password": edge_password,
    }
    api_post("/user/bind", payload, token=token)

def do_unbind_edge():
    edge_id = input("Edge ID to unbind: ")
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    api_post(f"/user/unbind/{edge_id}", {}, token=token)

def do_list_edges():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    api_get("/user/list/edge_id", token=token)


def do_update_edge_name():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    edge_id = input("Edge ID: ")
    edge_name = input("New Edge Name: ")
    payload = {"edge_id": edge_id, "edge_name": edge_name}
    api_post("/user/update/edge_name", payload, token=token)


def do_update_user_name():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    user_name = input("New User Name: ")
    payload = {"user_name": user_name}
    api_post("/user/update/user_name", payload, token=token)


def do_update_user_password():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    password = input("Current Password: ")
    new_password = input("New Password: ")
    payload = {"password": password, "new_password": new_password}
    api_post("/user/update/password", payload, token=token)


def do_update_edge_password():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    edge_id = input("Edge ID: ")
    edge_password = input("Current Edge Password: ")
    new_edge_password = input("New Edge Password: ")
    payload = {
        "edge_id": edge_id,
        "edge_password": edge_password,
        "new_edge_password": new_edge_password,
    }
    api_post("/user/update/edge_password", payload, token=token)


def do_create_otp():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    data = api_post("/auth/create/otp", {}, token=token)
    if data:
        print("✅ 取得 OTP 秘鑰與備援碼，請妥善保管：", data)


def do_signin_with_otp():
    email = input("Email: ")
    password = input("Password: ")
    otp_code = input("OTP Code (如使用備援碼可留空): ").strip()
    otp_backup_code = input("Backup Code (若使用備援碼請輸入，否則留空): ").strip()
    payload = {
        "email": email,
        "password": password,
        "otp_code": otp_code or None,
        "otp_backup_code": otp_backup_code or None,
    }
    data = api_post("/auth/signin/otp", payload)
    if data:
        print("✅ OTP Signin 成功:", data)
        save_tokens(data.get("access_token"), data.get("refresh_token"))


def do_ios_register():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    ios_device_id = input("iOS Device ID (optional, UUID): ").strip()
    if not ios_device_id:
        ios_device_id = None
    apns_token = input("APNS Token: ")
    device_name = input("Device Name (optional): ").strip() or None
    payload = {
        "apns_token": apns_token,
    }
    if ios_device_id:
        payload["ios_device_id"] = ios_device_id
    if device_name:
        payload["device_name"] = device_name
    api_post("/ios/reg", payload, token=token)


def main():
    requests.packages.urllib3.disable_warnings()

    while True:
        print("\n--- RED API 測試腳本 ---")
        print("1) signup")
        print("2) signin")
        print("3) refresh")
        print("4) register edge")
        print("5) bind edge")
        print("6) unbind edge")
        print("7) list edges")
        print("8) update edge name")
        print("9) update user name")
        print("10) update user password")
        print("11) update edge password")
        print("12) register iOS device")
        print("13) send edge command")
        print("14) create OTP secret")
        print("15) signin with OTP")
        choice = input("選擇操作: ").strip()
        if choice == "1":
            do_signup()
        elif choice == "2":
            do_signin()
        elif choice == "3":
            do_refresh()
        elif choice == "4":
            do_register_edge()
        elif choice == "5":
            do_bind_edge()
        elif choice == "6":
            do_unbind_edge()
        elif choice == "7":
            do_list_edges()
        elif choice == "8":
            do_update_edge_name()
        elif choice == "9":
            do_update_user_name()
        elif choice == "10":
            do_update_user_password()
        elif choice == "11":
            do_update_edge_password()
        elif choice == "12":
            do_ios_register()
        elif choice == "13":
            do_send_edge_command()
        elif choice == "14":
            do_create_otp()
        elif choice == "15":
            do_signin_with_otp()
        else:
            print("無效選項")


if __name__ == "__main__":
    main()
