<script>
  /** @type {{ label:string; value:string; detail?:string }[]} */
  export let stats = [];
</script>

<div class="grid auto-fit-sm" role="list">
  {#each stats as s}
    <article class="card" role="listitem" aria-label={s.label}>
      <p class="muted" style="font-size:.7rem; letter-spacing:.18em; text-transform:uppercase; margin:0 0 .4rem;">{s.label}</p>
      <p style="margin:.15rem 0; font-size:1.5rem; font-weight:600; letter-spacing:.02em;">{s.value}</p>
      {#if s.detail}<p class="muted" style="margin:0; font-size:.8rem;">{s.detail}</p>{/if}
    </article>
  {/each}
</div>
