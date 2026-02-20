#include <Arduino.h>

#include "web_ui.h"

namespace
{
    const char PAGE_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>ESP32 Wall-E</title>
	<style>
		body { font-family: Arial, sans-serif; background: #111827; color: #f9fafb; margin: 0; padding: 16px; }
		h1 { margin: 0 0 16px; font-size: 1.4rem; }
		.layout { display: grid; gap: 14px; max-width: 760px; margin: 0 auto; }
		.card { background: #1f2937; border-radius: 12px; padding: 12px; }
		.card h2 { margin: 0 0 10px; font-size: 1rem; }
        .motion-wrap { display: grid; grid-template-columns: 1fr 170px; gap: 10px; align-items: start; }
		.pad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; }
		.row { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; }
        .speed-box { background: #111827; border-radius: 10px; padding: 10px; }
        .speed-box label { display: block; margin-bottom: 8px; font-size: 0.9rem; }
        .speed-box input[type="range"] { width: 100%; }
        .speed-value { margin-top: 6px; font-size: 0.9rem; opacity: 0.9; }
		button {
			border: 0; border-radius: 10px; padding: 12px; font-size: 0.95rem;
			background: #374151; color: #f9fafb; cursor: pointer;
		}
		button:active { transform: scale(0.98); }
		.empty { visibility: hidden; }
		.status { font-size: 0.9rem; opacity: 0.85; }
	</style>
</head>
<body>
	<div class="layout">
        <h1>ESP32 Wall-E</h1>

		<div class="card">
			<h2>Motion</h2>
            <div class="motion-wrap">
                <div class="pad">
                    <div class="empty"></div>
                    <button onpointerdown="startMotion('forward')">▲</button>
                    <div class="empty"></div>
                    <button onpointerdown="startMotion('left')">◀</button>
                    <button onpointerdown="stopMotion()">■</button>
                    <button onpointerdown="startMotion('right')">▶</button>
                    <div class="empty"></div>
                    <button onpointerdown="startMotion('backward')">▼</button>
                    <div class="empty"></div>
                </div>
                <div class="speed-box">
                    <label for="speedRange">Speed</label>
                    <input id="speedRange" type="range" min="60" max="255" value="185" oninput="updateSpeed(this.value)" />
                    <div class="speed-value" id="speedValue">185</div>
                </div>
			</div>
		</div>

		<div class="card">
			<h2>Head</h2>
			<div class="row">
				<button onclick="send('head','left')">Left</button>
				<button onclick="send('head','center')">Center</button>
				<button onclick="send('head','right')">Right</button>
			</div>
		</div>

		<div class="card">
			<h2>Left Arm</h2>
			<div class="row">
				<button onclick="send('left_arm','up')">Up</button>
				<button onclick="send('left_arm','center')">Center</button>
				<button onclick="send('left_arm','down')">Down</button>
			</div>
		</div>

		<div class="card">
			<h2>Right Arm</h2>
			<div class="row">
				<button onclick="send('right_arm','up')">Up</button>
				<button onclick="send('right_arm','center')">Center</button>
				<button onclick="send('right_arm','down')">Down</button>
			</div>
		</div>

		<div class="status" id="status">Last command: none</div>
	</div>

	<script>
        let activeMotion = null;
        let motionSpeed = 185;

		async function send(target, action) {
			const res = await fetch(`/cmd?target=${encodeURIComponent(target)}&action=${encodeURIComponent(action)}`);
			const text = await res.text();
			document.getElementById('status').textContent = `Last command: ${text}`;
		}

        async function sendMotion(action) {
            const url = `/cmd?target=motion&action=${encodeURIComponent(action)}&speed=${encodeURIComponent(motionSpeed)}`;
            const res = await fetch(url);
            const text = await res.text();
            document.getElementById('status').textContent = `Last command: ${text}`;
        }

        function updateSpeed(value) {
            motionSpeed = Number(value);
            document.getElementById('speedValue').textContent = String(motionSpeed);
        }

        async function startMotion(action) {
            if (activeMotion === action) return;
            activeMotion = action;
            await sendMotion(action);
        }

        async function stopMotion() {
            if (activeMotion === null) return;
            activeMotion = null;
            await sendMotion('stop');
        }

        document.addEventListener('pointerup', stopMotion);
        document.addEventListener('pointercancel', stopMotion);
        window.addEventListener('blur', stopMotion);
	</script>
</body>
</html>
)HTML";
}

const char *getPageHtml()
{
    return PAGE_HTML;
}