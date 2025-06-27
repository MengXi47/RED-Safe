function renderUserMenu() {
  const userMenu = document.getElementById('userMenu');
  const user = JSON.parse(localStorage.getItem('user'));
  const isAboutPage = window.location.pathname.startsWith('/about');
  const aboutBtnText = isAboutPage ? '回首頁' : '介紹';
  const aboutBtnLink = isAboutPage ? window.INDEX_URL : window.ABOUT_URL;

  if (user && user.name) {
    userMenu.innerHTML = `
      <button id="aboutBtn" class="about-btn">${aboutBtnText}</button>
      <div class="dropdown">
        <button class="dropbtn user-name">👤 ${user.name}</button>
        <div class="dropdown-content">
          <a href="${window.PROFILE_URL}">會員專區</a>
          <a href="#" id="logoutBtn">登出</a>
        </div>
      </div>
    `;
    document.getElementById('logoutBtn').onclick = function() {
      localStorage.removeItem('user');
      renderUserMenu();
    };
    document.getElementById('aboutBtn').onclick = function() {
      window.location.href = aboutBtnLink;
    };
  } else {
    userMenu.innerHTML = `
      <button id="aboutBtn" class="about-btn">${aboutBtnText}</button>
      <button id="signBtn" class="sign-btn">Sign In / Sign Up</button>
    `;
    document.getElementById('signBtn').onclick = function() {
      document.getElementById('signModal').style.display = 'flex';
    };
    document.getElementById('aboutBtn').onclick = function() {
      window.location.href = aboutBtnLink;
    };
  }
}
window.renderUserMenu = renderUserMenu;
document.addEventListener('DOMContentLoaded', renderUserMenu);
