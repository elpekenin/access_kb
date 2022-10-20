import { writable } from 'svelte/store';

export const tauriCounter = writable(0);

export function tauriIncrement() {
    tauriCounter.update(n => n+1);
}