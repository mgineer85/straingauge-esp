<template>
  <q-page padding>
    <q-card class="">
      <q-card-section class="">
        <div class="text-h6">Playground</div>
      </q-card-section>
      <q-card-section>
        <div>Ping: <span id="ping">xxx</span></div>
        <div>Battery: <span id="battery">xxx</span></div>
      </q-card-section>
      <q-separator />

      <q-card-actions vertical>
        <q-btn flat>getWifiInfo</q-btn>

        <!--<a href="javascript:getWifiInfo();">getWifiInfo</a><br>
            <a href="javascript:cmd('tare');">Tare</a><br>
            <a href="javascript:cmd('savePreferences');">savePreferences</a><br>
            <a href="javascript:cmd('loadPreferences');">loadPreferences</a><br>-->


      </q-card-actions>
    </q-card>


      <q-card class="bg-white full-width">
        <Bar :chart-options="chartOptions" :chart-data="chartData" :chart-id="chartId" :dataset-id-key="datasetIdKey" :plugins="plugins" :css-classes="cssClasses" :styles="styles" :width="width" :height="height" />
      </q-card>
  </q-page>
</template>

<script>

import { Bar } from 'vue-chartjs'
import { Chart as ChartJS, Title, Tooltip, Legend, BarElement, CategoryScale, LinearScale } from 'chart.js'

ChartJS.register(Title, Tooltip, Legend, BarElement, CategoryScale, LinearScale)

export default {
  name: 'BarChart',
  components: { Bar },
  props: {
    chartId: {
      type: String,
      default: 'bar-chart'
    },
    datasetIdKey: {
      type: String,
      default: 'label'
    },
    width: {
      type: Number,
      default: 100
    },
    height: {
      type: Number,
      default: 100
    },
    cssClasses: {
      default: '',
      type: String
    },
    styles: {
      type: Object,
      default: () => { }
    },
    plugins: {
      type: Object,
      default: () => { }
    }
  },
  data () {
    return {
      chartData: {
        labels: ['January', 'February', 'March'],
        datasets: [{ data: [40, 20, 12] }]
      },
      chartOptions: {
        responsive: true
      }
    }
  }
}

</script>
