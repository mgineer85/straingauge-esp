<template>
  <q-page padding>

    <q-card class="">
      <q-card-section>
        <div class="text-h6">Force</div>
      </q-card-section>
      <q-card-section>
        <div><span class="text-h3" id="force">___</span><span>N</span></div>

        <div style="margin-top:10px">
          <div>Chart</div>
          <img src="https://cdn.quasar.dev/img/mountains.jpg">
        </div>
        <div>Reading: <span id="reading">xxx</span> -</div>
      </q-card-section>
      <q-separator />

      <q-card-actions>
        <CmdLink action="tare" caption="Tare"/>
      </q-card-actions>
    </q-card>





  </q-page>
</template>

<script>
import { defineComponent } from 'vue'
import CmdLink from '../components/CmdLink.vue';

export default defineComponent({
    name: "IndexPage",
    components: { CmdLink }
})


var getJSON = function (url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.responseType = 'json';
  xhr.onload = function () {
    var status = xhr.status;
    if (status === 200) {
      callback(null, xhr.response);
    } else {
      callback(status, xhr.response);
    }
  };
  xhr.send();
};

if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function (e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function (e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener(null, function (e) {
    console.log("any event received:", e.data);
  }, false);

  source.addEventListener('reading', function (e) {
    const force = e.data;
    const elementForce = document.getElementById("reading");
    elementForce.textContent = force;
  }, false);

  source.addEventListener('force', function (e) {
    const force = e.data;
    const elementForce = document.getElementById("force");
    elementForce.textContent = force;
  }, false);

  source.addEventListener('battery', function (e) {
    const force = e.data;
    const elementForce = document.getElementById("battery");
    elementForce.textContent = force;
  }, false);

  source.addEventListener('ping', function (e) {
    const force = e.data;
    const elementForce = document.getElementById("ping");
    elementForce.textContent = force;
  }, false);

  source.onmessage = (event) => {
    const newElement = document.createElement("li");
    const eventList = document.getElementById("list");

    newElement.textContent = `message: ${event.data}`;
    eventList.appendChild(newElement);
  }
}

function getWifiInfo () {
  let url = '/api/wifi-info';

  fetch(url)
    .then(res => res.json())
    .then(out =>
      console.log('Checkout this JSON! ', out))
    .catch(err => console.log('Checkout this error! ', err));
}

function send (e, form) {
  fetch("/api/cmd", { method: 'post', body: new FormData(form) });

  console.log('We send post asynchronously (AJAX)');
  e.preventDefault();
}
</script>
