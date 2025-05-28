// sign.js
let signControl = 0; // 0=只阻止表單送出, 1=發送API

const ERROR_MESSAGES = {
  0:   "Success",
  99:  "Unknown_endpoint",
  100: "Invalid_JSON",
  101: "Invalid_serialnumber_format",
  102: "Invalid_apnstoken_format",
  103: "Invalid_email_format",
  104: "Invalid_username_format",
  105: "Invalid_password_format",
  201: "Email Email_or_Password_Error",
  301: "Edge_device_already_registered",
  302: "Email Email_already_registered",
  303: "Binding_already_exists",
  401: "Missing_serial_number_or_version",
  402: "Missing_email_or_user_name_or_password",
  403: "Missing_email_or_password",
  404: "Missing_user_id_or_apns_token",
  405: "Missing_user_id_or_serial_number",
  500: "Internal_server_error"
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
