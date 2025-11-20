<script>
  import SectionHeader from '$lib/components/SectionHeader.svelte';
  import StatGrid from '$lib/components/StatGrid.svelte';
  import ExperienceTimeline from '$lib/components/ExperienceTimeline.svelte';
  import Footer from '$lib/components/Footer.svelte';

  const heroStats = [
    { label: '跨場域部署', value: '6', detail: '居家 / 安養 / 醫院 / 社區 / 旅宿 / 保險' },
    { label: '平均安裝時間', value: '48h', detail: 'Edge Box 預組件' },
    { label: '跌倒偵測準確率', value: '>92%', detail: 'MediaPipe + YOLO 混合推理' }
  ];

  const principleCards = [
    { title: '情境化視覺語言', desc: '延伸 elder-icon 雙手弧線，打造光圈、弧形矩陣與柔和霓虹，讓 UI 在夜間也醒目。' },
    { title: '資料即任務', desc: '所有感測資料直接轉成任務軸，護理站可以拖曳卡片分配責任。' },
    { title: 'Edge to Cloud 雙軌', desc: '事件判斷留在現場，洞察與稽核同步至雲端，符合資安規範。' }
  ];

  const systemStacks = [
    { tag: '01', title: 'Sense Layer', body: '多尺寸鏡頭、毫米波雷達與穿戴組合，自動校正高度與光線。', bullets: ['RGBD + 深度融合演算法', '動態遮蔽補償', '場域快速校準工具'] },
    { tag: '02', title: 'Think Layer', body: 'Edge Box 內建容器化推理，AI 模組能依場域滾動更新。', bullets: ['MediaPipe Skeleton', 'YOLOv8 跌倒 / 久未移動', '自適應臥床模型'] },
    { tag: '03', title: 'Act Layer', body: '從 UI 流程定義行動劇本，串接語音、App、簡訊與 API。', bullets: ['119 / 1922 API', 'LINE / SMS 推播', 'SSE 任務白板'] },
    { tag: '04', title: 'Learn Layer', body: '資料湖 + 可視化儀表板，產出趨勢、熱區與稽核報告。', bullets: ['行為曲線', '跌倒熱區地圖', 'PDF / CSV 匯出'] }
  ];

  const deploymentJourney = [
    { phase: '診斷', detail: '共同檢視平面與監視死角，建立 Virtual Twin。', assets: ['空間掃描', '需求工作坊'] },
    { phase: '導入', detail: '48 小時完工，包含鏡頭定位、Edge Box 與 UI 白標。', assets: ['模組化配線', '場域訓練'] },
    { phase: '啟動', detail: '三軸通知（護理站 / 家屬 / 系統）同步啟用，持續驗證模型。', assets: ['多角色帳號', '事件模擬'] },
    { phase: '優化', detail: '月度報告建議設備調整與復健課表，封存可稽核軌跡。', assets: ['AI 報告', 'API 迭代'] }
  ];

  const journeySteps = deploymentJourney.map((s, i) => ({
    step: String(i + 1).padStart(2, '0'),
    title: s.phase,
    detail: s.detail,
    chips: s.assets
  }));

  const integrationStreams = [
    { title: '智慧建築', desc: 'BACnet / KNX 資料匯入，事件觸發照明與門禁。' },
    { title: '醫療系統', desc: 'HL7 / FHIR 互通，風險寫入病歷。' },
    { title: '保險與福利', desc: '匿名指標提供保險精算與企業 EAP。' },
    { title: '社福網絡', desc: '連動 1966 與長照 2.0，提供外部關懷站。' }
  ];
</script>

<svelte:head>
	<title>RED-Safe 產品藍圖</title>
	<meta
		name="description"
		content="深入了解 RED-Safe Edge AI 長者安全平台：視覺語言、技術堆疊與導入旅程。"
	/>
</svelte:head>

<main>
  <section class="section" id="about-hero" aria-labelledby="about-title">
    <div class="container">
      <p class="eyebrow">Designing Care Futures</p>
      <h1 id="about-title" class="h1">RED‑Safe 守護藍圖</h1>
      <p class="lead measure">我們將 elder‑icon 的雙手延伸為 UI 語彙：弧形光圈、漸層霧面玻璃與流線指示，將複雜的 AI 推理化為一眼可懂的照護旅程。</p>
      <div style="margin-top:1.5rem;">
        <StatGrid stats={heroStats} />
      </div>
    </div>
  </section>

  <section class="section" id="principles">
    <div class="container">
      <SectionHeader eyebrow="UI DNA" title="重新定義照護介面三原則" />
      <div class="pillar-grid">
        {#each principleCards as card}
          <article class="pillar-card">
            <h3 class="h3">{card.title}</h3>
            <p class="muted">{card.desc}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section" id="about-tech">
    <div class="container">
      <SectionHeader eyebrow="System Stack" title="從感測到學習的閉環架構" />
      <div class="grid auto-fit-md" style="--grid-gap:1.2rem;">
        {#each systemStacks as stack}
          <article class="card">
            <span class="tag" style="font-size:.65rem; letter-spacing:.22em; text-transform:uppercase; background: var(--color-surface-alt); padding:.35rem .6rem; border-radius: var(--radius-sm); display:inline-block;">{stack.tag}</span>
            <h3 class="h3" style="margin:.4rem 0 .5rem;">{stack.title}</h3>
            <p class="muted">{stack.body}</p>
            <ul style="margin:.6rem 0 0; padding-left:1.1rem;">
              {#each stack.bullets as item}
                <li>{item}</li>
              {/each}
            </ul>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <section class="section" id="about-journey">
    <div class="container">
      <SectionHeader eyebrow="Deployment Journey" title="導入四階段，邊緣與雲端同步" lead="每一階段 UI 顏色與任務節奏都會切換，方便跨部門協作。" />
      <ExperienceTimeline steps={journeySteps} />
    </div>
  </section>

  <section class="section" id="about-integrations">
    <div class="container">
      <SectionHeader eyebrow="Ecosystem" title="與既有系統共生" />
      <div class="grid auto-fit-md">
        {#each integrationStreams as stream}
          <article class="card">
            <h3 class="h3">{stream.title}</h3>
            <p class="muted">{stream.desc}</p>
          </article>
        {/each}
      </div>
    </div>
  </section>

  <Footer />
</main>
