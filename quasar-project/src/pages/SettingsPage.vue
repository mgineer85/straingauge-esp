<template>
  <q-page padding>
    <q-card class="q-my-md">
      <q-card-section>
        <div class="text-h6">Calibrate to Known Weight</div>
      </q-card-section>
      <q-card-section>
        Calibrate the force sensor to a known weight.
        1. Tare the sensor
        2. Apply a defined force to the sensor
        3. Recalc the new calibration value
        <q-form action="/api/cmd" method="post" class="q-gutter-md">
          <input hidden name="action" value="calibrateToKnownValue"/>
          <q-input type="text" name="knownValue" v-model="knownValue" label="Known Weight" />
          <q-btn label="Calc new calibration value" type="submit" color="primary" />
        </q-form>
      </q-card-section>
      <q-separator />
    </q-card>
    <q-card class="q-my-md">
      <q-card-section>
        <div class="text-h6">Set Calibration Value</div>
      </q-card-section>
      <q-card-section>
        <q-form action="/api/cmd" method="post" class="q-gutter-md">
          <input hidden name="action" value="setCalibrationFactor" />
          <q-input type="text" name="calibrationFactor" v-model="calibrationFactor" label="calibrationFactor" />
          <q-btn label="set calibration factor" type="submit" color="primary" />
        </q-form>
      </q-card-section>
      <q-separator />
    </q-card>
    <q-card class="q-my-md">
      <q-card-section>
        <div class="text-h6">Load/Save Settings</div>
      </q-card-section>
      <q-card-actions vertical>
        <CmdLink action="savePreferences" caption="Save Preferences" />
        <CmdLink action="loadPreferences" caption="Load Preferences" />
      </q-card-actions>
    </q-card>

  </q-page>
</template>

<script>
import { defineComponent, ref } from 'vue'
import CmdLink from '../components/CmdLink.vue';
export default defineComponent({
  name: 'SettingsPage',
  components: { CmdLink },
  setup () {
    const knownValue = ref(null)
    return {
      knownValue,

    }
  }
})

</script>
