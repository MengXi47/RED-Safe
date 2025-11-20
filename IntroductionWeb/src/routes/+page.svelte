<script>
  import Footer from '$lib/components/Footer.svelte';
  import { fadeObserver } from '$lib/actions/fade';

  const heroDetails = {
    title: 'RED-Safe 長者安全即時通報系統',
    subtitle: '基於邊緣運算與深度學習打造的跌倒偵測通報方案',
    school: '國立勤益科技大學資工系',
    advisor: '林俊榮 教授',
    members: '陳柏恩、古信汯'
  };

  const heroStats = [
    { label: '學校', value: heroDetails.school },
    { label: '指導教授', value: heroDetails.advisor },
    { label: '組員', value: heroDetails.members }
  ];

  const quickShortcuts = [
    {
      label: '導入流程',
      href: '#workflow',
      detail: '從 Capture、加密傳輸到通知推播的一條龍介紹。'
    },
    {
      label: '邊緣解決方案',
      href: '#edge',
      detail: '了解邊緣端介面、服務、資料層如何協作。'
    },
    {
      label: '雲端解決方案',
      href: '#server',
      detail: '深入 Nginx、微服務與資料層的整合方式。'
    }
  ];

  const journeyFlow = [
    { title: '前言', detail: '聚焦台灣即將邁入超高齡社會，揭示跌倒風險的迫切性。' },
    {
      title: '系統架構',
      detail: '結合邊緣裝置與雲端微服務，形成安全且可水平擴展的閉環。'
    },
    {
      title: '系統介紹與說明',
      detail: '展示從掃描攝影機到即時監看的完整情境，凸顯使用者旅程。'
    },
    { title: 'Q＆A', detail: '回應評審對隱私、佈建與準確率的疑慮，驗證方案成熟度。' }
  ];

  const motivationPoints = [
    '台灣於 2025 年邁入超高齡社會，老年人口比例突破 20%，跌倒已成為長者第二大事故死因，往往在無人注意的瞬間錯失黃金救援時間。',
    '長照人力嚴重斷層，許多家庭照顧者在外工作仍掛念長者獨自在家，擔心一旦發生意外無人得知。',
    '因此需要一套能即時偵測與即時通報的智慧照護方案，陪伴長者、減輕家屬負擔，打造更安全安心的生活品質。'
  ];

  const limitations = [
    { title: '不易部署', detail: '客戶難以自行架設，需要專業人士安裝。' },
    { title: '隱私問題', detail: '為減省算力而改用伺服器辨識，導致畫面外洩風險。' },
    { title: '易誤判', detail: '輕微倒下並非跌倒動作也會觸發，造成大量誤報。' },
    { title: '價格高昂', detail: '成熟產品價格偏高，難以大規模推廣。' }
  ];

  const coreValues = ['易部署', '無穿戴', '高準確', '低成本', '高安全'];

  const workflowSteps = [
    {
      title: 'Capture image',
      detail: '邊緣運算裝置自攝影機截取影像，並維持端點安全。'
    },
    {
      title: 'Encrypted Channel',
      detail: '偵測結果透過 TLS 加密通道上報伺服器，避免資料外洩。'
    },
    {
      title: 'Interaction',
      detail: '提供 Web 與 APP 介面，讓使用者與邊緣運算裝置交互。'
    },
    {
      title: 'Notify',
      detail: '若偵測到跌倒事件，即時推播 Web、APP 與 Email 通報。'
    }
  ];

  const edgeHighlights = [
    {
      section: '介面層',
      summary: '提供使用者與系統的互動入口，包含 Web、APP 與 API。'
    },
    {
      section: '服務層',
      summary:
        'Core Service 負責處理系統內與伺服器的交互邏輯；Web Service 提供操作介面；Log Service 統一記錄系統日誌；IP Service 提供底層網路配置 API；IPC Scan Service 掃描內網攝影機；Computer Vision Service 負責影像處理與即時預覽。'
    },
    {
      section: '資料層',
      summary: 'PostgreSQL 儲存所有需要永久保存的資料。'
    },
    {
      section: '執行環境',
      summary: '系統可執行於 Linux、Docker、Windows、MacOS 等環境。'
    }
  ];

  const serverHighlights = [
    { title: 'Nginx TLS 與負載平衡', detail: '外部流量使用 Nginx 作為終端，確保加密與分流。' },
    {
      title: 'Microservices + gRPC',
      detail: '內部採微服務架構，服務間以 gRPC 解耦，每個服務只處理一個業務。'
    },
    {
      title: 'Redis / PostgreSQL',
      detail: 'Redis 儲存暫時性數據與快取，PostgreSQL 負責永久資料。'
    },
    { title: '集中式日誌', detail: '獨立服務統一收集並保存所有微服務日誌。' },
    {
      title: '第三方工具',
      detail: 'Jenkins 自動化 CI/CD、EMQX 作為 MQTT Broker、Stalwart 擔任郵件伺服器。'
    }
  ];

  const modelFormulae = [
    {
      title: '身體傾角 θ',
      formula: 'θ = arctan((xₕ − xₛ) / (yₕ − yₛ)) × 180 / π',
      notes: 'xₕ, yₕ 為髖兩點中心座標；xₛ, yₛ 為肩膀兩點中心座標。'
    },
    {
      title: '傾角角速度 ω',
      formula: 'ω = dθ / dt = d/dt [ arctan((xₕ − xₛ) / (yₕ − yₛ)) ]'
    },
    {
      title: '高度比例 Height_ratio',
      formula: 'Height_ratio = √[(xₐ − xₕ)² + (yₐ − yₕ)²] / B',
      notes: 'xₕ, yₕ 為頭部座標；xₐ, yₐ 為腳踝兩點中心座標；B 為偵測框高度。'
    },
    {
      title: '框高比例 r',
      formula: 'r = hₜ / hₜ−Δt'
    },
    {
      title: '框高變化率 ṙ',
      formula: 'ṙ = −(hₜ / hₜ−Δt) / Δt',
      notes: 'hₜ 為當前影格框高、hₜ−Δt 為上一影格框高、Δt 為時間差。'
    }
  ];

  const webInterface = [
    {
      title: '邊緣運算裝置管理',
      detail: '管理頁面列出所有綁定的邊緣裝置，提供掃描與設定流程。',
      asset: '/assets/web.png'
    },
    {
      title: '攝影機綁定流程',
      detail: '填入各類參數即可將攝影機加入辨識清單，綁定成功後立即啟動。',
      asset: '/assets/camera.png'
    },
    {
      title: '即時顯示頁面',
      detail: '提供單視角或多分割畫面，讓使用者檢視辨識狀態與偵測畫面。',
      asset: '/assets/camera2.png'
    },
    {
      title: '跌倒事件偵測',
      detail: '偵測到跌倒事件時於即時畫面標示，並保留事件截圖。',
      asset: '/assets/camera3.png'
    }
  ];

  const notificationInfo = {
    title: '事件通報',
    detail: '系統以 Email 發送跌倒事件通知，並附上當下跌倒畫面供稽核。',
    asset: '/assets/mail.png'
  };

  let activePreview = webInterface[0];

  const setActivePreview = (panel) => {
    activePreview = panel;
  };

  const handleCardKeydown = (event, panel) => {
    if (event.key === 'Enter' || event.key === ' ') {
      event.preventDefault();
      setActivePreview(panel);
    }
  };

  const appInterface = [
    { title: '登入／註冊', detail: '首次登入需完成信箱認證，確保帳號安全。' },
    { title: '裝置綁定', detail: '可輸入邊緣裝置資訊或掃描 QR code 綁定，並即時同步狀態。' },
    { title: '網路與攝影機配置', detail: '提供網路設定與攝影機管理功能，所有操作與邊緣裝置保持連動。' },
    { title: '帳號管理', detail: '支援多種介面顏色、二階段認證與安全性設定。' }
  ];

  const qaItems = [
    {
      question: 'Q：導入系統需要多少時間？',
      answer: 'A：邊緣裝置開機自動掃描網路，填寫參數即可完成綁定，單點約 30 分鐘即可完成。'
    },
    {
      question: 'Q：隱私與資料安全如何保障？',
      answer: 'A：影像僅在邊緣端推論，伺服器僅接收推論結果並透過 TLS 加密傳輸與儲存。'
    },
    {
      question: 'Q：偵測結果如何驗證準確度？',
      answer: 'A：YOLOv11-Pose 透過姿態角度、速度與高度比例交叉驗證，並於 UI 顯示每次事件截圖。'
    }
  ];
