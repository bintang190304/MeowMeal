var hoursContainer = document.querySelector('.hours');
var minutesContainer = document.querySelector('.minutes');
var secondsContainer = document.querySelector('.seconds');
var tickElements = Array.from(document.querySelectorAll('.tick'));

var last = new Date(); // Menggunakan waktu saat ini
last.setUTCHours(-1); // Penyesuaian tidak diperlukan

function updateTime() {
  var now = new Date(); // Menggunakan waktu saat ini

  var lastHours = last.getHours().toString();
  var nowHours = now.getHours().toString();
  if (lastHours !== nowHours) {
    updateContainer(hoursContainer, nowHours);
  }

  var lastMinutes = last.getMinutes().toString();
  var nowMinutes = now.getMinutes().toString();
//   if (lastMinutes !== nowMinutes) {
    // console.log(nowMinutes)
    updateContainer(minutesContainer, nowMinutes);
//   }

  var lastSeconds = last.getSeconds().toString();
  var nowSeconds = now.getSeconds().toString();
  if (lastSeconds !== nowSeconds) {
    updateContainer(secondsContainer, nowSeconds);
  }

  last = now;
}

function updateContainer(container, newTime) {
  var time = newTime.split('');

  if (time.length === 1) {
    time.unshift('0');
  }

  var first = container.firstElementChild;
  if (first.textContent !== time[0]) {
    updateNumber(first, time[0]);
  }

  var last = container.lastElementChild;
  if (last.textContent !== time[1]) {
    updateNumber(last, time[1]);
  }
}

function updateNumber(element, number) {
  element.textContent = number;
  element.classList.add('move');

  setTimeout(function () {
    element.classList.remove('move');
  }, 990);
}

setInterval(updateTime, 100);
