document.addEventListener('DOMContentLoaded', () => {
  // 取得所有帶 fade-element 類別的元素（包含流程項目、卡片、頁尾等）
  const fadeEls = document.querySelectorAll('.fade-element');

  // IntersectionObserver 用來監聽元素是否進入視窗
  const observer = new IntersectionObserver(entries => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        // 元素進入視窗時，加上 visible 類別觸發 CSS 動畫
        entry.target.classList.add('visible');
      } else {
        // 元素離開視窗時，移除 visible 類別
        entry.target.classList.remove('visible');
      }
    });
  }, { threshold: 0.3 }); // 30% 可見時觸發

  // 對所有 fade-element 元素啟用監聽
  fadeEls.forEach(el => observer.observe(el));

  // 直接用程式碼改三張流程區塊的圖片大小
  const flowImages = document.querySelectorAll('#flow .flow-item img');
  if(flowImages.length >= 3){
    flowImages[0].style.width = '200px';  // 第一張圖片大小
    flowImages[1].style.width = '200px';  // 第二張圖片大小
    flowImages[2].style.width = '200px';  // 第三張圖片大小
  }
});
