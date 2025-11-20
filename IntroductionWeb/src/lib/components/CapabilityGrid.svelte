<script>
  /** @type {{ title:string; description:string; accent?:string; bullets:string[] }[]} */
  export let items = [];
</script>
<div class="capability-grid" role="list">
  {#each items as c}
    <article class="capability-card accent-{c.accent}" role="listitem">
      <header>
        <p>{c.title}</p>
        <h3>{c.description}</h3>
      </header>
      <ul>
        {#each c.bullets as b}
          <li>{b}</li>
        {/each}
      </ul>
    </article>
  {/each}
</div>
