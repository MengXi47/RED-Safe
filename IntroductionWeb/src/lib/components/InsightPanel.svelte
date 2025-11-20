<script>
  /** @type {{ title:string; desc:string; stat:string }[]} */
  export let insights = [];
</script>
<div class="command-insights" role="list">
  {#each insights as card}
    <article class="insight-card" role="listitem">
      <h3>{card.title}</h3>
      <p>{card.desc}</p>
      <span>{card.stat}</span>
    </article>
  {/each}
</div>
