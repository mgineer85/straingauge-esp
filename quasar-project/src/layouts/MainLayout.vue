<template>
  <q-layout view="hHh lpR fFf">

    <q-header elevated class="bg-primary text-white" height-hint="98">
      <q-toolbar>
        <q-toolbar-title>
          qMeasure
        </q-toolbar-title>
      </q-toolbar>

      <q-tabs align="left">
        <q-route-tab to="/" label="Measure" />
        <q-route-tab to="/Settings" label="Settings" />
        <q-route-tab to="/Playground" label="Playground" />
      </q-tabs>
    </q-header>

    <q-page-container>
      <router-view />
    </q-page-container>

    <q-footer class="bg-grey-8 text-white">
      <q-toolbar>
        <ul id="list">
          <li>first entry</li>
        </ul>
        <q-separator vertical />

        <q-chip icon="link"><span id="ping">__</span></q-chip>
        <q-chip icon="battery_std"><span id="battery">__</span>%</q-chip>

      </q-toolbar>
    </q-footer>

  </q-layout>
</template>
<script>
import { defineComponent, ref } from 'vue'


export default defineComponent({
  name: 'MainLayout',

  components: {

  },

  setup () {

  }
})
</script>
