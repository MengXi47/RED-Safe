from django import forms
import requests

class ApiProfileForm(forms.Form):
    username = forms.CharField(label='使用者名稱', max_length=150, required=True)
    email = forms.EmailField(label='Email', required=True)

    def clean(self):
        cleaned_data = super().clean()
        username = cleaned_data.get('username')
        email = cleaned_data.get('email')

        api_url = 'https://你的-api-網址/api/profile/check'  # ← 請自行填寫

        try:
            response = requests.post(api_url, json={
                'username': username,
                'email': email
            })
            if response.status_code != 200:
                raise forms.ValidationError('API 伺服器異常，請稍後再試。')
            result = response.json()
            # 根據 API 回傳結果做驗證
            if result.get('error'):
                raise forms.ValidationError(result['error'])
        except Exception as e:
            raise forms.ValidationError(f'API 連線失敗：{e}')

        return cleaned_data
