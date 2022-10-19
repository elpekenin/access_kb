<script lang="ts">
  import { invoke } from '@tauri-apps/api';

  export let command = "greet";
  export let args = {"name": "peke"};

  try { invoke(command, args); }
  catch (e) { console.error(e); }
</script>