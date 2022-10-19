<script lang="ts">
  import { invoke } from '@tauri-apps/api';

  export let command;
  export let args;

  try { invoke(command, args); }
  catch (e) { console.error(e); }
</script>