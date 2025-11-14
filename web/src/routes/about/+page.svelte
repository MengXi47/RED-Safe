<script>
	import { browser } from '$app/environment';
	import { fadeObserver } from '$lib/actions/fade';
	import { onMount } from 'svelte';

	const aboutImages = [
		{ src: '/about/png/owzqpshggwfzql71ngre.png', alt: '長者居家安全情境' },
		{ src: '/about/png/oofdjghl18qpikuy5c64.png', alt: 'AI 影像分析示意' },
		{ src: '/about/png/nzjtzvdsl7im5gbrxk7f.png', alt: '即時通報 App' }
	];

	const highlightCards = [
		{
			icon: '🛰️',
			title: 'Edge AI 架構',
			desc: '影像在地推理，僅上傳事件結果，符合醫療資安規範。'
		},
		{
			icon: '🤝',
			title: '照護流程整合',
			desc: '自動與護理站、家屬、119 API 串接，縮短聯繫時間。'
		},
		{
			icon: '📊',
			title: '洞察儀表板',
			desc: '活動趨勢、跌倒熱區、使用率統計一目了然。'
		}
	];

	const timelineSteps = [
		{
			step: '01',
			title: '部署感測網',
			detail: '根據場域建立多鏡頭與穿戴裝置組合，確保覆蓋率。',
			accent: 'blue'
		},
		{
			step: '02',
			title: '行為建模',
			detail: '以 MediaPipe / YOLO 特徵訓練客製化模型，校正每位長者姿態。',
			accent: 'green'
		},
		{
			step: '03',
			title: '事件通報',
			detail: '當 AI 判斷異常，立即通知家屬、照護團隊與保全單位。',
			accent: 'coral'
		},
		{
			step: '04',
			title: '迭代優化',
			detail: '根據標記資料持續更新模型，降低誤報並蒐集統計。',
			accent: 'purple'
		}
	];

	const integrationChips = ['智慧家庭', '安養中心', '長照 2.0', '企業 EAP', '保險商品', '醫療院所'];

	onMount(() => {
		if (browser) {
			document.getElementById('hero')?.scrollIntoView({ behavior: 'auto' });
		}
	});
</script>

<svelte:head>
	<title>RED-Safe 產品介紹</title>
	<meta
		name="description"
		content="RED-Safe 結合邊緣運算與 AI 深度學習，打造長者安全即時通報系統與智慧照護方案。"
	/>
</svelte:head>

<main>
	<section class="hero about-hero" id="hero">
		<div class="hero-grid">
			<div class="hero-copy fade-element" use:fadeObserver>
				<p class="hero-eyebrow">Brand Story</p>
				<h1>RED-Safe 守護藍圖</h1>
				<p class="hero-intro">
					我們以「雙手接住每一次跌倒」為核心理念，將 elder-icon 中的藍與橘手勢延伸成完整 UI 視覺語言，提供跨場域可擴充的安全方案。
				</p>
				<div class="hero-stats">
					<div class="stat-chip">
						<span class="stat-title">6</span>
						<span class="stat-desc">部署場域類型</span>
					</div>
					<div class="stat-chip">
						<span class="stat-title">90%</span>
						<span class="stat-desc">跌倒判斷準確率</span>
					</div>
					<div class="stat-chip">
						<span class="stat-title">3 秒內</span>
						<span class="stat-desc">事件推播延遲</span>
					</div>
				</div>
			</div>
			<div class="hero-visual fade-element" use:fadeObserver>
				<div class="halo halo-green"></div>
				<div class="halo halo-blue"></div>
				<div class="halo halo-coral"></div>
				<img src="/assets/icons/elder-icon.png" alt="RED-Safe caring hands" draggable="false" />
				<div class="visual-card">
					<p class="visual-title">使命宣言</p>
					<p class="visual-text">讓每個家庭都擁有 AI 托住的第三雙手。</p>
				</div>
			</div>
		</div>
	</section>

	<section class="scroll-section" id="about-highlights">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">System DNA</p>
			<div>
				<h2>產品三大特徵</h2>
				<p>兼具低延遲、高隱私與可視化洞察，協助導入單位快速掌握成效。</p>
			</div>
		</div>
		<div class="value-grid">
			{#each highlightCards as card}
				<article class="value-card fade-element" use:fadeObserver>
					<span class="value-icon">{card.icon}</span>
					<div>
						<h3>{card.title}</h3>
						<p>{card.desc}</p>
					</div>
				</article>
			{/each}
		</div>
	</section>

	<section class="scroll-section" id="about-imgs">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Scenarios</p>
			<div>
				<h2>情境剪影</h2>
				<p>實際導入畫面呈現長者日常、AI 偵測與行動端體驗。</p>
			</div>
		</div>
		<div class="about-img-row">
			{#each aboutImages as image}
				<img class="fade-element" use:fadeObserver src={image.src} alt={image.alt} draggable="false" />
			{/each}
		</div>
	</section>

	<section class="scroll-section about-timeline" id="about-tech">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Deployment Flow</p>
			<div>
				<h2>導入旅程</h2>
				<p>循序佈建、校正、串接與優化，確保系統與照護團隊同步運作。</p>
			</div>
		</div>
		<div class="timeline-grid">
			{#each timelineSteps as step}
				<article class="timeline-card fade-element" use:fadeObserver data-accent={step.accent}>
					<span class="timeline-step">Step {step.step}</span>
					<h3>{step.title}</h3>
					<p>{step.detail}</p>
				</article>
			{/each}
		</div>
	</section>

	<section class="scroll-section" id="about-integrations">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Ecosystem</p>
			<div>
				<h2>可串接服務</h2>
				<p>延伸 elder-icon 守護概念，與不同平台協作出更完整的照護環境。</p>
			</div>
		</div>
		<div class="chip-group">
			{#each integrationChips as chip}
				<span class="chip fade-element" use:fadeObserver>{chip}</span>
			{/each}
		</div>
	</section>

	<footer class="footer fade-element" use:fadeObserver>
		<p>RED-Safe</p>
	</footer>
</main>
