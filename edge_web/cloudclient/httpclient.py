import os
from typing import Any, Dict

import requests

BASE_URL = os.environ.get("RED_API_BASE_URL", "https://api.redsafe-tw.com").rstrip("/")


def _compose_url(path: str) -> str:
    if not path.startswith("/"):
        path = f"/{path}"
    return f"{BASE_URL}{path}"


def update_edge_password(edge_id: str, edge_password: str, new_edge_password: str) -> str:
    """向伺服器發送修改密碼請求，回傳 error_code 或錯誤代碼字串。"""
    url = _compose_url("/edge/update/edge_password")
    headers = {"Content-Type": "application/json"}
    payload = {
        "edge_id": edge_id,
        "edge_password": edge_password,
        "new_edge_password": new_edge_password,
    }

    try:
        response = requests.post(url, headers=headers, json=payload, timeout=10)
    except requests.exceptions.RequestException as exc:
        print("Request error:", exc)
        return "request_failed"

    try:
        data = response.json()
    except ValueError:
        return "invalid_json_response"

    return data.get("error_code", "no_error_code_in_response")


def get_bound_users(edge_id: str) -> Dict[str, Any]:
    """取得綁定該 edge 的使用者清單，成功時回傳遠端 API 的 JSON 結果。"""
    url = _compose_url("/edge/user/list")
    try:
        response = requests.get(url, params={"edge_id": edge_id}, timeout=10)
    except requests.exceptions.RequestException as exc:
        print("Request error:", exc)
        return {"error": "request_failed", "edge_id": edge_id}

    try:
        data = response.json()
    except ValueError:
        return {
            "error": "invalid_json_response",
            "status": response.status_code,
            "edge_id": edge_id,
        }

    if not response.ok:
        return {
            "error": "http_error",
            "status": response.status_code,
            "edge_id": edge_id,
            "data": data,
        }

    return data


def remove_bound_user(edge_id: str, email: str) -> Dict[str, Any]:
    """解除指定使用者與 edge 的綁定，回傳遠端 API 的 JSON 結果。"""
    url = _compose_url("/edge/user/unbind")
    payload = {"edge_id": edge_id, "email": email}
    try:
        response = requests.post(url, json=payload, timeout=10)
    except requests.exceptions.RequestException as exc:
        print("Request error:", exc)
        return {"error": "request_failed", "edge_id": edge_id, "email": email}

    try:
        data = response.json()
    except ValueError:
        return {
            "error": "invalid_json_response",
            "status": response.status_code,
            "edge_id": edge_id,
            "email": email,
        }

    if not response.ok:
        return {
            "error": "http_error",
            "status": response.status_code,
            "edge_id": edge_id,
            "email": email,
            "data": data,
        }

    return data
