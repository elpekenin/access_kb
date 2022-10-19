import { defineConfig } from "astro/config";
import svelte from "@astrojs/svelte";

// https://astro.build/config
export default defineConfig({
    server: {
        host: true,
        port: 8000
    },
    integrations: [svelte()]
});