</script>

<svelte:head>
  <title>RED-Safe | 長者安全即時通報專題</title>
  <meta
    name="description"
    content="RED-Safe 為基於邊緣運算與深度學習的長者安全即時通報系統，專題 1-2 完整呈現動機、架構、模型與介面。"
  />
</svelte:head>

<main class="page">
  <section class="hero" id="hero" use:fadeObserver>
    <div class="container hero-layout">
      <div class="hero-copy">
        <h1 class="hero-title">{heroDetails.title}</h1>
        <p class="lead hero-lead">{heroDetails.subtitle}</p>
        <p class="muted hero-subtext">
          基於邊緣運算與深度學習打造的跌倒偵測方案，將研究成果濃縮成即戰力展示。
        </p>
        <div class="hero-actions">
          <a class="btn primary" href="#workflow">了解運作流程</a>
          <a class="btn ghost" href="#web-ui">觀看操作介面</a>
        </div>
        <div class="hero-stats">
          {#each heroStats as stat}
            <article>
              <span>{stat.label}</span>
              <strong>{stat.value}</strong>
            </article>
          {/each}
        </div>
      </div>
    </div>
  </section>

  <section class="section shortcuts" aria-label="快速導覽" use:fadeObserver>
    <div class="container quick-grid">
      {#each quickShortcuts as shortcut}
        <a class="quick-card" href={shortcut.href}>
          <p class="eyebrow">{shortcut.label}</p>
          <p class="muted small">{shortcut.detail}</p>
        </a>
      {/each}
    </div>
  </section>

  <section class="section story" id="motivation" use:fadeObserver>
    <div class="container story-grid">
      <article class="panel depth">
        <div class="section-header">
          <p class="eyebrow">前言</p>
          <h2 class="h2">製作動機</h2>
        </div>
        <div class="stacked">
          {#each motivationPoints as point}
            <p>{point}</p>
          {/each}
        </div>
      </article>
      <div class="journey-panel">
        <p class="eyebrow">Pitch 旅程</p>
        <h3 class="h3">面對問題一路走到 Demo</h3>
        <ul class="journey-list">
          {#each journeyFlow as step, index}
            <li>
              <span class="journey-index">{index + 1}</span>
              <div>
                <h4 class="h4">{step.title}</h4>
                <p class="muted">{step.detail}</p>
              </div>
            </li>
          {/each}
        </ul>
      </div>
    </div>
  </section>

  <section class="section challenges" id="why-redsafe" use:fadeObserver>
    <div class="container challenge-grid">
      <div>
        <div class="section-header">
          <p class="eyebrow">目前解決方案的限制</p>
          <h2 class="h2">為什麼需要 RED-Safe</h2>
        </div>
        <div class="grid cards">
          {#each limitations as item}
            <article class="card">
              <h3 class="h4">{item.title}</h3>
              <p>{item.detail}</p>
            </article>
          {/each}
        </div>
      </div>
      <article class="panel value-panel">
        <p class="eyebrow">核心主張</p>
        <h3 class="h3">RED-Safe 五大價值</h3>
        <div class="value-tags">
          {#each coreValues as value}
            <span>{value}</span>
          {/each}
        </div>
        <p class="muted">
          以邊緣端算力與安全通道打造真正能落地的智慧照護系統。
        </p>
      </article>
    </div>
  </section>

  <section class="section workflow" id="workflow" use:fadeObserver>
    <div class="container">
      <div class="section-header">
        <p class="eyebrow">運作方式</p>
        <h2 class="h2">從感測到通報的閉環流程</h2>
      </div>
      <ol class="timeline">
        {#each workflowSteps as step, index}
          <li>
            <span class="timeline-index">{index + 1}</span>
            <div class="timeline-card">
              <p class="tag">{step.title}</p>
              <p>{step.detail}</p>
            </div>
          </li>
        {/each}
      </ol>
    </div>
  </section>

  <section class="section architecture edge" id="edge" use:fadeObserver>
    <div class="container architecture-grid">
      <article class="panel highlight">
        <p class="eyebrow">系統架構</p>
        <h2 class="h2">邊緣運算裝置軟體核心架構</h2>
        <p>
          介面、服務與資料層以模組方式組成，確保裝置可獨立運行並與雲端安全同步。
        </p>
      </article>
      <div class="grid two">
        {#each edgeHighlights as block}
          <article class="card">
            <h3 class="h4">{block.section}</h3>
            <p>{block.summary}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section architecture server" id="server" use:fadeObserver>
    <div class="container architecture-grid">
      <article class="panel highlight">
        <p class="eyebrow">伺服器架構</p>
        <h2 class="h2">雲端協調與服務整合</h2>
        <p>
          微服務搭配 gRPC、Redis 與 PostgreSQL，讓通報、儲存與監控能安全擴張。
        </p>
      </article>
      <div class="grid two">
        {#each serverHighlights as item}
          <article class="card">
            <h3 class="h4">{item.title}</h3>
            <p>{item.detail}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section model" id="model" use:fadeObserver>
    <div class="container">
      <div class="section-header">
        <p class="eyebrow">跌倒辨識與數學推論模型</p>
        <h2 class="h2">YOLOv11-Pose + Math Model</h2>
      </div>
      <div class="model-grid">
        {#each modelFormulae as item}
          <article class="formula-card">
            <div class="formula-head">
              <h3 class="h4">{item.title}</h3>
            </div>
            <pre>{item.formula}</pre>
            {#if item.notes}<p class="muted small">{item.notes}</p>{/if}
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section ui" id="web-ui" use:fadeObserver>
    <div class="container feature-grid">
      <article class="panel">
        <p class="eyebrow">邊緣運算裝置 Web 操作介面</p>
        <h2 class="h2">從綁定到即時監看</h2>
        <p>
          重新整理的卡片敘事展示出管理邊緣裝置、掃描攝影機與查看事件的完整旅程。
        </p>
        <div class="interface-preview" aria-live="polite">
          <div class="preview-frame">
            <img src={activePreview.asset} alt={`${activePreview.title} 介面示意`} loading="lazy" />
          </div>
        </div>
      </article>
      <div class="feature-cards">
        {#each webInterface as panel}
          <article
            class="feature-card clickable"
            class:active={panel === activePreview}
            role="button"
            tabindex="0"
            aria-pressed={panel === activePreview}
            on:click={() => setActivePreview(panel)}
            on:keydown={(event) => handleCardKeydown(event, panel)}
          >
            <h3 class="h4">{panel.title}</h3>
            <p>{panel.detail}</p>
          </article>
        {/each}
        <article
          class="feature-card highlight clickable"
          class:active={notificationInfo === activePreview}
          role="button"
          tabindex="0"
          aria-pressed={notificationInfo === activePreview}
          on:click={() => setActivePreview(notificationInfo)}
          on:keydown={(event) => handleCardKeydown(event, notificationInfo)}
        >
          <h3 class="h4">{notificationInfo.title}</h3>
          <p>{notificationInfo.detail}</p>
        </article>
      </div>
    </div>
  </section>

  <section class="section ui" id="app-ui" use:fadeObserver>
    <div class="container feature-grid">
      <article class="panel">
        <p class="eyebrow">手機 APP 操作介面</p>
        <h2 class="h2">連動邊緣裝置的行動體驗</h2>
        <p>
          App 以同樣的語彙呈現裝置狀態、綁定流程與帳號安全，確保隨時掌握即時警示。
        </p>
      </article>
      <div class="feature-cards">
        {#each appInterface as panel}
          <article class="feature-card">
            <h3 class="h4">{panel.title}</h3>
            <p>{panel.detail}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section qa" id="qa" use:fadeObserver>
    <div class="container">
      <div class="section-header">
        <p class="eyebrow">Q＆A</p>
        <h2 class="h2">常見問題</h2>
      </div>
      <div class="qa-grid">
        {#each qaItems as item}
          <article class="qa-card">
            <h3 class="h4">{item.question}</h3>
            <p class="muted">{item.answer}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <Footer />
</main>

<style>
  .page {
    --surface-strong: var(--layer-strong, rgba(4, 8, 18, 0.96));
    --surface-panel: var(--layer-panel, rgba(6, 12, 26, 0.86));
    --surface-soft: var(--layer-soft, rgba(255, 255, 255, 0.05));
    --border-soft: var(--border, rgba(255, 255, 255, 0.08));
    --border-strong: var(--border-strong, rgba(255, 255, 255, 0.16));
    --accent: var(--accent-salmon, #ff7c9c);
    --accent-alt: var(--accent-blue, #45b4ff);
    --radius-xl: clamp(22px, 3vw, 30px);
    --radius-xxl: clamp(28px, 4vw, 44px);
    --shadow-lg: var(--shadow-hard, 0 40px 120px rgba(0, 0, 0, 0.55));
    --shadow-md: var(--shadow-soft, 0 25px 60px rgba(2, 5, 15, 0.55));
    --shadow-sm: 0 18px 40px rgba(2, 5, 15, 0.35);
    display: flex;
    flex-direction: column;
    gap: 5rem;
    padding-bottom: 4rem;
    color: var(--text-strong, #f4f7ff);
  }

  .hero {
    margin-top: 1rem;
    padding: clamp(2.5rem, 6vw, 4rem) 0;
    border-radius: var(--radius-xxl);
    background: radial-gradient(circle at top right, rgba(255, 124, 156, 0.25), transparent 42%),
      radial-gradient(circle at 15% 10%, rgba(69, 180, 255, 0.2), transparent 45%), var(--surface-strong);
    border: 1px solid var(--border-strong);
    box-shadow: var(--shadow-lg);
    overflow: hidden;
    position: relative;
  }

  .hero-layout {
    display: grid;
    gap: 2.5rem;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    align-items: center;
  }

  .hero-copy {
    display: flex;
    flex-direction: column;
    gap: 1.25rem;
  }

  .hero-title {
    font-size: clamp(2.5rem, 4vw, 3.5rem);
    line-height: 1.1;
  }

  .hero-subtext {
    max-width: 480px;
    color: var(--text-muted, rgba(228, 232, 255, 0.75));
  }

  .hero-pill {
    display: inline-flex;
    align-items: center;
    padding: 0.35rem 1rem;
    border-radius: 999px;
    background: var(--surface-soft);
    border: 1px solid var(--border-soft);
    letter-spacing: 0.08em;
  }

  .hero-actions {
    display: flex;
    flex-wrap: wrap;
    gap: 0.75rem;
  }

  .btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    padding: 0.65rem 1.5rem;
    border-radius: var(--radius-lg, 32px);
    font-weight: 600;
    border: 1px solid var(--border-soft);
    transition: transform 150ms ease, background 150ms ease, color 150ms ease, border 150ms ease;
    background: transparent;
  }

  .btn.primary {
    background: linear-gradient(120deg, var(--accent), var(--accent-alt));
    color: #050608;
    border-color: transparent;
  }

  .btn.ghost {
    color: var(--text-muted, rgba(228, 232, 255, 0.75));
    background: var(--surface-soft);
  }

  .btn:hover {
    transform: translateY(-2px);
  }

  .hero-stats {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
    gap: 1rem;
  }

  .hero-stats article {
    padding: 1rem;
    border-radius: var(--radius-lg, 32px);
    background: var(--surface-panel);
    border: 1px solid var(--border-soft);
    box-shadow: var(--shadow-sm);
    backdrop-filter: blur(var(--blur, 22px));
  }

  .hero-stats span {
    font-size: 0.85rem;
    color: var(--text-muted, rgba(228, 232, 255, 0.75));
  }

  .shortcuts {
    padding: 0;
  }

  .quick-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 1rem;
  }

  .quick-card {
    border-radius: var(--radius-xl);
    border: 1px solid var(--border-soft);
    padding: 1.25rem 1.5rem;
    background: var(--surface-panel);
    box-shadow: var(--shadow-sm);
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    transition: transform 200ms var(--ease, ease), border 200ms var(--ease, ease);
  }

  .quick-card:hover {
    transform: translateY(-3px);
    border-color: var(--accent);
  }

  .story-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    gap: 1.5rem;
  }

  .panel {
    padding: 1.75rem;
    border-radius: var(--radius-xxl);
    background: var(--surface-panel);
    box-shadow: var(--shadow-md);
    border: 1px solid var(--border-soft);
    backdrop-filter: blur(var(--blur, 22px));
  }

  .panel.depth {
    background: linear-gradient(145deg, rgba(255, 255, 255, 0.12), rgba(255, 255, 255, 0)) var(--surface-panel);
  }

  .stacked {
    display: flex;
    flex-direction: column;
    gap: 1rem;
    line-height: 1.6;
  }

  .journey-panel {
    padding: 1.75rem;
    border-radius: var(--radius-xxl);
    background: var(--surface-strong);
    border: 1px solid var(--border-strong);
    gap: 1rem;
    display: flex;
    flex-direction: column;
  }

  .journey-list {
    list-style: none;
    margin: 0;
    padding: 0;
    display: flex;
    flex-direction: column;
    gap: 1rem;
  }

  .journey-list li {
    display: grid;
    grid-template-columns: auto 1fr;
    gap: 0.75rem;
    align-items: start;
  }

  .journey-index {
    width: 2rem;
    height: 2rem;
    border-radius: 999px;
    background: var(--accent);
    color: #050608;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-weight: 600;
  }

  .challenge-grid {
    display: grid;
    gap: 2rem;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    align-items: start;
  }

  .grid.cards {
    display: grid;
    gap: 1rem;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  }

  .card {
    padding: 1.5rem;
    border-radius: var(--radius-xl);
    background: var(--surface-panel);
    box-shadow: var(--shadow-sm);
    border: 1px solid var(--border-soft);
  }

  .value-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.75rem;
    margin: 1.5rem 0;
  }

  .value-tags span {
    border-radius: 999px;
    padding: 0.45rem 1.25rem;
    background: var(--surface-soft);
    border: 1px solid var(--border-soft);
  }

  .timeline {
    list-style: none;
    padding: 0;
    margin: 0;
    display: grid;
    gap: 1.5rem;
  }

  .timeline li {
    display: grid;
    grid-template-columns: auto 1fr;
    gap: 1rem;
    align-items: start;
  }

  .timeline-index {
    width: 2.5rem;
    height: 2.5rem;
    border-radius: 999px;
    border: 2px solid var(--accent);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-weight: 600;
  }

  .timeline-card {
    padding: 1.5rem;
    border-radius: var(--radius-xl);
    background: var(--surface-panel);
    border: 1px solid var(--border-soft);
    box-shadow: var(--shadow-sm);
  }

  .timeline-card .tag {
    font-size: 0.85rem;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    color: var(--accent);
    margin-bottom: 0.35rem;
    display: inline-block;
  }

  .architecture-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    gap: 1.5rem;
    align-items: start;
  }

  .panel.highlight {
    background: linear-gradient(160deg, rgba(255, 255, 255, 0.12), rgba(255, 255, 255, 0)) var(--surface-strong);
    border: 1px solid var(--border-strong);
  }

  .model-grid {
    display: grid;
    gap: 1rem;
    grid-template-columns: 1fr;
  }

  .formula-card {
    border-radius: var(--radius-xl);
    padding: 1.5rem;
    background: var(--surface-panel);
    border: 1px solid var(--border-soft);
    box-shadow: var(--shadow-sm);
  }

  .formula-head {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 0.75rem;
  }

  .formula-card pre {
    background: rgba(3, 6, 16, 0.72);
    padding: 0.9rem;
    border-radius: var(--radius-md, 20px);
    overflow-x: auto;
    font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', monospace;
    font-size: 0.95rem;
  }

  .feature-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    gap: 1.5rem;
    align-items: start;
  }

  .feature-cards {
    display: grid;
    gap: 1rem;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  }

  .feature-card {
    padding: 1.5rem;
    border-radius: var(--radius-xl);
    background: var(--surface-panel);
    box-shadow: var(--shadow-sm);
    border: 1px solid var(--border-soft);
    transition: border-color 0.2s ease, box-shadow 0.2s ease, transform 0.2s ease;
  }

  .feature-card.clickable {
    cursor: pointer;
  }

  .feature-card.clickable:focus-visible {
    outline: none;
    border-color: var(--accent-alt);
    box-shadow: var(--shadow-md);
  }

  .feature-card.clickable.active {
    border-color: var(--accent);
    box-shadow: var(--shadow-md);
    transform: translateY(-2px);
  }

  .interface-preview {
    margin-top: 1.25rem;
    border-radius: var(--radius-xxl);
    background: var(--surface-panel);
    border: 1px solid var(--border-soft);
    box-shadow: var(--shadow-sm);
    padding: 1rem;
  }

  .preview-frame {
    border-radius: var(--radius-xl);
    background: #01030a;
    box-shadow: var(--shadow-sm);
    border: 1px solid var(--border-soft);
    padding: 0.5rem;
  }

  .preview-frame img {
    width: 100%;
    border-radius: calc(var(--radius-xl) - 6px);
    object-fit: contain;
    background: #01030a;
  }

  .qa-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    gap: 1rem;
  }
  .qa-card {
    border-radius: var(--radius-xl);
    border: 1px solid var(--border-soft);
    padding: 1.5rem;
    background: var(--surface-panel);
    box-shadow: var(--shadow-sm);
  }

  :global(html[data-theme='light'] body) {
    background: #fdfdfd;
    color: #111322;
  }

  :global([data-theme='light']) .page {
    --surface-strong: #ffffff;
    --surface-panel: #ffffff;
    --surface-soft: rgba(15, 19, 37, 0.06);
    --border-soft: rgba(15, 19, 37, 0.12);
    --border-strong: rgba(15, 19, 37, 0.2);
    --accent: #ff5c7a;
    --accent-alt: #1973ff;
    --shadow-lg: 0 35px 85px rgba(15, 19, 37, 0.14);
    --shadow-md: 0 20px 55px rgba(15, 19, 37, 0.12);
    --shadow-sm: 0 12px 25px rgba(15, 19, 37, 0.08);
  }

  :global([data-theme='light']) .page,
  :global([data-theme='light']) .page p,
  :global([data-theme='light']) .page h1,
  :global([data-theme='light']) .page h2,
  :global([data-theme='light']) .page h3,
  :global([data-theme='light']) .page h4 {
    color: #111322;
  }

  :global([data-theme='light']) .page .muted,
  :global([data-theme='light']) .page .lead {
    color: rgba(17, 19, 34, 0.7);
  }

  :global([data-theme='light']) .hero,
  :global([data-theme='light']) .panel,
  :global([data-theme='light']) .journey-panel,
  :global([data-theme='light']) .card,
  :global([data-theme='light']) .feature-card,
  :global([data-theme='light']) .qa-card,
  :global([data-theme='light']) .timeline-card,
  :global([data-theme='light']) .quick-card,
  :global([data-theme='light']) .hero-stats article {
    background: #ffffff;
    border-color: var(--border-soft);
    box-shadow: var(--shadow-sm);
  }

  :global([data-theme='light']) .hero-pill,
  :global([data-theme='light']) .value-tags span {
    background: #ffffff;
    border-color: rgba(15, 19, 37, 0.12);
  }

  :global([data-theme='light']) .timeline-card,
  :global([data-theme='light']) .formula-card pre {
    background: #ffffff;
    color: #111322;
  }

  @media (max-width: 640px) {
    .hero {
      padding: 2rem 0;
    }

    .timeline li {
      grid-template-columns: 1fr;
    }

    .timeline-index {
      justify-self: flex-start;
    }
  }
</style>
