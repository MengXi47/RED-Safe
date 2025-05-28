// Sign In / Sign Up 彈窗邏輯
document.addEventListener('DOMContentLoaded', () => {
  // 彈窗開關
  const signBtn = document.getElementById('signBtn');
  const signModal = document.getElementById('signModal');
  const closeModal = document.getElementById('closeModal');
  if(signBtn && signModal && closeModal){
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
  if(loginTab && signupTab && loginForm && registerForm){
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
    // 表單送出預設阻止（僅前端展示，設定完資料庫請刪除）
    loginForm.onsubmit = e => { e.preventDefault(); alert('登入功能僅展示'); };
    registerForm.onsubmit = e => { e.preventDefault(); alert('註冊功能僅展示'); };
  }
});
