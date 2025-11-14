<script>
	import { browser } from '$app/environment';
	import { goto } from '$app/navigation';
	import { fadeObserver } from '$lib/actions/fade';
	import { onMount } from 'svelte';

	const flowSteps = [
		{
			title: '即時影像',
			description: '多鏡頭自動校正，低延遲蒐集環境資訊。',
			src: '/assets/icons/live-video.png',
			alt: '即時影像',
			accent: 'blue'
		},
		{
			title: '邊緣分析',
			description: 'Edge AI 裝置本地推理，維持資料隱私與速度。',
			src: '/assets/icons/edge-analysis.png',
			alt: '邊緣分析',
			accent: 'green'
		},
		{
			title: 'AI 判斷',
			description: '多模組風險判斷引擎，剖析跌倒、久未活動等狀態。',
			src: '/assets/icons/ai-judgment.png',
			alt: 'AI 判斷',
			accent: 'coral'
		}
	];

	const valuePillars = [
		{
			icon: '✅',
			title: '即時警示',
			desc: '毫秒級推播與電話提醒，縮短黃金救援期。'
		},
		{
			icon: '🔒',
			title: '資安保障',
			desc: '影像留在家中設備，只傳送事件結果與統計。'
		},
		{
			icon: '📈',
			title: '數據驅動',
			desc: '長期行為曲線與健康趨勢視覺化，協助預防跌倒。'
		}
	];

	const insightCards = [
		{
			label: '邊緣節點',
			value: '120+',
			desc: '已佈建的居家與照護中心'
		},
		{
			label: '誤報下降',
			value: '38%',
			desc: '多重感測器情境比對'
		},
		{
			label: '應用領域',
			value: '6',
			desc: '居家/安養/醫院/社區/旅宿/保險'
		}
	];

	onMount(() => {
		if (browser) {
			document.getElementById('hero')?.scrollIntoView({ behavior: 'auto' });
		}
	});

	const focusAlert = () => {
		if (browser) {
			document.getElementById('alert')?.scrollIntoView({ behavior: 'smooth', block: 'center' });
		}
	};

	const goAbout = () => goto('/about');
</script>

<svelte:head>
	<title>長者安全即時通報系統 | RED-Safe</title>
	<meta
		name="description"
		content="RED-Safe 長者安全即時通報系統，透過即時影像、邊緣分析與 AI 判斷守護長者安全。"
	/>
</svelte:head>

<main>
	<section class="hero" id="hero">
		<div class="hero-grid">
			<div class="hero-copy fade-element" use:fadeObserver>
				<p class="hero-eyebrow">靈感源自守護之手</p>
				<h1>長者安全即時通報系統</h1>
				<p class="hero-intro">
					以 AI 和邊緣運算打造安心防護罩，從影像偵測、狀態判讀到推播應對，讓每次突發都能被看見、被即時照護。
				</p>
				<div class="hero-actions">
					<button class="cta primary" type="button" on:click={focusAlert}>即刻守護</button>
					<button class="cta ghost" type="button" on:click={goAbout}>瞭解產品</button>
				</div>
				<div class="hero-stats">
					<div class="stat-chip">
						<span class="stat-title">24/7</span>
						<span class="stat-desc">全天候監測</span>
					</div>
					<div class="stat-chip">
						<span class="stat-title">90%</span>
						<span class="stat-desc">AI 跌倒準確率</span>
					</div>
					<div class="stat-chip">
						<span class="stat-title">3 秒內</span>
						<span class="stat-desc">事件推播</span>
					</div>
				</div>
			</div>
			<div class="hero-visual fade-element" use:fadeObserver>
				<div class="halo halo-green"></div>
				<div class="halo halo-blue"></div>
				<div class="halo halo-coral"></div>
				<img src="/assets/icons/elder-icon.png" alt="RED-Safe caring hands" draggable="false" />
				<div class="visual-card">
					<p class="visual-title">Edge AI 安全網</p>
					<p class="visual-text">居家影像僅於本地分析，保護每份隱私並即時呼叫照護者。</p>
				</div>
			</div>
		</div>
	</section>

	<section class="scroll-section flow-panel" id="flow">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Workflow</p>
			<div>
				<h2>護理任務自動串聯</h2>
				<p>從即時影像取得到 AI 判斷推播，每一步都沿用守護之手的漸層光暈，直覺辨識狀態。</p>
			</div>
		</div>
		<div class="flow-grid">
			{#each flowSteps as step, index}
				<article
					class="flow-card fade-element"
					use:fadeObserver
					data-accent={step.accent}
					style={`--flow-index:${index + 1}`}
				>
					<div class="flow-icon">
						<img src={step.src} alt={step.alt} draggable="false" />
					</div>
					<div class="flow-meta">
						<span class="flow-step">Step {String(index + 1).padStart(2, '0')}</span>
						<h3>{step.title}</h3>
						<p>{step.description}</p>
					</div>
				</article>
			{/each}
		</div>
	</section>

	<section class="scroll-section" id="alert">
		<div class="phone-sim fade-element" use:fadeObserver>
			<p>偵測跌倒！</p>
			<button type="button">您是否安全？</button>
		</div>
	</section>

	<section class="scroll-section" id="value">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Why RED-Safe</p>
			<div>
				<h2>三大守護支柱</h2>
				<p>用 AI 與實務流程協助家屬、護理師與社福單位同步掌握狀況。</p>
			</div>
		</div>
		<div class="value-grid">
			{#each valuePillars as pillar}
				<article class="value-card fade-element" use:fadeObserver>
					<span class="value-icon">{pillar.icon}</span>
					<div>
						<h3>{pillar.title}</h3>
						<p>{pillar.desc}</p>
					</div>
				</article>
			{/each}
		</div>
	</section>

	<section class="scroll-section insight-panel">
		<div class="section-heading fade-element" use:fadeObserver>
			<p class="eyebrow">Impact</p>
			<div>
				<h2>量化守護成效</h2>
				<p>平台持續收斂行為資料，提供決策者客觀指標。</p>
			</div>
		</div>
		<div class="insight-grid">
			{#each insightCards as card}
				<article class="insight-card fade-element" use:fadeObserver>
					<p class="insight-label">{card.label}</p>
					<p class="insight-value">{card.value}</p>
					<p class="insight-desc">{card.desc}</p>
				</article>
			{/each}
		</div>
	</section>

	<footer class="footer fade-element" use:fadeObserver>
		<p>RED-Safe</p>
	</footer>
</main>
