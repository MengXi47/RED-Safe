#!/usr/bin/env python3
import json
import os
import sys
import requests

BASE_URL = "https://api.redsafe-tw.com"
#BASE_URL = "https://localhost"
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


def api_post(path, payload, token=None):
    url = BASE_URL.rstrip("/") + path
    headers = {
        "Content-Type": "application/json"
    }
    if token:
        headers["Authorization"] = f"Bearer {token}"

    print(url)
    print(headers)

    try:
        resp = requests.post(url, verify=False, headers=headers, json=payload, timeout=15)
    except requests.RequestException as e:
        print(f"❌ 網路連線失敗: {e}")
        return None

    try:
        data = resp.json()
    except ValueError:
        print(f"❌ 非 JSON 回應，HTTP {resp.status_code}: {resp.text[:300]}")
        return None

    if not resp.ok:
        print("❌ API 錯誤：", data)
        return None
    return data


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
    payload = {"edge_id": edge_id, "version": version}
    data = api_post("/edge/reg", payload)
    if data:
        print("✅ Edge 註冊成功:", data)


def do_bind_edge():
    edge_id = input("Edge ID to bind: ")
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    # 假如你的 API route 是 POST /bind/{edge_id}
    path = f"/user/bind/{edge_id}"
    payload = {}  # 如果你的 bind API 不需要 body，可以傳空 dict
    data = api_post(path, payload, token=token)
    if data:
        print("✅ Bind 成功:", data)


# 新增 do_unbind_edge
def do_unbind_edge():
    edge_id = input("Edge ID to unbind: ")
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    # 假如你的 API route 是 POST /unbind/{edge_id}
    path = f"/user/unbind/{edge_id}"
    payload = {}  # 如果你的 unbind API 不需要 body，可以傳空 dict
    data = api_post(path, payload, token=token)
    if data:
        print("✅ Unbind 成功:", data)

def do_list_edges():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    url = BASE_URL.rstrip("/") + "/user/list/edge_id"
    headers = {
        "Authorization": f"Bearer {token}"
    }
    try:
        resp = requests.get(url, verify=False, headers=headers, timeout=15)
    except requests.RequestException as e:
        print(f"❌ 網路連線失敗: {e}")
        return
    try:
        data = resp.json()
    except ValueError:
        print(f"❌ 非 JSON 回應，HTTP {resp.status_code}: {resp.text[:300]}")
        return
    if not resp.ok:
        print("❌ API 錯誤：", data)
        return
    print("✅ List Edges 成功:", data)


def do_update_edge_name():
    token, _ = load_tokens()
    if not token:
        print("❌ 請先 signin 取得 access_token")
        return
    edge_id = input("Edge ID: ")
    edge_name = input("New Edge Name: ")
    payload = {"edge_id": edge_id, "edge_name": edge_name}
    data = api_post("/user/update/edge_name", payload, token=token)
    if data:
        print("✅ Update Edge Name 成功:", data)


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
        else:
            print("無效選項")


if __name__ == "__main__":
    main()