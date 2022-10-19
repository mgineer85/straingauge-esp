<template>
  <q-btn :label="caption" @click="cmd" />
</template>

<script>
export default {
  // name: 'ComponentName',
  props: {
    caption: {
      type: String,
      required: true
    },

    link: {
      type: String,
      default: '/api/cmd'
    },
    action: {
      type: String,
      required: true
    },

  },
  setup (props) {
    function cmd (evt) {
      var _action = props.action;
      const formData = new FormData();
      formData.append("action", _action);
      fetch(props.link, {
        body: formData,
        method: "POST"
      })
        .then((response) => {
          if (response.status === 200) {
            console.log(response);
          }
        }).catch((error) => {
          console.error(error);
        });
    }
    return { cmd };
  },
}
</script>
