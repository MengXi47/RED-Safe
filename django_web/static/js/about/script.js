document.addEventListener('DOMContentLoaded', () => {
  const goTopBtn = document.getElementById('goTopBtn');
  if (goTopBtn) {
    goTopBtn.addEventListener('click', () => {
      // 取得目前路徑（去掉最後斜線）
      const path = window.location.pathname.replace(/\/+$/, '');
      // 取得首頁路徑（去掉最後斜線）
      const indexPath = window.INDEX_URL.replace(/\/+$/, '');
      // 如果在首頁就滾頂，否則跳回首頁
      if (path === '' || path === indexPath) {
        window.scrollTo({ top: 0, behavior: 'smooth' });
      } else {
        window.location.href = window.INDEX_URL;
      }
    });
  }

  // IntersectionObserver 觸發動畫
  const fadeEls = document.querySelectorAll('.fade-element');
  const observer = new IntersectionObserver(entries => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        entry.target.classList.add('visible');
      } else {
        entry.target.classList.remove('visible');
      }
    });
  }, { threshold: 0.3 });

  fadeEls.forEach(el => observer.observe(el));
});
