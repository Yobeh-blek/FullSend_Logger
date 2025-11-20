/* app.js
   SPA logic for Full Send Logger (Option A)
   - File open/save
   - Browse layout (create zip with CSV)
   - Web Serial API connect (simple)
   - Dashboard with charts (Chart.js)
*/

(() => {
  // --- Utilities ---
  const $ = sel => document.querySelector(sel);
  const $all = sel => Array.from(document.querySelectorAll(sel));
  function fmtTime(ms){
    if (ms === null) return "00:00.000";
    const s = Math.floor(ms/1000);
    const m = Math.floor(s/60);
    const sec = s%60;
    const msPart = Math.floor(ms%1000).toString().padStart(3,'0');
    return `${m.toString().padStart(2,'0')}:${sec.toString().padStart(2,'0')}.${msPart}`;
  }

  // State
  const state = {
    layouts: [], // loaded layout objects
    selectedLayout: null,
    historyRows: [],
    sessionData: [], // live stream rows
    connectedPort: null,
    reader: null,
    keepReading: false
  };

  // --- Elements ---
  const layoutFilesInput = $('#layoutFilesInput');
  const historyFileInput = $('#historyFileInput');
  const btnBrowseLayouts = $('#btnBrowseLayouts');
  const btnOpenHistory = $('#btnOpenHistory');
  const btnSaveData = $('#btnSaveData');
  const btnCancelFile = $('#btnCancelFile');
  const selectedLayoutDiv = $('#selectedLayout');
  const btnCreateLayoutZip = $('#btnCreateLayoutZip');

  const btnRadioConnect = $('#btnRadioConnect');
  const radioPortSelect = $('#radioPortSelect');
  const radioBaud = $('#radioBaud');

  const btnDataConnect = $('#btnDataConnect');
  const dataPortSelect = $('#dataPortSelect');
  const dataBaud = $('#dataBaud');

  const historyList = $('#historyList');

  // Dashboard elements
  const dashboard = $('#dashboard');
  const appMain = $('#app');
  const backToMain = $('#backToMain');
  const dashTrackName = $('#dashTrackName');
  const headingIndicator = $('#headingIndicator');
  const lapTimeEl = $('#lapTime');
  const tireTempEl = $('#tireTemp');
  const tirePressureEl = $('#tirePressure');
  const liveDataTableBody = $('#liveDataTable tbody');
  const btnGotoCompare = $('#gotoCompare');
  const downloadSession = $('#downloadSession');

  // Compare
  const comparePage = $('#compare');
  const backFromCompare = $('#backFromCompare');
  const lapSelectionArea = $('#lapSelectionArea');
  const compareButton = $('#compareButton');

  // Charts - create Chart.js charts with empty data
  let speedChart = null, timeDistChart = null, compareChart = null;

  function createCharts(){
    const ctxS = document.getElementById('chartSpeed').getContext('2d');
    speedChart = new Chart(ctxS, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{label:'Speed', data:[], tension:0.2}]
      },
      options: { responsive:true, maintainAspectRatio:false, scales:{x:{display:true}}}
    });

    const ctxTD = document.getElementById('chartTimeDistance').getContext('2d');
    timeDistChart = new Chart(ctxTD, {
      type: 'line',
      data: { labels: [], datasets:[{label:'Time vs Dist', data:[], tension:0.2}] },
      options:{responsive:true, maintainAspectRatio:false}
    });

    const ctxC = document.getElementById('compareChart').getContext('2d');
    compareChart = new Chart(ctxC, {
      type: 'line',
      data: { labels: [], datasets:[] },
      options:{responsive:true}
    });
  }

  // --- File: Browse layouts ---
  btnBrowseLayouts.addEventListener('click', () => layoutFilesInput.click());
  layoutFilesInput.addEventListener('change', async (ev) => {
    const files = Array.from(ev.target.files);
    // Accept JSON or CSV or ZIP - we attempt to parse JSON or CSV
    state.layouts = [];
    for (const f of files){
      try {
        const txt = await f.text();
        if (f.name.toLowerCase().endsWith('.json')){
          const obj = JSON.parse(txt);
          obj.__filename = f.name;
          state.layouts.push(obj);
        } else if (f.name.toLowerCase().endsWith('.csv')){
          // CSV parser for minimal layout format: name,startLat,startLon,finishLat,finishLon,entryLat,entryLon,exitLat,exitLon
          const r = txt.split(/\r?\n/).filter(Boolean)[0]||'';
          const fields = r.split(',');
          const obj = {name: f.name.replace('.csv',''), start:[fields[1],fields[2]], finish:[fields[3],fields[4]], entry:[fields[5],fields[6]], exit:[fields[7],fields[8]]};
          obj.__filename = f.name;
          state.layouts.push(obj);
        } else {
          // skip or handle zip later
        }
      } catch(err){
        console.warn('layout parse fail', f.name, err);
      }
    }
    renderLayoutSelection();
  });

  function renderLayoutSelection(){
    if (state.layouts.length === 0){
      selectedLayoutDiv.textContent = '— none —';
    } else {
      selectedLayoutDiv.textContent = state.layouts.map(l => l.name || l.__filename).join(', ');
      // select first by default
      state.selectedLayout = state.layouts[0];
    }
  }

  // Create folder (ZIP) containing CSV of start/finish/entry/exit coordinates
  btnCreateLayoutZip.addEventListener('click', async () => {
    if (!state.selectedLayout) {
      alert('No layout selected. Please Browse and choose a layout file first.');
      return;
    }
    const layout = state.selectedLayout;
    // create CSV with header
    const csv = `name,start_lat,start_lon,finish_lat,finish_lon,entry_lat,entry_lon,exit_lat,exit_lon
${layout.name || layout.__filename},${layout.start?.[0]||''},${layout.start?.[1]||''},${layout.finish?.[0]||''},${layout.finish?.[1]||''},${layout.entry?.[0]||''},${layout.entry?.[1]||''},${layout.exit?.[0]||''},${layout.exit?.[1]||''}
`;

    const zip = new JSZip();
    const folderName = (layout.name || layout.__filename || 'layout').replace(/\s+/g,'_');
    const folder = zip.folder(folderName);
    folder.file(`${folderName}.csv`, csv);

    // Optionally include a metadata JSON
    folder.file('layout.json', JSON.stringify(layout, null, 2));

    const content = await zip.generateAsync({type:'blob'});

    // trigger download
    const url = URL.createObjectURL(content);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${folderName}.zip`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    URL.revokeObjectURL(url);

    alert('Layout folder ZIP created and downloaded. On an ESP32-backed server you would create a folder and write the CSV to SD instead.');
  });

  // --- History open/save ---
  btnOpenHistory.addEventListener('click', () => historyFileInput.click());
  historyFileInput.addEventListener('change', async (ev) => {
    const f = ev.target.files[0];
    if (!f) return;
    const txt = await f.text();
    parseHistoryCSV(txt);
  });

  function parseHistoryCSV(txt){
    // naive CSV parser: assume header exists
    const rows = txt.split(/\r?\n/).filter(Boolean);
    const header = rows[0].split(',');
    const dataRows = rows.slice(1).map(r => {
      const cols = r.split(',');
      const obj = {};
      header.forEach((h,i)=> obj[h.trim()]=cols[i]);
      return obj;
    });
    state.historyRows = dataRows;
    renderHistory();
  }

  function renderHistory(){
    historyList.innerHTML = '';
    if (state.historyRows.length === 0) {
      historyList.innerHTML = `<div class="history-item">No history loaded</div>`;
      return;
    }
    state.historyRows.slice(0,50).forEach((r,i) => {
      const div = document.createElement('div');
      div.className = 'history-item';
      div.innerHTML = `<div>Row ${i+1}: ${r.time || r.timestamp || ''}</div><div><button data-index="${i}">View</button></div>`;
      historyList.appendChild(div);
    });
    // attach view buttons
    historyList.querySelectorAll('button').forEach(btn => {
      btn.addEventListener('click', (e)=>{
        const idx = +e.target.dataset.index;
        alert(JSON.stringify(state.historyRows[idx], null, 2));
      });
    });
  }

  btnSaveData.addEventListener('click', () => {
    // save sessionData as CSV
    if (!state.sessionData.length){
      alert('No session data to save.');
      return;
    }
    const header = Object.keys(state.sessionData[0]);
    const csv = [header.join(',')].concat(state.sessionData.map(r => header.map(h => r[h] ?? '').join(','))).join('\n');
    const blob = new Blob([csv], {type:'text/csv'});
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `session_${Date.now()}.csv`;
    document.body.appendChild(a); a.click(); a.remove(); URL.revokeObjectURL(url);
  });

  btnCancelFile.addEventListener('click', () => {
    state.layouts = []; state.selectedLayout = null; layoutFilesInput.value = '';
    selectedLayoutDiv.textContent = '— none —';
  });

  // --- Web Serial API (simple) ---
  async function requestPortAndConnect(selectElement, baudSelector, role){
    if (!('serial' in navigator)){
      alert('Web Serial API not supported in this browser. Use Chrome/Edge with secure context (https or localhost).');
      return;
    }
    try {
      const port = await navigator.serial.requestPort();
      // open with baud rate
      await port.open({ baudRate: +baudSelector.value });
      // fill select
      addPortToSelect(selectElement, port);
      // store if role= data panel
      if (role === 'data') {
        state.connectedPort = port;
        startReadingFromPort(port);
      }
      return port;
    } catch (err){
      console.error('serial connect',err);
    }
  }

  function addPortToSelect(selectEl, port){
    const opt = document.createElement('option');
    opt.value = Math.random().toString(36).slice(2,9);
    opt.textContent = `Device ${opt.value.slice(0,4)}`;
    opt._portRef = port;
    selectEl.appendChild(opt);
    selectEl.value = opt.value;
  }

  btnRadioConnect.addEventListener('click', async () => requestPortAndConnect(radioPortSelect, radioBaud, 'radio'));
  btnDataConnect.addEventListener('click', async () => {
    // when pressing Data Page, open the dashboard view
    const p = await requestPortAndConnect(dataPortSelect, dataBaud, 'data');
    // navigate to dashboard
    showDashboard();
    if (!speedChart) createCharts();
  });

  // Read from connected port (Data)
  async function startReadingFromPort(port){
    if (!port) return;
    stopReading();
    state.keepReading = true;
    // set up readable stream
    try {
      const textDecoder = new TextDecoderStream();
      const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
      const reader = textDecoder.readable.getReader();
      state.reader = reader;
      // read loop
      while (state.keepReading) {
        const { value, done } = await reader.read();
        if (done) break;
        if (value) {
          handleSerialLine(value);
        }
      }
    } catch (err){
      console.error('read error', err);
    }
  }

  function stopReading(){
    state.keepReading = false;
    if (state.reader) {
      try { state.reader.cancel(); } catch(e){}
      state.reader = null;
    }
    if (state.connectedPort && state.connectedPort.readable){
      // keep port open—closing left to user
    }
  }

  // Simple parser: assume device sends newline-delimited JSON objects or CSV rows
  function handleSerialLine(line){
    // many devices send many lines at once; split by newline
    const lines = line.split(/\r?\n/).filter(Boolean);
    for (const l of lines){
      let parsed = null;
      try {
        parsed = JSON.parse(l);
      } catch(e){
        // try CSV
        const parts = l.split(',');
        if (parts.length >= 5){
          parsed = { time: parts[0], distance:parts[1], speed:parts[2], brake:parts[3], gas:parts[4] };
        } else {
          // ignore
          continue;
        }
      }
      if (parsed) pushSessionRow(parsed);
    }
  }

  function pushSessionRow(row){
    // ensure timestamps
    row._ts = Date.now();
    state.sessionData.push(row);
    // limit kept rows
    if (state.sessionData.length > 2000) state.sessionData.shift();

    // update dashboard live UI
    updateLiveTable(row);
    updateCharts();
    updateTopbar(row);
  }

  function updateTopbar(row){
    if (row.heading !== undefined) headingIndicator.textContent = `${Math.round(row.heading)}°`;
    if (row.laptime !== undefined) lapTimeEl.textContent = row.laptime;
    if (row.tireTemp !== undefined) tireTempEl.textContent = `FL: ${row.tireTemp} / FR: ${row.tireTempFR || '—'}`;
    if (row.pressure !== undefined) tirePressureEl.textContent = `Pressure: ${row.pressure}`;
  }

  function updateLiveTable(row){
    const tr = document.createElement('tr');
    const t = new Date(row._ts).toLocaleTimeString();
    tr.innerHTML = `<td>${t}</td><td>${row.distance ?? ''}</td><td>${row.speed ?? ''}</td><td>${row.brake ?? ''}</td><td>${row.gas ?? ''}</td>`;
    liveDataTableBody.prepend(tr);
    // keep only 200 rows in DOM
    while (liveDataTableBody.children.length > 200) liveDataTableBody.removeChild(liveDataTableBody.lastChild);
  }

  function updateCharts(){
    if (!speedChart || !timeDistChart) return;
    const lastN = state.sessionData.slice(-150);
    const labels = lastN.map(r => new Date(r._ts).toLocaleTimeString());
    speedChart.data.labels = labels;
    speedChart.data.datasets[0].data = lastN.map(r=>parseFloat(r.speed || 0));
    speedChart.update('none');

    timeDistChart.data.labels = labels;
    timeDistChart.data.datasets[0].data = lastN.map(r=>parseFloat(r.distance || 0));
    timeDistChart.update('none');
  }

  // Navigation
  function showDashboard(){
    appMain.style.display = 'none';
    dashboard.style.display = 'block';
    comparePage.style.display = 'none';
    dashTrackName.textContent = `Track: ${state.selectedLayout?.name || state.selectedLayout?.__filename || '—'}`;
  }
  function showMain(){
    appMain.style.display = 'block';
    dashboard.style.display = 'none';
    comparePage.style.display = 'none';
  }
  function showCompare(){
    appMain.style.display = 'none';
    dashboard.style.display = 'none';
    comparePage.style.display = 'block';
    populateLapSelection();
  }

  backToMain.addEventListener('click', ()=> {
    stopReading();
    showMain();
  });
  $('#gotoCompare')?.addEventListener('click', showCompare);
  backFromCompare.addEventListener('click', () => dashboard.style.display = 'block' && comparePage.style.display='none');

  // Download session CSV from dashboard
  downloadSession.addEventListener('click', () => {
    if (!state.sessionData.length) { alert('No session data'); return; }
    const header = Object.keys(state.sessionData[0]).filter(k=>!k.startsWith('_'));
    const csv = [header.join(',')].concat(state.sessionData.map(r => header.map(h => r[h] ?? '').join(','))).join('\n');
    const blob = new Blob([csv], {type:'text/csv'});
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a'); a.href = url; a.download = `session_${Date.now()}.csv`; document.body.appendChild(a); a.click(); a.remove(); URL.revokeObjectURL(url);
  });

  // Lap comparison UI
  function populateLapSelection(){
    lapSelectionArea.innerHTML = '';
    // For demo: treat each 30-second block of sessionData as a lap
    const lapLength = 30 * 1000;
    const laps = {};
    for (const r of state.sessionData){
      const lapIndex = Math.floor((r._ts - state.sessionData[0]._ts) / lapLength);
      if (!laps[lapIndex]) laps[lapIndex] = [];
      laps[lapIndex].push(r);
    }
    const keys = Object.keys(laps);
    if (!keys.length){
      lapSelectionArea.innerHTML = '<div>No laps available (need session data)</div>';
      return;
    }
    keys.forEach(k => {
      const el = document.createElement('div');
      el.innerHTML = `<label><input type="checkbox" value="${k}" /> Lap ${k} (${laps[k].length} samples)</label>`;
      lapSelectionArea.appendChild(el);
    });
  }

  compareButton.addEventListener('click', () => {
    // collect selected lap IDs and overlay speed
    const checked = Array.from(lapSelectionArea.querySelectorAll('input:checked')).map(i => +i.value);
    if (!checked.length){ alert('choose 1 or more laps'); return; }
    const lapLength = 30 * 1000;
    const laps = {};
    for (const r of state.sessionData){
      const lapIndex = Math.floor((r._ts - state.sessionData[0]._ts) / lapLength);
      if (!laps[lapIndex]) laps[lapIndex] = [];
      laps[lapIndex].push(r);
    }
    // build datasets
    compareChart.data.labels = []; compareChart.data.datasets = [];
    let maxLen = 0;
    checked.forEach((id, idx) => {
      const arr = laps[id] || [];
      const labels = arr.map((s,i)=>i);
      if (labels.length > maxLen) maxLen = labels.length;
      const ds = { label: `Lap ${id}`, data: arr.map(a => parseFloat(a.speed||0)) };
      compareChart.data.datasets.push(ds);
    });
    compareChart.update();
    // switch to compare page already
  });

  // Initialize
  function init(){
    renderLayoutSelection();
    renderHistory();
    // create initial chart placeholders lazily when entering dashboard
    // attach some safety checks
    if (!window.Chart) console.warn('Chart.js not loaded — charts will not work.');
    if (!window.JSZip) console.warn('JSZip not loaded — layout zip creation will not work.');
  }

  init();

  // Expose small helpers for debugging
  window._fullSend = { state, pushSessionRow };
})();
