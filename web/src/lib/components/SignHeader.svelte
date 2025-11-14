<script>
	import { goto } from '$app/navigation';
	import { page } from '$app/stores';

	const navLinks = [
		{ label: '首頁', path: '/' },
		{ label: '產品介紹', path: '/about' }
	];

	const handleLogoClick = () => goto('/');

	/**
	 * @param {string} path
	 */
	const isActive = (path) => {
		const current = $page.url.pathname;
		if (path === '/') {
			return current === '/';
		}
		return current.startsWith(path);
	};

	/**
	 * @param {string} path
	 */
	const navigate = (path) => goto(path);
</script>

<header class="site-header">
	<button class="brand" type="button" on:click={handleLogoClick} aria-label="回到首頁">
		<img src="/assets/icons/elder-icon.png" alt="RED-Safe logo" draggable="false" />
		<div class="brand-copy">
			<strong>RED-Safe</strong>
			<span>Long Life Guard</span>
		</div>
	</button>
	<nav class="site-nav" aria-label="Primary navigation">
		{#each navLinks as link}
			<button
				type="button"
				class={`nav-link ${isActive(link.path) ? 'active' : ''}`}
				on:click={() => navigate(link.path)}
			>
				{link.label}
			</button>
		{/each}
	</nav>
</header>
