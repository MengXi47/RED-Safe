let signControl = 0; // 0=只阻止表單送出, 1=發送API

const ERROR_MESSAGES = {
  0:   "Success",
  99:  "Unknown API endpoint",
  100: "Invalid JSON format",
  101: "Invalid serial number format",
  102: "Invalid APNS token format",
  103: "Invalid email format",
  104: "Invalid username format",
  105: "Invalid password format",
  201: "Email or password incorrect",
  301: "Device already registered",
  302: "Email already registered",
  303: "Binding already exists",
  401: "Missing serial number or version",
  402: "Missing email, username, or password",
  403: "Missing email or password",
  404: "Missing user ID or APNS token",
  405: "Missing user ID or serial number",
  500: "Internal server error"
};

function getErrorMessage(code) {
  return ERROR_MESSAGES[code] || `Unknown error (code: ${code})`;
}

document.addEventListener('DOMContentLoaded', () => {
  // --- goTopBtn 回首頁/滾頂功能 ---
  const goTopBtn = document.getElementById('goTopBtn');
  if (goTopBtn) {
    goTopBtn.addEventListener('click', () => {
      const path = window.location.pathname.replace(/\/+$/, '');
      if (path === '' || path === '/index.html') {
        window.scrollTo({ top: 0, behavior: 'smooth' });
      } else {
        window.location.href = '/';
      }
    });
  }

  // --- sign 彈窗開關 ---
  const signBtn = document.getElementById('signBtn');
  const signModal = document.getElementById('signModal');
  const closeModal = document.getElementById('closeModal');
  if (signBtn && signModal && closeModal) {
    signBtn.onclick = () => signModal.style.display = 'flex';
    closeModal.onclick = () => signModal.style.display = 'none';
    window.onclick = (e) => {
      if (e.target === signModal) signModal.style.display = 'none';
    };

  }

  // --- 登入/註冊 tab 切換 ---
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

    // --- 登入表單送出 ---
    loginForm.onsubmit = async (e) => {
      e.preventDefault();
      if (signControl === 0) {
        alert('Test mode: submit prevented, API not sent.');
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
          alert('HTTP error: ' + response.status);
          return;
        }

        const body = await response.json();

        if (typeof body.error_code !== "undefined") {
          if (body.error_code === 0) {
            alert('Login successful!\nUser name: ' + body.user_name + '\nEmail: ' + body.email);
            signModal.style.display = 'none';
          } else {
            alert(getErrorMessage(body.error_code));
          }
        } else {
          alert('Server response format error');
        }
      } catch (err) {
        alert('Login failed, please check your network or try again later.');
      }
    };

    // --- 註冊表單送出 ---
    registerForm.onsubmit = async (e) => {
      e.preventDefault();
      if (signControl === 0) {
        alert('Test mode: submit prevented, API not sent.');
        return;
      }
      const inputs = registerForm.querySelectorAll('input');
      const email = inputs[0].value;
      const user_name = inputs[1].value;
      const password = inputs[2].value;
      const confirm = inputs[3].value;
      if (password !== confirm) {
        alert('The two passwords do not match.');
        return;
      }
      try {
        const response = await fetch('https://api.redsafe-tw.com/user/signup', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ email, user_name, password })
        });

        if (!response.ok) {
          alert('HTTP error: ' + response.status);
          return;
        }

        const body = await response.json();

        if (typeof body.error_code !== "undefined") {
          if (body.error_code === 0) {
            alert('Registration successful, please login.');
            loginTab.classList.add('active');
            signupTab.classList.remove('active');
            loginForm.classList.add('active');
            registerForm.classList.remove('active');
          } else {
            alert(getErrorMessage(body.error_code));
          }
        } else {
          alert('Server response format error');
        }
      } catch (err) {
        alert('Registration failed, please check your network or try again later.');
      }
    };
  }
});
