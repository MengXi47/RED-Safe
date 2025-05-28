// sign.js
let signControl = 0; // 0=只阻止表單送出, 1=發送API

const ERROR_MESSAGES = {
  0:   "成功",
  99:  "未知的 API 端點",
  100: "JSON 格式錯誤",
  101: "序號格式錯誤",
  102: "APNS Token 格式錯誤",
  103: "Email 格式錯誤",
  104: "使用者名稱格式錯誤",
  105: "密碼格式錯誤",
  201: "Email 或密碼錯誤",
  301: "裝置已註冊",
  302: "Email 已註冊",
  303: "綁定已存在",
  401: "缺少序號或版本資訊",
  402: "缺少 Email、使用者名稱或密碼",
  403: "缺少 Email 或密碼",
  404: "缺少使用者 ID 或 APNS Token",
  405: "缺少使用者 ID 或序號",
  500: "伺服器內部錯誤"
};

function getErrorMessage(code) {
  return ERROR_MESSAGES[code] || `未知錯誤（代碼：${code}）`;
}

document.addEventListener('DOMContentLoaded', () => {
  // 彈窗開關
  const signBtn = document.getElementById('signBtn');
  const signModal = document.getElementById('signModal');
  const closeModal = document.getElementById('closeModal');
  if (signBtn && signModal && closeModal) {
    signBtn.onclick = () => signModal.style.display = 'block';
    closeModal.onclick = () => signModal.style.display = 'none';
    window.onclick = (e) => {
      if (e.target === signModal) signModal.style.display = 'none';
    };
  }

  // 登入/註冊 tab 切換
  const loginTab = document.getElementById('loginTab');
  const signupTab = document.getElementById('signupTab');
  const loginForm = document.getElementById('loginForm');
  const registerForm = document.getElementById('registerForm');
  if (loginTab && signupTab && loginForm && registerForm) {
    loginTab.onclick = () => {
      loginTab.classList.add('active');
      signupTab.classList.remove('active');
      loginForm.classList.add('active');
      registerForm.classList.remove('active');
    };
    signupTab.onclick = () => {
      signupTab.classList.add('active');
      loginTab.classList.remove('active');
      registerForm.classList.add('active');
      loginForm.classList.remove('active');
    };

    // 登入表單送出
    loginForm.onsubmit = async (e) => {
      e.preventDefault();
      if (signControl === 0) {
        alert('目前為測試模式，阻止送出，不發送API');
        return;
      }
      const email = loginForm.querySelector('input[type="text"], input[type="email"]').value;
      const password = loginForm.querySelector('input[type="password"]').value;

      try {
        const response = await fetch('https://api.redsafe-tw.com/user/signin', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ email, password })
        });

        if (!response.ok) {
          alert('HTTP 錯誤：' + response.status);
          return;
        }

        const body = await response.json();

        if (typeof body.error_code !== "undefined") {
          if (body.error_code === 0) {
            alert('登入成功！\n用戶名稱：' + body.user_name + '\nEmail：' + body.email);
            signModal.style.display = 'none';
          } else {
            alert(getErrorMessage(body.error_code));
          }
        } else {
          alert('伺服器回傳格式異常');
        }
      } catch (err) {
        alert('登入失敗，請檢查網路或稍後再試');
      }
    };

    // 註冊表單送出
    registerForm.onsubmit = async (e) => {
      e.preventDefault();
      if (signControl === 0) {
        alert('目前為測試模式，阻止送出，不發送API');
        return;
      }
      const inputs = registerForm.querySelectorAll('input');
      const email = inputs[0].value;
      const user_name = inputs[1].value;
      const password = inputs[2].value;
      const confirm = inputs[3].value;
      if (password !== confirm) {
        alert('兩次密碼輸入不一致');
        return;
      }
      try {
        const response = await fetch('https://api.redsafe-tw.com/user/signup', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ email, user_name, password })
        });

        if (!response.ok) {
          alert('HTTP 錯誤：' + response.status);
          return;
        }

        const body = await response.json();

        if (typeof body.error_code !== "undefined") {
          if (body.error_code === 0) {
            alert('註冊成功，請登入');
            loginTab.classList.add('active');
            signupTab.classList.remove('active');
            loginForm.classList.add('active');
            registerForm.classList.remove('active');
          } else {
            alert(getErrorMessage(body.error_code));
          }
        } else {
          alert('伺服器回傳格式異常');
        }
      } catch (err) {
        alert('註冊失敗，請檢查網路或稍後再試');
      }
    };
  }
});
