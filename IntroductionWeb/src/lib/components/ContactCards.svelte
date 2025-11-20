<script>
  /** @type {{ title:string; desc:string; action:string; link:string }[]} */
  export let contacts = [];
</script>
<div class="contact-grid" role="list">
  {#each contacts as c}
    <article class="contact-card" role="listitem">
      <h3>{c.title}</h3>
      <p>{c.desc}</p>
      <a class="cta-link" href={c.link}>{c.action}</a>
    </article>
  {/each}
</div>
