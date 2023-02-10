var targetUrl = `ws://${window.location.hostname}/ws`;
var camIp = "192.168.4.10";
var websocket;
var liveSocket;
var FPS = 0;
var lastFrameTime = 0;

/*
 * Utility Functions
 */
function cleanString(str) {
  return str.normalize("NFD").replace(/[^a-zA-Z0-9\s]/g, "");
}

window.addEventListener("deviceorientation", function (event) {
  var alpha = event.alpha;
  var beta = event.beta;
  var gamma = event.gamma;
  var orientation = window.orientation;
  var message = `{"alpha":${alpha},"beta":${beta},"gamma":${gamma},"orientation":${orientation}}`;
  console.log(message);
});

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
  if (websocket.readyState === WebSocket.OPEN) websocket.send(message);
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
 * Joystick Basic
 * O-Pad/ D-Pad Controller and Javascript Code
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

document.querySelectorAll(".control").forEach((item) => {
  item.addEventListener("touchstart", touchStartHandler);
});

document.querySelectorAll(".control").forEach((item) => {
  item.addEventListener("touchend", touchEndHandler);
});

/*
 * Joystick Advanced
 */

var canvas_joystick, ctx_joystick;
var ctx_button;

// setup the controls for the page
window.addEventListener("load", () => {
  canvas_joystick = document.getElementById("canvas_joystick");
  ctx_joystick = canvas_joystick.getContext("2d");

  resize();

  canvas_joystick.addEventListener("mousedown", startDrawing);
  canvas_joystick.addEventListener("mouseup", stopDrawing);
  canvas_joystick.addEventListener("mousemove", Draw);
  canvas_joystick.addEventListener("touchstart", startDrawing);
  canvas_joystick.addEventListener("touchend", stopDrawing);
  canvas_joystick.addEventListener("touchcancel", stopDrawing);
  canvas_joystick.addEventListener("touchmove", Draw);
  window.addEventListener("resize", resize);
});

var width, height, radius, button_size;
let origin_joystick = { x: 0, y: 0 };
let origin_button = { x: 0, y: 0 };
const width_to_radius_ratio = 0.04;
const width_to_size_ratio = 0.15;
const radius_factor = 5;

function resize() {
  console.log("Resizing");
  if (window.innerWidth > window.innerHeight) {
    width = window.innerHeight; // half the window for two canvases
  } else {
    width = window.innerWidth;
  }

  width = width * 0.8; // 80% of the window
  radius = width_to_radius_ratio * width;
  button_size = width_to_size_ratio * width;
  height = radius * radius_factor * 2 + 100; // use the diameter

  // configure and draw the joystick canvas
  ctx_joystick.canvas.width = width;
  ctx_joystick.canvas.height = height;
  origin_joystick.x = width / 2;
  origin_joystick.y = height / 2;
  joystick(origin_joystick.x, origin_joystick.y);
}

// Draw the background/outer circle of the joystick
function joystick_background() {
  // clear the canvas
  ctx_joystick.clearRect(0, 0, canvas_joystick.width, canvas_joystick.height);
  // draw the background circle
  ctx_joystick.beginPath();
  ctx_joystick.arc(
    origin_joystick.x,
    origin_joystick.y,
    radius * radius_factor,
    0,
    Math.PI * 2,
    true
  );
  ctx_joystick.fillStyle = "#ECE5E5";
  ctx_joystick.fill();
}

// Draw the main circle of the joystick
function joystick(x, y) {
  const SIZE = 2;
  // draw the background
  joystick_background();
  // draw the joystick circle
  ctx_joystick.beginPath();
  ctx_joystick.arc(x, y, radius * SIZE, 0, Math.PI * 2, true);
  ctx_joystick.fillStyle = "#6c757d";
  ctx_joystick.fill();
  ctx_joystick.strokeStyle = "lightgray";
  ctx_joystick.lineWidth = 2;
  ctx_joystick.stroke();
}

let coord = { x: 0, y: 0 };
let paint = false;
var movimento = 0;

// Get the position of the mouse/touch press (joystick canvas)
function getPosition_joystick(event) {
  var mouse_x =
    event.clientX || event.touches[0].clientX || event.touches[1].clientX;
  var mouse_y =
    event.clientY || event.touches[0].clientY || event.touches[1].clientY;
  coord.x = mouse_x - canvas_joystick.offsetLeft;
  coord.y = mouse_y - canvas_joystick.offsetTop;
}

