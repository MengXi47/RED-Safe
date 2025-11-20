<script>
  export let eyebrow = '';
  export let title = '';
  export let lead = '';
</script>

<div class="section-header fade-in">
  {#if eyebrow}<p class="eyebrow">{eyebrow}</p>{/if}
  {#if title}<h2 class="h2">{title}</h2>{/if}
  {#if lead}<p class="lead">{lead}</p>{/if}
</div>
