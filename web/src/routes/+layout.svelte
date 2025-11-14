<script>
	import '../app.css';
	import SignHeader from '$lib/components/SignHeader.svelte';
</script>

<svelte:head>
	<link rel="icon" href="/favicon.ico" />
</svelte:head>

<div class="app-shell">
	<SignHeader />
	<slot />
</div>
