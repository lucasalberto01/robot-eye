var targetUrl = `ws://${window.location.hostname}/ws`;
var camIp = "192.168.4.10";
var websocket;
var liveSocket;
var FPS = 0;
var lastFrameTime = 0;
window.addEventListener("load", onLoad);

function onLoad() {
  initializeSocket();
  initLive();
}

function initializeSocket() {
  console.log("Opening WebSocket connection to ESP32...");
  websocket = new WebSocket(targetUrl);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log("Starting connection to server..");
}
function onClose(event) {
  console.log("Closing connection to server..");
  setTimeout(initializeSocket, 2000);
}
function onMessage(event) {
  console.log("WebSocket message received:", event);
}

function sendMessage(message) {
  websocket.send(message);
}

/*
Speed Settings Handler
*/
var speedSettings = document.querySelectorAll(
  'input[type=radio][name="speed-settings"]'
);
speedSettings.forEach((radio) =>
  radio.addEventListener("change", () => {
    var speedSettings = radio.value;
    console.log("Speed Settings :: " + speedSettings);
    sendMessage(speedSettings);
  })
);

/*
O-Pad/ D-Pad Controller and Javascript Code
*/
// Prevent scrolling on every click!
// super sweet vanilla JS delegated event handling!
document.body.addEventListener("click", function (e) {
  if (e.target && e.target.nodeName == "A") {
    e.preventDefault();
  }
});

function touchStartHandler(event) {
  var direction = event.target.dataset.direction;
  console.log("Touch Start :: " + direction);
  sendMessage(direction);
}

function touchEndHandler(event) {
  const stop_command = "stop";
  var direction = event.target.dataset.direction;
  console.log("Touch End :: " + direction);
  sendMessage(stop_command);
}

function touchEmotionStartHandler(event) {
  var emotion = event.target.dataset.emotion;
  console.log("Emotion :: " + emotion);
  sendMessage(emotion);
}

function touchEmotionEndHandler(event) {
  // TODO: Add stop command for emotion
}

document.querySelectorAll(".control").forEach((item) => {
  item.addEventListener("touchstart", touchStartHandler);
});

document.querySelectorAll(".control").forEach((item) => {
  item.addEventListener("touchend", touchEndHandler);
});

document.querySelectorAll(".btn-emotion").forEach((item) => {
  item.addEventListener("touchstart", touchEmotionStartHandler);
});

document.querySelectorAll(".btn-emotion").forEach((item) => {
  item.addEventListener("touchend", touchEmotionEndHandler);
});

function cleanString(str) {
  return str.normalize("NFD").replace(/[^a-zA-Z0-9\s]/g, "");
}

document.getElementById("text-submit").addEventListener("submit", (e) => {
  e.preventDefault();
  let text = document.getElementById("text-eye").value;
  text = cleanString(text);
  console.log("Text :: " + text);
  if (text.length > 0) sendMessage(text);
  // document.getElementById("text-eye").value = "";
});

document.getElementById("btn-flash").addEventListener("click", () => {
  fetch(`http://${camIp}/toggle`).then((response) => {
    console.log("Flashlight toggled!");
  });
});

document.getElementById("btn-led").addEventListener("click", () => {
  console.log("LED toggled!");
  sendMessage("led");
});

document.getElementById("quality-input").addEventListener("change", (e) => {
  let value = e.target.value;
  value = parseInt(value);
  let val = 0;
  console.log("Quality changed to :: " + value);
  if (value === 1) {
    document.getElementById("set-quality").innerHTML = "120p";
    val = 1;
  } else if (value === 2) {
    document.getElementById("set-quality").innerHTML = "240p";
    val = 5;
  } else if (value === 3) {
    document.getElementById("set-quality").innerHTML = "320p";
    val = 7;
  } else if (value === 4) {
    document.getElementById("set-quality").innerHTML = "480p";
    val = 8;
  } else if (value === 5) {
    document.getElementById("set-quality").innerHTML = "720p";
    val = 11;
  } else {
    return;
  }

  fetch(
    `http://${camIp}/framesize?` +
      new URLSearchParams({ var: "framesize", val })
  ).then((response) => {
    console.log("Quality changed!");
  });
});

function liveOpen() {
  console.log("Opening Live Stream...");
}

function liveClose() {
  console.log("Closing Live Stream...");
  setTimeout(initLive, 2000);
}

function liveMessage(msg) {
  var bytes = new Uint8Array(msg.data);
  var binary = "";
  var len = bytes.byteLength;
  for (var i = 0; i < len; i++) {
    binary += String.fromCharCode(bytes[i]);
  }
  var img = document.getElementById("live");
  img.src = "data:image/jpg;base64," + window.btoa(binary);
  FPS = 1000 / (Date.now() - lastFrameTime);
  lastFrameTime = Date.now();
  document.getElementById("fps").innerHTML = FPS.toFixed(2);
}

function initLive() {
  console.log("Initializing Live Stream...");
  var host = `ws://${camIp}:81`;
  liveSocket = new WebSocket(host);
  liveSocket.binaryType = "arraybuffer";
  liveSocket.onopen = liveOpen;
  liveSocket.onclose = liveClose;
  liveSocket.onmessage = liveMessage;
}
