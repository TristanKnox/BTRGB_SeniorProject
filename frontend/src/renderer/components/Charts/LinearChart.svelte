<script lang="ts">
  import "@carbon/charts/styles.min.css";
  import "carbon-components/css/carbon-components.min.css";
  import { ScatterChart } from "@carbon/charts-svelte";
  import { text } from "svelte/internal";

  // https://github.com/antimatter15/rgb-lab/blob/master/color.js
  function lab2rgb(lab) {
    var y = (lab[0] + 16) / 116,
      x = lab[1] / 500 + y,
      z = y - lab[2] / 200,
      r,
      g,
      b;

    x = 0.95047 * (x * x * x > 0.008856 ? x * x * x : (x - 16 / 116) / 7.787);
    y = 1.0 * (y * y * y > 0.008856 ? y * y * y : (y - 16 / 116) / 7.787);
    z = 1.08883 * (z * z * z > 0.008856 ? z * z * z : (z - 16 / 116) / 7.787);

    r = x * 3.2406 + y * -1.5372 + z * -0.4986;
    g = x * -0.9689 + y * 1.8758 + z * 0.0415;
    b = x * 0.0557 + y * -0.204 + z * 1.057;

    r = r > 0.0031308 ? 1.055 * Math.pow(r, 1 / 2.4) - 0.055 : 12.92 * r;
    g = g > 0.0031308 ? 1.055 * Math.pow(g, 1 / 2.4) - 0.055 : 12.92 * g;
    b = b > 0.0031308 ? 1.055 * Math.pow(b, 1 / 2.4) - 0.055 : 12.92 * b;

    return [
      Math.max(0, Math.min(1, r)) * 255,
      Math.max(0, Math.min(1, g)) * 255,
      Math.max(0, Math.min(1, b)) * 255,
    ];
  }

  export let data;
  export let matrix = "CM";

  $: if (data?.["matrix_values"]) {
    dataLL = [];
    colors = {};
    let chartData = [];
    data["matrix_values"]
      .find((ele) => ele.name === `${matrix} L*_camera`)
      ["data"].map((row, i) => {
        row.map((colData, k) => {
          chartData = [
            ...chartData,
            {
              col: String.fromCharCode(k + 65),
              row: i + 1,
              lcam: data["matrix_values"].find(
                (ele) => ele.name === `${matrix} L*_camera`
              )["data"][i][k],
              lref: data["matrix_values"].find(
                (ele) => ele.name === `${matrix} L*_ref`
              )["data"][i][k],
            },
          ];
        });
      });

    chartData.map((value) => {
      let rgb = lab2rgb([(value.lcam + value.lref) / 2, 0, 0]);
      dataLL.push({
        group: `${value.col}:${value.row}`,
        lref: value.lref,
        lcam: value.lcam,
      });
      colors[
        `${value.col}:${value.row}`
      ] = `rgb(${rgb[0]}, ${rgb[1]}, ${rgb[2]})`;
    });
  }

  let dataLL = [];
  let colors = {};
</script>

<div class="liner-chart">
  <ScatterChart
    data={dataLL}
    options={{
      title: "L*ref vs L*cam",
      axes: {
        bottom: {
          title: "L*ref",
          mapsTo: "lref",
          ticks: {
            values: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100],
          },
          domain: [0, 100],
        },
        left: {
          title: "L*cam",
          mapsTo: "lcam",
          ticks: {
            values: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100],
          },
          domain: [0, 100],
        },
      },
      legend: {
        enabled: false,
      },
      toolbar: {
        enabled: true,
        // controls: [
        //   { type: "Show as table" },
        //   { type: "Make Fullscrean" },
        //   { type: "Zoom out" },
        // ],
        numberOfIcons: 1,
      },
      height: "70vh",
      resizable: true,
      color: {
        scale: colors,
      },
    }}
  />
</div>

<!-- <div class="heatmap-number-grid">
  {#each valueGrid.reverse() as row, i}
    {#each row as col, i}
      <p class="heatmap-value">
        {col.value}
      </p>
    {/each}
  {/each}
</div> -->
<style lang="postcss" global>
  .liner-chart {
    @apply w-full flex justify-center;
  }
  .liner-chart .bx--chart-holder {
    @apply aspect-square w-auto;
  }
  .liner-chart .bx--cc--scatter circle {
    fill-opacity: 1;
    r: 7;
  }
  .liner-chart .bx--cc--scatter circle {
    stroke: black !important;
  }
</style>
