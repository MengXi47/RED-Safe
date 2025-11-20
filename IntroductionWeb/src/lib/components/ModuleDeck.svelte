<script>
  /** @type {{ tag:string; title:string; desc:string; metrics:string[] }[]} */
  export let modules = [];
</script>
<div class="module-grid" role="list">
  {#each modules as m}
    <article class="module-card" role="listitem">
      <p class="tag">{m.tag}</p>
      <h3>{m.title}</h3>
      <p>{m.desc}</p>
      <ul>
        {#each m.metrics as metric}
          <li>{metric}</li>
        {/each}
      </ul>
    </article>
  {/each}
</div>
