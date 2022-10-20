<script lang="ts">
  import { invoke } from '@tauri-apps/api';
  import type { InvokeArgs } from '@tauri-apps/api/tauri';
  import { tauriCounter, tauriIncrement } from "@js/stores.js";

  //update counter so ids are unique
  tauriIncrement();

  //tauri function to be called
  export let command: string;
  //arguments to be passed
  export let args : InvokeArgs = {};

  //text on the button
  export let text: string = "Placeholder";
  
  //use an image instead, takes precedence over text
  export let image: string = "";
  //image size
  export let width: string = "50px";

  //read current value -> this way each button can change its parent div content
  let div_id: string;
  tauriCounter.subscribe(value => {
    div_id = `tauri-${value}`;
  });

  function handleClick() {
    try {
      invoke(command, args)
      .then((response: string) => {
        //find parent div
        let div = document.getElementById(div_id);

        // TODO: Implement support for other types of return options
        if (response) //we may not get an aswer
          //this will remove the button as it's part of the inner HTML
          div.innerHTML = response;
          div.style.fontSize = "20px";
      })
    } catch (e) {
      console.error(e);
    }
  }
</script>

<div id={div_id}>
  <button on:click={handleClick}>
  {#if image} 
    <img src={image} alt="missing" width={width} />
  {:else}
    {text}
  {/if}
  </button>
</div>

