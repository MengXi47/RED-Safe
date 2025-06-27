from django.shortcuts import render
from django.http import JsonResponse
import requests

def index(request):
    return render(request, 'index.html')

def about(request):
    return render(request, 'about/about.html')
def profile(request):
    return render(request, 'profile.html')

# 會員資料 API 代理
# def profile_api(request):
#     if request.method == 'POST':
#         # 假設前端傳來 token 或 email
#         token = request.POST.get('token')
#         # === 這裡填你的 API 位置 ===
#         api_url = 'https://你的-api-網址/api/profile'  # <--- 請自行填寫
#         try:
#             response = requests.post(api_url, data={'token': token})
#             return JsonResponse(response.json())
#         except Exception as e:
#             return JsonResponse({'error': f'API 連線失敗：{e}'}, status=500)
#     return JsonResponse({'error': '只支援 POST'}, status=405)
