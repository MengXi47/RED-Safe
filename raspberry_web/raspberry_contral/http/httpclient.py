import requests

# 向伺服器發送修改密碼請求
#
# 需包含 edge_id edge_password edge_new_password
def update_edge_password(edge_id: str, edge_password: str, new_edge_password: str) -> str:
    url = "https://api.redsafe-tw.com/edge/update/edge_password"
    headers = {
        "Content-Type": "application/json"
    }
    payload = {
        "edge_id": edge_id,
        "edge_password": edge_password,
        "new_edge_password": new_edge_password
    }

    try:
        response = requests.post(url, headers=headers, json=payload, timeout=10)
        try:
            data = response.json()
            return data.get("error_code", "no_error_code_in_response")
        except ValueError:
            return "invalid_json_response"
    except requests.exceptions.RequestException as e:
        print("Request error:", e)
        return "request_failed"