const SIZE = 176;
const SCENE_MS = 5000;
const palette = {
  black: '#111610', white: '#dce0c8', red: '#b83b32', green: '#4e8c49',
  blue: '#3d668e', yellow: '#c9b83e', cyan: '#65a39d', magenta: '#8c567d'
};

const canvas = document.querySelector('#display');
const ctx = canvas.getContext('2d');
ctx.imageSmoothingEnabled = false;

const ui = {
  name: document.querySelector('#scene-name'),
  description: document.querySelector('#scene-description'),
  progress: document.querySelector('#progress-bar'),
  play: document.querySelector('#play'),
  mask: document.querySelector('#mask'),
  grid: document.querySelector('#grid-toggle')
};

function clear(color = palette.white) { ctx.fillStyle = color; ctx.fillRect(0, 0, SIZE, SIZE); }
function box(x, y, w, h, color = palette.black) { ctx.fillStyle = color; ctx.fillRect(x, y, w, h); }
function line(x1, y1, x2, y2, color = palette.black, width = 1) {
  ctx.strokeStyle = color; ctx.lineWidth = width; ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke();
}
function text(value, x, y, size, color = palette.black, align = 'left', weight = 700) {
  ctx.fillStyle = color; ctx.font = `${weight} ${size}px monospace`; ctx.textAlign = align; ctx.textBaseline = 'top'; ctx.fillText(value, x, y);
}
function ring(x, y, radius, progress, color = palette.green) {
  ctx.strokeStyle = palette.black; ctx.lineWidth = 4; ctx.beginPath(); ctx.arc(x, y, radius, 0, Math.PI * 2); ctx.stroke();
  ctx.strokeStyle = color; ctx.lineWidth = 4; ctx.beginPath(); ctx.arc(x, y, radius, -Math.PI / 2, -Math.PI / 2 + Math.PI * 2 * progress); ctx.stroke();
}
function header(left, right) { text(left, 9, 79, 7); text(right, 167, 79, 7, palette.black, 'right'); line(8, 89, 168, 89); }
function grid() {
  if (!ui.grid.checked) return;
  ctx.save(); ctx.globalAlpha = .18; ctx.strokeStyle = palette.black; ctx.lineWidth = .25;
  for (let i = 0; i <= SIZE; i += 4) { ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, SIZE); ctx.stroke(); ctx.beginPath(); ctx.moveTo(0, i); ctx.lineTo(SIZE, i); ctx.stroke(); }
  ctx.restore();
}
function commonTop(label, metric) {
  ring(38, 38, 24, .72, palette.green); text('72', 38, 29, 13, palette.black, 'center'); text(label, 38, 46, 5, palette.black, 'center');
  text(metric, 121, 18, 13, palette.black, 'center'); line(85, 37, 158, 37); text('STRATA', 121, 44, 7, palette.blue, 'center');
}

const scenes = [
  {
    name: 'World time', description: 'Local time with two glanceable world clocks.',
    draw(t) { clear(); commonTop('BAT', 'NYC'); header('MON 18', 'WORLD'); text('10:09', 88, 98, 31, palette.black, 'center'); text('NEW YORK', 88, 136, 7, palette.blue, 'center'); text('LON 15:09', 12, 154, 7); text('TYO 23:09', 164, 154, 7, palette.black, 'right'); box(8, 92, 3, 53, palette.red); }
  },
  {
    name: 'Activity', description: 'Daily movement and recovery at a glance.',
    draw(t) { clear(); commonTop('MOVE', '84%'); header('TODAY', 'ACTIVE'); ring(49, 126, 27, .84, palette.green); ring(49, 126, 19, .61, palette.yellow); text('8,421', 101, 101, 15); text('STEPS', 101, 121, 6, palette.blue); text('5.8 KM', 101, 137, 8); text('42 MIN', 101, 151, 8); }
  },
  {
    name: 'Weather', description: 'A compact forecast that works without a backlight.',
    draw(t) { clear(); commonTop('UV 3', 'BOS'); header('NOW', 'FORECAST'); ctx.fillStyle = palette.yellow; ctx.beginPath(); ctx.arc(42, 120, 16, 0, Math.PI * 2); ctx.fill(); for (let a = 0; a < 8; a++) { const q = a * Math.PI / 4; line(42 + Math.cos(q) * 22, 120 + Math.sin(q) * 22, 42 + Math.cos(q) * 29, 120 + Math.sin(q) * 29, palette.yellow, 3); } text('68°', 102, 96, 27); text('CLEAR', 102, 127, 7, palette.blue); text('H 72°  L 54°', 102, 143, 7); text('SUNSET 7:42', 102, 157, 6); }
  },
  {
    name: 'Navigation', description: 'Turn guidance composed for the segmented faceplate.',
    draw(t) { clear(); commonTop('N', '0.4 MI'); header('WALK', 'NAV'); line(31, 158, 31, 112, palette.blue, 7); line(31, 112, 68, 112, palette.blue, 7); line(68, 112, 68, 98, palette.blue, 7); ctx.fillStyle = palette.blue; ctx.beginPath(); ctx.moveTo(58, 102); ctx.lineTo(68, 92); ctx.lineTo(78, 102); ctx.fill(); text('TURN RIGHT', 91, 103, 9); text('ON BROAD ST', 91, 121, 7); text('6 MIN', 91, 145, 13, palette.green); }
  },
  {
    name: 'Notification', description: 'A quiet, readable notification preview.',
    draw(t) { clear(); commonTop('MSG', '1 NEW'); header('PHONE', 'NOW'); box(10, 98, 5, 61, palette.blue); text('ALEX', 25, 98, 9, palette.blue); text('The prototype', 25, 116, 9); text('looks great.', 25, 131, 9); text('10:09', 25, 151, 6); }
  },
  {
    name: 'Timer', description: 'High-contrast timing with unmistakable progress.',
    draw(t) { clear(); commonTop('TMR', 'RUN'); header('FOCUS', '25:00'); const remaining = Math.max(0, 24 * 60 + 37 - Math.floor((t % SCENE_MS) / 1000)); text(`${String(Math.floor(remaining / 60)).padStart(2, '0')}:${String(remaining % 60).padStart(2, '0')}`, 88, 103, 29, palette.black, 'center'); box(15, 143, 146, 8); box(15, 143, 112, 8, palette.green); text('PAUSE  •  RESET', 88, 158, 6, palette.blue, 'center'); }
  }
];

let index = 0;
let playing = true;
let sceneStarted = performance.now();
function select(next) { index = (next + scenes.length) % scenes.length; sceneStarted = performance.now(); ui.name.textContent = scenes[index].name; ui.description.textContent = scenes[index].description; }
function frame(now) {
  const elapsed = now - sceneStarted;
  if (playing && elapsed >= SCENE_MS) select(index + 1);
  scenes[index].draw(elapsed); grid();
  ui.progress.style.width = `${playing ? Math.min(100, (elapsed / SCENE_MS) * 100) : 0}%`;
  requestAnimationFrame(frame);
}
document.querySelector('#previous').addEventListener('click', () => select(index - 1));
document.querySelector('#next').addEventListener('click', () => select(index + 1));
ui.play.addEventListener('click', () => { playing = !playing; ui.play.textContent = playing ? 'Pause' : 'Play'; sceneStarted = performance.now(); });
document.querySelector('#mask-toggle').addEventListener('change', event => ui.mask.classList.toggle('hidden', !event.target.checked));
select(0); requestAnimationFrame(frame);
