<script lang="ts">
  import Sorter from "@components/Process/Sorter.svelte";
  import { currentPage, processState } from "@util/stores";
  $: if ($processState.artStacks[0].fields) {
    let done = true;
    Object.keys($processState.artStacks[0].fields).map((ele) => {
      if (
        $processState.artStacks[0].fields[ele].length === 0 &&
        !ele.includes("target")
      ) {
        done = false;
      }
    });
    if (done && !$processState.completedTabs[2]) {
      $processState.completedTabs[2] = true;
    }
  }
</script>

<main>
  <left>
    <h1>Specifiy Image Roles</h1>
    <p>Drag and drop each image into its appropriate role</p>
  </left>
  <right>
    <div>
      <Sorter />
    </div></right
  >
</main>

<style lang="postcss">
  main {
    @apply flex justify-between h-full w-full overflow-hidden;
  }
  left {
    @apply bg-gray-600 w-full h-full p-6 flex-col;
  }
  right {
    @apply bg-gray-700 w-full h-full p-6 flex justify-center overflow-auto;
  }
  h1 {
    @apply text-3xl;
  }
  p {
    @apply text-center pt-[30vh] bg-gray-500/25 m-6 h-[90%] rounded-lg;
  }
  input {
    @apply translate-y-[35vh];
  }
</style>
