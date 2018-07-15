  window.addEventListener('load', () => {
    let filter_input = document.getElementById('filter-input')
    let forms_cmd = document.getElementsByClassName('form-cmd')

    if (filter_input) {
      filter_input.addEventListener('input', function () {
        for (i = 0; forms_cmd[i]; i++) {
          let attr = forms_cmd[i].getAttribute('href') || forms_cmd[i].getAttribute('action')
          if (attr.includes(this.value)) {
            forms_cmd[i].style.display = 'block'
          } else {
            forms_cmd[i].style.display = 'none'
          }
        }
      })
    }

    if (forms_cmd) {
      for (i = 0; forms_cmd[i]; i++) {
        forms_cmd[i].addEventListener('submit', function (e) {
          e.preventDefault() // stop form from submitting
          let resposes_text = document.getElementsByClassName('response-container')
          for (i = 0; resposes_text[i]; i++) {
            resposes_text[i].innerHTML = '' // reset response in all forms
          }
          let xhttp = new XMLHttpRequest() // make a AJAX-XMLHttp request
          let form = this
          xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
              if (this.responseText) {
                let res = JSON.parse(this.responseText)
                form.getElementsByClassName('response-container')[0].innerHTML =
                  '<pre class="response"><tprfx>' + res.terminal_prefix + '</tprfx>' + res.command + '<br>' + res.response.text +'</pre>'
              }
            }
          }
          xhttp.open('POST', this.action)
          xhttp.send(new FormData(this))
        })
      }
    }
  })
