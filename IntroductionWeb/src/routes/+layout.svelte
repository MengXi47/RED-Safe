<script>
	import '../app.css';
	import '$lib/styles/design-system.css';
	import SignHeader from '$lib/components/SignHeader.svelte';
</script>

<svelte:head>
	<link rel="icon" href="/favicon.ico" />
</svelte:head>

<div class="app-shell" style="padding-top:72px;">
	<SignHeader />
	<slot />
</div>