// Check if the mouse/touch was pressed inside the background/outer circle of the joystick
function in_circle() {
  var current_radius = Math.sqrt(
    Math.pow(coord.x - origin_joystick.x, 2) +
      Math.pow(coord.y - origin_joystick.y, 2)
  );
  if (radius * radius_factor >= current_radius) {
    // consider the outer circle
    console.log("INSIDE circle");
    return true;
  } else {
    console.log("OUTSIDE circle");
    return false;
  }
}

// Handler: on press for the joystick canvas
function startDrawing(event) {
  document.body.style.overflow = "hidden"; // Fix scrolling on mobile
  window.scrollTo(0, 0); // Fix scrolling on mobile
  paint = true;
  getPosition_joystick(event);
  if (in_circle()) {
    // draw the new graphics
    joystick(coord.x, coord.y);
    Draw(event);
  }
}

// Handler: on release for the joystick canvas
function stopDrawing() {
  document.body.style.overflow = "auto"; // Fix scrolling on mobile
  paint = false; // reset

  // update to the default graphics
  joystick(origin_joystick.x, origin_joystick.y);
  // document.getElementById("speed").innerText = 0;
  // document.getElementById("angle").innerText = 0;
  // update the WebSocket client
  if (movimento == 1) {
    send_joystick(0, 0);
    movimento = 0;
  }
}

// Handler: on move for the joystick canvas
// Return value a and b
function send_joystick(speed, angle) {
  sendMessage("motor#" + angle + "#" + speed);
}

function mapFunction(x, in_min, in_max, out_min, out_max) {
  return ((x - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}

// Semi-handler: update the drawing of the joystick canvas
function Draw(event) {
  if (paint) {
    // update the position
    getPosition_joystick(event);
    var angle_in_degrees, x, y, speed;
    // calculate the angle
    var angle = Math.atan2(
      coord.y - origin_joystick.y,
      coord.x - origin_joystick.x
    );

    if (in_circle()) {
      x = coord.x - radius / 2; // correction to center on the tip of the mouse, by why? (Thought for another time.)
      y = coord.y - radius / 2; // correction to center on the tip of the mouse, by why? (Thought for another time.)
    } else {
      x = radius * radius_factor * Math.cos(angle) + origin_joystick.x; // consider the outer circle
      y = radius * radius_factor * Math.sin(angle) + origin_joystick.y; // consider the outer circle
    }

    // calculate the speed (radial coordinate) in percentage [0;100]
    var speed = Math.round(
      (100 *
        Math.sqrt(
          Math.pow(x - origin_joystick.x, 2) +
            Math.pow(y - origin_joystick.y, 2)
        )) /
        (radius * radius_factor)
    ); // consider the outer circle
    if (speed > 100) {
      speed = 100; // limit
    }

    // convert the angle to degrees [0;360]
    // angle += Math.PI / 2;
    if (Math.sign(angle) == -1) {
      angle_in_degrees = Math.round((-angle * 180) / Math.PI);
    } else {
      angle_in_degrees = Math.round(360 - (angle * 180) / Math.PI);
    }

    // update the elements
    joystick(x, y);
    send_joystick(speed, angle_in_degrees);
    movimento = 1;
  }
}

/*
 * Function send command to ESP32
 */

document.querySelectorAll(".btn-emotion").forEach((btn) => {
  btn.addEventListener("click", (e) => {
    let emotion = e.target.getAttribute("data-emotion");
    console.log("Emotion :: " + emotion);
    sendMessage(emotion);
  });
});
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

/*
 * Joystick Type
 */

function changeTypeJoy(type) {
  if (type === "basic") {
    document
      .getElementById("joy-basic")
      .setAttribute("style", "display: block !important;");
    document
      .getElementById("joy-advanced")
      .setAttribute("style", "display: none !important;");
    localStorage.setItem("joyType", "basic");
  } else if (type === "advanced") {
    document
      .getElementById("joy-basic")
      .setAttribute("style", "display: none !important;");
    document
      .getElementById("joy-advanced")
      .setAttribute("style", "display: flex !important;");
    localStorage.setItem("joyType", "advanced");
  } else {
    alert("Invalid type!");
  }
}

document.getElementById("joy-basic-btn").addEventListener("click", (e) => {
  changeTypeJoy("basic");
});

document.getElementById("joy-advanced-btn").addEventListener("click", (e) => {
  changeTypeJoy("advanced");
});

changeTypeJoy(localStorage.getItem("joyType") || "basic");

/*
 * Live Stream Websocket
 */
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
