<script>
  import { browser } from '$app/environment';
  import { goto } from '$app/navigation';
  import { page } from '$app/stores';

  /** @typedef {{ label: string; path?: string; hash?: string }} NavLink */
  /** @type {NavLink[]} */
  const navLinks = [
    { label: '首頁', path: '/' },
    { label: '導入流程', hash: '#workflow' },
    { label: '邊緣解決方案', hash: '#edge' },
    { label: '雲端解決方案', hash: '#server' },
    { label: '操作介面', hash: '#web-ui' },
    { label: 'Q＆A', hash: '#qa' }
  ];

  const handleLogoClick = () => goto('/');

  /** @param {string} id */
  const scrollToId = (id) => {
    if (!browser) return;
    document.getElementById(id)?.scrollIntoView({ behavior: 'smooth', block: 'start' });
    history.replaceState({}, '', `#${id}`);
  };

  /** @param {NavLink} link */
  const isActive = (link) => {
    const current = $page.url;
    if (link.hash) return current.pathname === '/' && current.hash === link.hash;
    if (link.path === '/') return current.pathname === '/';
    return current.pathname.startsWith(link.path ?? '');
  };

  const scrollToTop = () => {
    if (!browser) return;
    window.scrollTo({ top: 0, behavior: 'smooth' });
    history.replaceState({}, '', '/');
  };

  /** @param {NavLink} link */
  const navigate = (link) => {
    if (link.hash) {
      if ($page.url.pathname === '/') scrollToId(link.hash.replace('#', ''));
      else goto(`/${link.hash}`);
      return;
    }
    if (link.path === '/' && $page.url.pathname === '/') {
      scrollToTop();
      return;
    }
    if (link.path) goto(link.path);
  };

</script>

<nav class="navbar glass" aria-label="Primary navigation">
  <div class="navbar-inner container">
    <button class="brand" type="button" on:click={handleLogoClick} aria-label="回到首頁">
      <img src="/assets/icons/elder-icon.png" alt="RED-Safe 標誌" draggable="false" />
      <span>RED‑Safe</span>
    </button>
    <div class="nav-links" role="list">
      {#each navLinks as link}
        <button
          type="button"
          class={`link ${isActive(link) ? 'active' : ''}`}
          aria-current={isActive(link) ? 'page' : undefined}
          on:click={() => navigate(link)}
        >{link.label}</button>
      {/each}
    </div>
  </div>
</nav>
