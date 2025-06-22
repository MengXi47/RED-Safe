// 滾動動畫
document.addEventListener('DOMContentLoaded', () => {
  // goTopBtn 點擊回首頁
  const goTopBtn = document.getElementById('goTopBtn');
  if (goTopBtn) {
    goTopBtn.addEventListener('click', () => {
      window.location.href = '../index.html';
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
