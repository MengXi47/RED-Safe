<script>
  /** @type {{ step:string; title:string; detail:string; chips:string[] }[]} */
  export let steps = [];
</script>
<div class="experience-grid" role="list">
  {#each steps as s}
    <article class="experience-card" role="listitem">
      <p class="step">Step {s.step}</p>
      <h3>{s.title}</h3>
      <p>{s.detail}</p>
      <div class="chip-row">
        {#each s.chips as chip}
          <span class="chip">{chip}</span>
        {/each}
      </div>
    </article>
  {/each}
</div>
