#include "index_html.h"

// ============================================================================
//  HTML-панель (PROGMEM)
// ============================================================================
const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>NightClock — Панель управления</title>
<style>
:root{
  --bg:#0b1016; --bg2:#141b25; --bg3:#1d2733; --bd:#2b3646;
  --tx:#e6edf3; --dim:#8b98a8; --acc:#38bdf8; --acc2:#0284c7;
  --ok:#22c55e; --warn:#f59e0b; --err:#ef4444; --r:12px; --rs:8px;
}
*{box-sizing:border-box}
body{
  margin:0; font-family:-apple-system,"Segoe UI",Roboto,sans-serif;
  background:radial-gradient(1200px 600px at 10% -10%,#13202e 0%,var(--bg) 50%);
  color:var(--tx); min-height:100vh; font-size:14px;
}
.container{max-width:980px;margin:0 auto;padding:24px 16px 120px}
header{display:flex;flex-wrap:wrap;gap:16px;align-items:center;justify-content:space-between;margin-bottom:24px}
h1{margin:0;font-size:26px;letter-spacing:.3px}
.subtitle{margin:4px 0 0;color:var(--dim);font-size:13px}
.status-bar{display:flex;flex-wrap:wrap;gap:8px}
.status-item{
  display:inline-flex;align-items:center;gap:8px;padding:7px 13px;
  background:var(--bg2);border:1px solid var(--bd);border-radius:20px;
  font-size:12.5px;color:var(--tx);white-space:nowrap;
}
.dot{width:8px;height:8px;border-radius:50%;background:var(--dim)}
.dot.ok{background:var(--ok);box-shadow:0 0 8px var(--ok)}
.dot.warn{background:var(--warn);box-shadow:0 0 8px var(--warn)}
.dot.err{background:var(--err);box-shadow:0 0 8px var(--err)}
.tabs{
  display:flex;gap:4px;background:var(--bg2);padding:5px;border-radius:var(--r);
  margin-bottom:20px;overflow-x:auto;border:1px solid var(--bd);
}
.tabs::-webkit-scrollbar{height:0}
.tab{
  flex:1;padding:10px 16px;border:none;border-radius:var(--rs);
  background:transparent;color:var(--dim);font-size:13.5px;font-weight:500;
  cursor:pointer;white-space:nowrap;transition:all .2s;
}
.tab:hover{color:var(--tx);background:var(--bg3)}
.tab.active{background:var(--acc);color:#0b1016;font-weight:600}
.panel{display:none;animation:fade .2s ease-out}
.panel.active{display:block}
@keyframes fade{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}
.card{
  background:var(--bg2);border:1px solid var(--bd);border-radius:var(--r);
  padding:20px;margin-bottom:16px;
}
.card-title{font-size:16px;font-weight:600;margin-bottom:16px;color:var(--tx)}
.hint{font-size:12.5px;color:var(--dim);line-height:1.5;margin:6px 0}
.hint code{background:var(--bg3);padding:1px 6px;border-radius:4px;font-size:11.5px;color:var(--acc)}
.input-group{display:flex;flex-direction:column;gap:6px;margin-bottom:14px}
.input-group.row{flex-direction:row;align-items:center;gap:12px}
.input-group label{font-size:12.5px;color:var(--dim);font-weight:500;letter-spacing:.2px}
.input-group input[type=text],
.input-group input[type=number],
.input-group input[type=password],
.input-group input[type=time],
.input-group input[type=date],
.input-group select{
  background:var(--bg3);border:1px solid var(--bd);border-radius:var(--rs);
  color:var(--tx);padding:10px 12px;font-size:14px;width:100%;
  transition:border-color .2s, box-shadow .2s;
}
.input-group input:focus,.input-group select:focus{
  outline:none;border-color:var(--acc);box-shadow:0 0 0 3px rgba(56,189,248,.15);
}
input[type=range]{
  -webkit-appearance:none;width:100%;height:6px;border-radius:3px;
  background:var(--bg3);outline:none;
}
input[type=range]::-webkit-slider-thumb{
  -webkit-appearance:none;width:20px;height:20px;border-radius:50%;
  background:var(--acc);cursor:pointer;box-shadow:0 0 10px rgba(56,189,248,.5);
}
input[type=range]::-moz-range-thumb{
  width:20px;height:20px;border-radius:50%;background:var(--acc);
  cursor:pointer;border:none;box-shadow:0 0 10px rgba(56,189,248,.5);
}
.btn{
  display:inline-flex;align-items:center;gap:8px;padding:10px 18px;
  border:1px solid var(--bd);border-radius:var(--rs);background:var(--bg3);
  color:var(--tx);font-size:13.5px;font-weight:600;cursor:pointer;
  transition:all .2s;
}
.btn:hover{background:var(--bd);transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.btn-primary{background:var(--acc);border-color:var(--acc);color:#0b1016}
.btn-primary:hover{background:var(--acc2);border-color:var(--acc2)}
.btn-danger{background:transparent;border-color:var(--err);color:var(--err)}
.btn-danger:hover{background:var(--err);color:#fff}
.btn-sm{padding:6px 12px;font-size:12px}
.switch{position:relative;display:inline-block;width:48px;height:26px;flex-shrink:0}
.switch input{opacity:0;width:0;height:0}
.slider{position:absolute;inset:0;background:var(--bg3);border-radius:26px;cursor:pointer;transition:.3s}
.slider:before{
  content:"";position:absolute;width:20px;height:20px;left:3px;bottom:3px;
  background:#fff;border-radius:50%;transition:.3s;
}
.switch input:checked + .slider{background:var(--acc)}
.switch input:checked + .slider:before{transform:translateX(22px)}
.wifi-row,.ntp-row,.holiday-row,.network-row{
  display:flex;gap:8px;align-items:center;padding:10px 12px;
  background:var(--bg3);border-radius:var(--rs);margin-bottom:8px;
}
.wifi-row input{flex:1}
.network-row{justify-content:space-between}
.network-row .name{flex:1;font-weight:500}
.network-row .rssi{color:var(--dim);font-size:12px}
.day-row{background:var(--bg3);border-radius:var(--r);padding:16px;margin-bottom:12px}
.day-header{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;margin-bottom:14px}
.day-name{font-size:15px;font-weight:600}
.mode-tabs{display:flex;gap:3px;background:var(--bg);padding:3px;border-radius:6px}
.mode-tab{
  padding:5px 11px;border:none;border-radius:4px;background:transparent;
  font-size:12px;color:var(--dim);cursor:pointer;transition:all .15s;
}
.mode-tab:hover{color:var(--tx)}
.mode-tab.active{background:var(--acc);color:#0b1016;font-weight:600}
.fields-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:12px}
.field label{display:block;font-size:11.5px;color:var(--dim);margin-bottom:4px}
.field input{
  width:100%;background:var(--bg2);border:1px solid var(--bd);border-radius:6px;
  color:var(--tx);padding:7px 9px;font-size:13px;
}
.field input:focus{outline:none;border-color:var(--acc)}
.footer-bar{
  position:fixed;bottom:0;left:0;right:0;
  background:linear-gradient(to top, var(--bg) 60%, transparent);
  padding:16px 24px 20px;display:flex;justify-content:center;gap:12px;
  backdrop-filter:blur(6px);
}
.info-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(180px,1fr));gap:12px}
.info-item{background:var(--bg3);border-radius:var(--rs);padding:14px}
.info-item .k{font-size:11.5px;color:var(--dim);margin-bottom:4px}
.info-item .v{font-size:15px;font-weight:600}
.tm1637{
  display:inline-flex;gap:3px;padding:14px 22px;
  background:#001200;border-radius:var(--r);border:1px solid #0a2a0a;
  font-family:'Courier New',monospace;font-size:44px;font-weight:700;
  color:#00ff41;text-shadow:0 0 10px #00ff41,0 0 22px #00ff00;
  letter-spacing:2px;box-shadow:inset 0 0 24px rgba(0,255,65,.15);
}
.tm1637 .c{opacity:.35;animation:blink 1s steps(1) infinite}
@keyframes blink{50%{opacity:1}}
.toast{
  position:fixed;bottom:100px;right:24px;padding:14px 20px;
  background:var(--bg2);border:1px solid var(--bd);border-left:4px solid var(--acc);
  border-radius:var(--rs);font-size:13.5px;box-shadow:0 8px 30px rgba(0,0,0,.5);
  opacity:0;transform:translateY(20px);transition:all .3s;z-index:1000;
}
.toast.show{opacity:1;transform:none}
.toast.success{border-left-color:var(--ok)}
.toast.error{border-left-color:var(--err)}
.tab-active-panel{display:block}
@media(max-width:600px){
  .tab{padding:8px 10px;font-size:12px}
  .footer-bar{flex-direction:column}
  .tm1637{font-size:32px;padding:10px 16px}
}
</style>
</head>
<body>
<div class="container">

<header>
  <div>
    <h1>🌙 Night Clock — AdmiralNET</h1>
    <p class="subtitle">ESP8266 · TM1637 · 4-разрядные часы</p>
  </div>
  <div class="status-bar" id="statusBar"></div>
</header>

<nav class="tabs">
  <button class="tab active" data-tab="status">📊 Статус</button>
  <button class="tab" data-tab="wifi">📶 Wi-Fi</button>
  <button class="tab" data-tab="ntp">🕐 NTP</button>
  <button class="tab" data-tab="display">💡 Дисплей</button>
  <button class="tab" data-tab="schedule">📅 Расписание</button>
  <button class="tab" data-tab="holidays">🎉 Праздники</button>
</nav>

<main>
  <!-- ============ СТАТУС ============ -->
  <section class="panel active" id="panel-status">
    <div class="card">
      <div class="card-title">Превью дисплея</div>
      <div style="display:flex;justify-content:center;padding:10px 0;">
        <div class="tm1637" id="tm1637">--<span class="c">:</span>--</div>
      </div>
      <p class="hint" style="text-align:center;">Двоеточие мигает. Ночью экран приглушается по расписанию.</p>
    </div>
    <div class="card">
      <div class="card-title">Текущее состояние</div>
      <div class="info-grid" id="infoGrid"></div>
    </div>
    <div class="card">
      <div class="card-title">Быстрые действия</div>
      <div style="display:flex;gap:10px;flex-wrap:wrap;">
        <button class="btn" onclick="reboot()">🔄 Перезагрузить часы</button>
      </div>
      <p class="hint" style="margin-top:10px;">После перезагрузки часы заново пройдут анимацию «PLAY», подключатся к Wi-Fi и синхронизируют время.</p>
    </div>
  </section>

  <!-- ============ WI-FI ============ -->
  <section class="panel" id="panel-wifi">
    <div class="card">
      <div class="card-title">Сохранённые сети</div>
      <p class="hint">Часы перебирают список сверху вниз и подключаются к первой доступной сети. Если ни одна сеть не найдена в течение <b>10 минут</b> после включения — часы создают точку доступа <code>NightClock-Setup</code> (без пароля), где вы можете открыть эту же панель по адресу <code>192.168.4.1</code>.</p>
      <div id="wifiList" style="margin-top:12px"></div>
      <button class="btn btn-primary" onclick="addWifi()">+ Добавить сеть вручную</button>
    </div>
    <div class="card">
      <div class="card-title">Найти доступные сети</div>
      <p class="hint">Сканирование занимает до 5 секунд. Нажмите «Добавить» рядом с нужной сетью — SSID автоматически попадёт в поле выше.</p>
      <button class="btn" onclick="scanWifi()">🔍 Сканировать</button>
      <div id="scanResult" style="margin-top:12px"></div>
    </div>
  </section>

  <!-- ============ NTP ============ -->
  <section class="panel" id="panel-ntp">
    <div class="card">
      <div class="card-title">NTP-серверы (до 3)</div>
      <p class="hint">Используется первый доступный. Рекомендуется указывать один локальный и два публичных.</p>
      <div id="ntpList" style="margin-top:12px"></div>
      <button class="btn" onclick="addNtp()">+ Добавить сервер</button>
    </div>
    <div class="card">
      <div class="card-title">Часовой пояс</div>
      <div class="input-group">
        <label>POSIX TZ</label>
        <input type="text" id="posixTz">
        <div class="hint">Строка POSIX-таймзоны. Для Екатеринбурга (UTC+5, без перехода на летнее время): <code>&lt;+05&gt;-5</code></div>
      </div>
      <div class="input-group">
        <label>Смещение UTC (часы)</label>
        <input type="number" id="offsetHours" min="-12" max="14" step="0.5">
        <div class="hint">Используется при расчёте восхода/заката. Для Екатеринбурга: <code>5</code></div>
      </div>
    </div>
  </section>

  <!-- ============ ДИСПЛЕЙ ============ -->
  <section class="panel" id="panel-display">
    <div class="card">
      <div class="card-title">Максимальная яркость</div>
      <div class="input-group">
        <label>Значение: <span id="maxBrightVal" style="color:var(--acc);font-weight:700">7</span> из 7</label>
        <input type="range" id="maxBrightness" min="0" max="7" step="1">
        <div class="hint">Верхняя граница. Все остальные яркости (в расписании) ограничиваются этим значением. Меньше — экономнее и мягче для глаз.</div>
      </div>
    </div>

    <div class="card">
      <div class="card-title">Адаптивная яркость (фоторезистор)</div>
      <div class="input-group row" style="margin-bottom:10px">
        <label class="switch"><input type="checkbox" id="alEnabled"><span class="slider"></span></label>
        <span>Включить адаптивную яркость</span>
      </div>
      <p class="hint">Если включено, итоговая яркость умножается на коэффициент, зависящий от освещённости комнаты. Днём — ярче, ночью — тусклее. Требуется подключить фоторезистор между <code>A0</code> и <code>GND</code>, а верхним плечом — резистор на <code>3V3</code>.</p>
      <div class="fields-grid">
        <div class="field"><label>Пин</label><input type="number" id="alPin" min="0" max="17"></div>
        <div class="field"><label>Мин. raw (темнота)</label><input type="number" id="alMinRaw" min="0" max="1023"></div>
        <div class="field"><label>Макс. raw (свет)</label><input type="number" id="alMaxRaw" min="0" max="1023"></div>
        <div class="field"><label>Мин. яркость</label><input type="number" id="alMinBrightness" min="0" max="7"></div>
      </div>
      <button class="btn" onclick="readLight()" style="margin-top:6px">💡 Замерить текущее значение</button>
      <span id="lightValue" style="margin-left:12px;color:var(--dim)"></span>
    </div>

    <div class="card">
      <div class="card-title">Кнопка «Сон» (D7)</div>
      <p class="hint">Физическая кнопка между пином и GND. При нажатии дисплей гаснет / возвращается к яркости по расписанию. Повторное нажатие — возврат.</p>
      <div class="fields-grid">
        <div class="field"><label>Пин кнопки</label><input type="number" id="btnPin" min="0" max="16"></div>
        <div class="field"><label>Антидребезг (мс)</label><input type="number" id="btnDebounce" min="50" max="2000"></div>
      </div>
    </div>
  </section>

  <!-- ============ РАСПИСАНИЕ ============ -->
  <section class="panel" id="panel-schedule">
    <div class="card">
      <div class="card-title">Расписание по дням недели</div>
      <p class="hint">У каждого дня свой режим. <b>⏰ Время</b> — ночь по фиксированному интервалу. <b>☀️ Солнце</b> — по восходу/закату с поправкой. <b>🌅 Гибрид</b> — ночь наступает по первому событию (солнце или время) и не позднее жёсткого лимита.</p>
      <div id="scheduleGrid" style="margin-top:12px"></div>
    </div>
  </section>

  <!-- ============ ПРАЗДНИКИ ============ -->
  <section class="panel" id="panel-holidays">
    <div class="card">
      <div class="card-title">Праздничные дни</div>
      <p class="hint">В эти даты используется расписание <b>воскресенья</b>. Формат: <code>ГГГГ-ММ-ДД</code>.</p>
      <div id="holidayList" style="margin-top:12px"></div>
      <div style="display:flex;gap:8px;margin-top:12px;flex-wrap:wrap">
        <input type="date" id="newHoliday" style="flex:1;min-width:160px;padding:10px;background:var(--bg3);border:1px solid var(--bd);border-radius:8px;color:var(--tx);font-size:14px">
        <button class="btn btn-primary" onclick="addHoliday()">+ Добавить</button>
      </div>
    </div>
  </section>
</main>

<div class="footer-bar">
  <button class="btn btn-primary" onclick="saveAll()">💾 Сохранить</button>
  <button class="btn" onclick="loadAll()">↺ Отменить</button>
</div>

<div class="toast" id="toast"></div>

<script>
const DAYS = [
  {key:"1", name:"Понедельник"},
  {key:"2", name:"Вторник"},
  {key:"3", name:"Среда"},
  {key:"4", name:"Четверг"},
  {key:"5", name:"Пятница"},
  {key:"6", name:"Суббота"},
  {key:"0", name:"Воскресенье"}
];
let config = {};
const $ = id => document.getElementById(id);

// ---------- ВКЛАДКИ ----------
document.querySelectorAll('.tab').forEach(t => {
  t.onclick = () => {
    document.querySelectorAll('.tab').forEach(x => x.classList.toggle('active', x === t));
    document.querySelectorAll('.panel').forEach(p =>
      p.classList.toggle('active', p.id === 'panel-' + t.dataset.tab));
  };
});

// ---------- ЗАГРУЗКА ----------
async function loadAll() {
  try {
    const r = await fetch('/api/config');
    config = await r.json();
    renderAll();
    toast('Конфигурация загружена', 'success');
  } catch (e) { toast('Ошибка загрузки: ' + e, 'error'); }
}

function renderAll() {
  renderWifi(); renderNtp(); renderDisplay();
  renderSchedule(); renderHolidays();
}

// ---------- СТАТУС ----------
async function pollStatus() {
  try {
    const r = await fetch('/api/status');
    const s = await r.json();
    const bar = $('statusBar');
    let h = '';
    if (s.apMode) {
      h += `<div class="status-item"><span class="dot warn"></span>AP: NightClock-Setup</div>`;
    } else if (s.wifiConnected) {
      h += `<div class="status-item"><span class="dot ok"></span>${s.ssid} · ${s.ip}</div>`;
    } else {
      h += `<div class="status-item"><span class="dot err"></span>Wi-Fi: не подключён</div>`;
    }
    h += `<div class="status-item"><span class="dot ${s.ntpSynced?'ok':'warn'}"></span>NTP: ${s.ntpSynced?'OK':'ожидание'}</div>`;
    if (s.time) h += `<div class="status-item">🕐 ${s.time}</div>`;
    h += `<div class="status-item">⏱ ${fmtUptime(s.uptime)}</div>`;
    bar.innerHTML = h;

    // превью TM1637
    if (s.time) {
      const [hh, mm] = s.time.split(':');
      const secOn = parseInt(s.time.split(':')[2] || '0') % 2 === 0;
      $('tm1637').innerHTML = `${hh}<span class="c" style="opacity:${secOn?1:.35}">:</span>${mm}`;
    }

    // info-grid
    $('infoGrid').innerHTML = `
      <div class="info-item"><div class="k">Время</div><div class="v">${s.time || '—'}</div></div>
      <div class="info-item"><div class="k">Дата</div><div class="v">${s.date || '—'}</div></div>
      <div class="info-item"><div class="k">Яркость</div><div class="v">${s.brightness ?? '—'} / 7</div></div>
      <div class="info-item"><div class="k">Сон</div><div class="v">${s.sleep ? '🌙 вкл' : '☀️ выкл'}</div></div>
      <div class="info-item"><div class="k">Свободная память</div><div class="v">${s.heap} B</div></div>
      <div class="info-item"><div class="k">Uptime</div><div class="v">${fmtUptime(s.uptime)}</div></div>`;
  } catch (e) { /* тихо */ }
}

function fmtUptime(ms) {
  const s = Math.floor(ms/1000), d = Math.floor(s/86400);
  const h = Math.floor((s%86400)/3600), m = Math.floor((s%3600)/60);
  return d ? `${d}д ${h}ч ${m}м` : `${h}ч ${m}м`;
}

// ---------- WI-FI ----------
function renderWifi() {
  const list = $('wifiList');
  list.innerHTML = '';
  (config.wifi || []).forEach((w, i) => {
    const row = document.createElement('div');
    row.className = 'wifi-row';
    row.innerHTML = `
      <input type="text" placeholder="SSID" value="${esc(w.ssid||'')}" onchange="config.wifi[${i}].ssid=this.value">
      <input type="password" placeholder="Пароль" value="${esc(w.pass||'')}" onchange="config.wifi[${i}].pass=this.value">
      <button class="btn btn-sm btn-danger" onclick="delWifi(${i})">✕</button>`;
    list.appendChild(row);
  });
  if (!config.wifi || !config.wifi.length)
    list.innerHTML = '<p class="hint">Список сетей пуст.</p>';
}
function addWifi() {
  config.wifi = config.wifi || [];
  config.wifi.push({ssid:'', pass:''});
  renderWifi();
}
function addWifiWithSsid(ssid) {
  config.wifi = config.wifi || [];
  config.wifi.push({ssid, pass:''});
  renderWifi();
  toast('Добавлено: ' + ssid + ' — введите пароль', 'success');
}
function delWifi(i) { config.wifi.splice(i,1); renderWifi(); }
async function scanWifi() {
  const box = $('scanResult');
  box.innerHTML = '<p class="hint">Сканирование…</p>';
  try {
    await fetch('/api/scan/start', { method: 'POST' });
    for (let i = 0; i < 20; i++) {           // до 10 сек
      await new Promise(r => setTimeout(r, 500));
      const res = await fetch('/api/scan/result');
      const data = await res.json();
      if (Array.isArray(data)) {             // скан завершён
        if (!data.length) {
          box.innerHTML = '<p class="hint">Сети не найдены.</p>';
        } else {
          box.innerHTML = data.map(n => `
            <div class="network-row">
              <span class="name">${esc(n.ssid)}</span>
              <span class="rssi">${n.rssi} dBm ${n.encrypted?'🔒':'🔓'}</span>
              <button class="btn btn-sm" onclick="addWifiWithSsid('${esc(n.ssid).replace(/'/g,"\\'")}')">Добавить</button>
            </div>`).join('');
        }
        return;
      }
      // иначе {"status":"running"} — продолжаем
    }
    box.innerHTML = '<p class="hint" style="color:var(--err)">Таймаут сканирования</p>';
  } catch(e) {
    box.innerHTML = `<p class="hint" style="color:var(--err)">Ошибка: ${e}</p>`;
  }
}

// ---------- NTP ----------
function renderNtp() {
  const list = $('ntpList');
  list.innerHTML = '';
  (config.ntp?.servers || []).forEach((s, i) => {
    const row = document.createElement('div');
    row.className = 'ntp-row';
    row.innerHTML = `
      <input type="text" value="${esc(s)}" onchange="config.ntp.servers[${i}]=this.value">
      <button class="btn btn-sm btn-danger" onclick="delNtp(${i})">✕</button>`;
    list.appendChild(row);
  });
  $('posixTz').value     = config.ntp?.posixTz || '<+05>-5';
  $('offsetHours').value = config.ntp?.offsetHours ?? 5;
}
function addNtp() {
  if (!config.ntp) config.ntp = {servers:[]};
  if (config.ntp.servers.length >= 3) { toast('Максимум 3 сервера', 'error'); return; }
  config.ntp.servers.push('');
  renderNtp();
}
function delNtp(i) { config.ntp.servers.splice(i,1); renderNtp(); }

// ---------- ДИСПЛЕЙ ----------
function renderDisplay() {
  $('maxBrightness').value = config.display?.maxBrightness ?? 7;
  $('maxBrightVal').textContent = $('maxBrightness').value;
  $('alEnabled').checked    = !!config.adaptiveLight?.enabled;
  $('alPin').value          = config.adaptiveLight?.pin ?? 17;
  $('alMinRaw').value       = config.adaptiveLight?.minRaw ?? 0;
  $('alMaxRaw').value       = config.adaptiveLight?.maxRaw ?? 1023;
  $('alMinBrightness').value= config.adaptiveLight?.minBrightness ?? 1;
  $('btnPin').value         = config.button?.pin ?? 13;
  $('btnDebounce').value    = config.button?.sleepDebounceMs ?? 300;
}
$('maxBrightness').oninput = e => $('maxBrightVal').textContent = e.target.value;
async function readLight() {
  try {
    const r = await fetch('/api/light');
    const j = await r.json();
    $('lightValue').textContent = `raw = ${j.value}`;
    toast('Текущее значение АЦП: ' + j.value, 'success');
  } catch(e) { toast('Ошибка: ' + e, 'error'); }
}

// ---------- РАСПИСАНИЕ ----------
function renderSchedule() {
  const grid = $('scheduleGrid');
  grid.innerHTML = '';
  DAYS.forEach(d => {
    const s = config.schedule?.[d.key] || {};
    const row = document.createElement('div');
    row.className = 'day-row'; row.dataset.day = d.key;
    row.innerHTML = `
      <div class="day-header">
        <span class="day-name">${d.name}</span>
        <div class="mode-tabs">
          <button class="mode-tab ${(s.mode||'time')==='time'?'active':''}" data-mode="time">⏰ Время</button>
          <button class="mode-tab ${s.mode==='sun'?'active':''}" data-mode="sun">☀️ Солнце</button>
          <button class="mode-tab ${s.mode==='hybrid'?'active':''}" data-mode="hybrid">🌅 Гибрид</button>
        </div>
      </div>
      <div class="fields-grid">
        <div class="field" data-modes="time hybrid"><label>Начало ночи</label>
          <input type="time" name="start" value="${s.start||'23:00'}"></div>
        <div class="field" data-modes="time hybrid"><label>Конец ночи</label>
          <input type="time" name="end" value="${s.end||'07:00'}"></div>
        <div class="field" data-modes="sun hybrid"><label>Смещ. до восхода, мин</label>
          <input type="number" name="sunOffsetStart" value="${s.sunOffsetStart ?? -60}"></div>
        <div class="field" data-modes="sun hybrid"><label>Смещ. после заката, мин</label>
          <input type="number" name="sunOffsetEnd" value="${s.sunOffsetEnd ?? 60}"></div>
        <div class="field" data-modes="sun hybrid"><label>Жёсткий лимит</label>
          <input type="time" name="sunHardLimit" value="${s.sunHardLimit||''}"></div>
        <div class="field" data-modes="time sun hybrid"><label>Яркость днём (0–7)</label>
          <input type="number" name="dayBrightness" min="0" max="7" value="${s.dayBrightness ?? 7}"></div>
        <div class="field" data-modes="time sun hybrid"><label>Яркость ночью (0–7)</label>
          <input type="number" name="nightBrightness" min="0" max="7" value="${s.nightBrightness ?? 1}"></div>
        <div class="field" data-modes="time sun hybrid"><label>Плавность (сек)</label>
          <input type="number" name="fadeSec" min="0" max="60" value="${s.fadeSec ?? 30}"></div>
      </div>`;
    grid.appendChild(row);
    row.querySelectorAll('.mode-tab').forEach(b => b.onclick = () => {
      row.querySelectorAll('.mode-tab').forEach(x => x.classList.toggle('active', x===b));
      updateFieldVisibility(row, b.dataset.mode);
    });
    updateFieldVisibility(row, s.mode || 'time');
  });
}
function updateFieldVisibility(row, mode) {
  row.querySelectorAll('.field').forEach(f =>
    f.style.display = f.dataset.modes.split(' ').includes(mode) ? '' : 'none');
}

// ---------- ПРАЗДНИКИ ----------
function renderHolidays() {
  const list = $('holidayList');
  list.innerHTML = '';
  (config.holidays || []).forEach((d, i) => {
    const row = document.createElement('div');
    row.className = 'holiday-row';
    row.innerHTML = `<span style="flex:1">${d}</span>
      <button class="btn btn-sm btn-danger" onclick="delHoliday(${i})">✕</button>`;
    list.appendChild(row);
  });
  if (!config.holidays || !config.holidays.length)
    list.innerHTML = '<p class="hint">Праздники не заданы.</p>';
}
function addHoliday() {
  const v = $('newHoliday').value;
  if (!v) return;
  config.holidays = config.holidays || [];
  if (config.holidays.includes(v)) return;
  config.holidays.push(v);
  config.holidays.sort();
  renderHolidays();
}
function delHoliday(i) { config.holidays.splice(i,1); renderHolidays(); }

// ---------- СБОРКА И СОХРАНЕНИЕ ----------
function collectFromUI() {
  config.display = config.display || {};
  config.display.maxBrightness = parseInt($('maxBrightness').value);

  config.adaptiveLight = config.adaptiveLight || {};
  config.adaptiveLight.enabled       = $('alEnabled').checked;
  config.adaptiveLight.pin           = parseInt($('alPin').value);
  config.adaptiveLight.minRaw        = parseInt($('alMinRaw').value);
  config.adaptiveLight.maxRaw        = parseInt($('alMaxRaw').value);
  config.adaptiveLight.minBrightness = parseInt($('alMinBrightness').value);

  config.button = config.button || {};
  config.button.pin             = parseInt($('btnPin').value);
  config.button.sleepDebounceMs = parseInt($('btnDebounce').value);

  config.ntp = config.ntp || {};
  config.ntp.posixTz     = $('posixTz').value;
  config.ntp.offsetHours = parseFloat($('offsetHours').value);

  config.schedule = config.schedule || {};
  document.querySelectorAll('.day-row').forEach(row => {
    const day = config.schedule[row.dataset.day] || {};
    const modeBtn = row.querySelector('.mode-tab.active');
    day.mode = modeBtn ? modeBtn.dataset.mode : 'time';
    row.querySelectorAll('input[name]').forEach(inp => {
      day[inp.name] = inp.type === 'number' ? parseFloat(inp.value) : inp.value;
    });
    config.schedule[row.dataset.day] = day;
  });
}

async function saveAll() {
  collectFromUI();
  try {
    const r = await fetch('/api/config', {
      method:'POST',
      headers:{'Content-Type':'application/json'},
      body: JSON.stringify(config)
    });
    if (!r.ok) throw new Error('HTTP ' + r.status);
    toast('Настройки сохранены ✓', 'success');
  } catch(e) { toast('Ошибка: ' + e, 'error'); }
}

async function reboot() {
  if (!confirm('Перезагрузить часы?')) return;
  try { await fetch('/api/reboot', {method:'POST'}); }
  catch(e) {}
  toast('Перезагрузка…', 'success');
}

// ---------- УТИЛИТЫ ----------
function esc(s) {
  return String(s).replace(/[&<>"']/g, m => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[m]));
}
let toastTimer = null;
function toast(msg, type='success') {
  const t = $('toast');
  t.textContent = msg;
  t.className = 'toast show ' + type;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => t.className = 'toast ' + type, 3000);
}

// ---------- СТАРТ ----------
loadAll();
pollStatus();
setInterval(pollStatus, 3000);
</script>
</body>
</html>)HTML";

