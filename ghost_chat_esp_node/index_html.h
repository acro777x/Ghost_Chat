// Auto-generated - Ghost Chat V5 HTML UI
#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
<title>Ghost Network</title>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
<style>
:root {
  --bg: #0d1117;
  --bg-pattern: url('data:image/svg+xml;utf8,<svg width="60" height="60" viewBox="0 0 60 60" xmlns="http://www.w3.org/2000/svg"><g fill="none" fill-rule="evenodd"><g fill="%23ffffff" fill-opacity="0.03"><path d="M36 34v-4h-2v4h-4v2h4v4h2v-4h4v-2h-4zm0-30V0h-2v4h-4v2h4v4h2V6h4V4h-4zM6 34v-4H4v4H0v2h4v4h2v-4h4v-2H6zM6 4V0H4v4H0v2h4v4h2V6h4V4H6z"/></g></g></svg>');
  --panel: rgba(22, 27, 34, 0.85);
  --border: rgba(255, 255, 255, 0.1);
  --accent: #007aff; /* iOS Blue */
  --accent-glow: rgba(0, 122, 255, 0.5);
  --sent: linear-gradient(145deg, #007aff, #0056b3);
  --recv: rgba(33, 38, 45, 0.9);
  --text: #f0f6fc;
  --text-muted: #8b949e;
  --danger: #ff453a;
  --success: #32d74b;
  --lora: #ff9f0a;
}

* { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif; -webkit-tap-highlight-color: transparent; }

body { background-color: var(--bg); background-image: var(--bg-pattern); color: var(--text); display: flex; flex-direction: column; height: 100vh; overflow: hidden; }

/* ======== TYPOGRAPHY & UTILS ======== */
.fw-500 { font-weight: 500; }
.fw-600 { font-weight: 600; }
.fw-700 { font-weight: 700; }
.fw-800 { font-weight: 800; }
.text-xs { font-size: 11px; }
.text-sm { font-size: 13px; }
.text-md { font-size: 15px; }
.text-lg { font-size: 17px; }

/* ======== TOASTS ======== */
#toasts { position: fixed; top: 15px; left: 50%; transform: translateX(-50%); z-index: 9999; display: flex; flex-direction: column; gap: 8px; width: 90%; max-width: 340px; pointer-events: none; }
.toast { background: rgba(30, 30, 30, 0.95); backdrop-filter: blur(15px); -webkit-backdrop-filter: blur(15px); border-radius: 12px; padding: 12px 16px; font-size: 14px; font-weight: 600; color: white; display: flex; align-items: center; gap: 10px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); animation: toastIn 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275) forwards; border: 1px solid var(--border); }
.toast.err { border-left: 4px solid var(--danger); }
.toast.ok { border-left: 4px solid var(--success); }
.toast.out { animation: toastOut 0.3s forwards; }
@keyframes toastIn { from { opacity: 0; transform: translateY(-20px) scale(0.95); } to { opacity: 1; transform: translateY(0) scale(1); } }
@keyframes toastOut { to { opacity: 0; transform: translateY(-10px) scale(0.95); } }

/* ======== LOGIN SCREEN ======== */
#lm { position: fixed; inset: 0; background: var(--bg); background-image: var(--bg-pattern); z-index: 9999; display: flex; align-items: center; justify-content: center; padding: 20px; animation: fadeIn 0.5s; }
.login-box { width: 100%; max-width: 360px; background: var(--panel); backdrop-filter: blur(20px); -webkit-backdrop-filter: blur(20px); padding: 40px 24px; border-radius: 24px; border: 1px solid var(--border); box-shadow: 0 25px 50px -12px rgba(0,0,0,0.5); text-align: center; }
.logo-icon { width: 72px; height: 72px; background: var(--sent); border-radius: 20px; display: flex; align-items: center; justify-content: center; font-size: 36px; margin: 0 auto 20px; box-shadow: 0 10px 25px var(--accent-glow); transform: rotate(-5deg); border: 2px solid rgba(255,255,255,0.2); }
.login-title { font-size: 28px; font-weight: 800; margin-bottom: 8px; background: linear-gradient(to right, #fff, #a5b4fc); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
.login-sub { font-size: 14px; color: var(--text-muted); margin-bottom: 32px; font-weight: 500; line-height: 1.4; }
.input-group { position: relative; margin-bottom: 16px; text-align: left; }
.login-input { width: 100%; background: rgba(0,0,0,0.5); border: 1px solid var(--border); padding: 14px 16px; border-radius: 12px; color: white; font-size: 16px; outline: none; transition: all 0.2s; }
.login-input:focus { border-color: var(--accent); box-shadow: 0 0 0 3px var(--accent-glow); background: rgba(0,0,0,0.8); }
.login-input::placeholder { color: #6e7681; }
.btn-primary { width: 100%; background: var(--sent); color: white; border: none; padding: 16px; border-radius: 12px; font-size: 16px; font-weight: 700; cursor: pointer; transition: all 0.2s; box-shadow: 0 8px 20px var(--accent-glow); margin-top: 8px; }
.btn-primary:active { transform: scale(0.96); box-shadow: 0 4px 10px var(--accent-glow); }

/* ======== HEADER ======== */
.header { height: 60px; background: rgba(22, 27, 34, 0.75); backdrop-filter: blur(25px); -webkit-backdrop-filter: blur(25px); display: flex; align-items: center; justify-content: space-between; padding: 0 16px; border-bottom: 1px solid var(--border); box-shadow: 0 4px 20px rgba(0,0,0,0.3); z-index: 100; flex-shrink: 0; }
.h-left { display: flex; align-items: center; gap: 12px; }
.room-avatar { width: 38px; height: 38px; border-radius: 50%; background: linear-gradient(135deg, #6366f1, #3b82f6); display: flex; align-items: center; justify-content: center; font-size: 18px; font-weight: 700; box-shadow: 0 4px 10px rgba(0,122,255,0.3); }
.room-info { display: flex; flex-direction: column; }
.room-name { font-size: 16px; font-weight: 700; letter-spacing: 0.2px; display: flex; align-items: center; gap: 6px; }
.status-dot { width: 8px; height: 8px; background: var(--success); border-radius: 50%; box-shadow: 0 0 8px var(--success); }
.status-text { font-size: 12px; color: var(--accent); font-weight: 500; margin-top: 2px; }
.h-right { display: flex; gap: 8px; }
.icon-btn { width: 36px; height: 36px; border-radius: 50%; background: rgba(255,255,255,0.05); border: none; color: white; display: flex; align-items: center; justify-content: center; font-size: 16px; cursor: pointer; transition: all 0.2s; }
.icon-btn:active { transform: scale(0.9); background: rgba(255,255,255,0.15); }

/* ======== BANNERS ======== */
.banner { width: 100%; padding: 8px 16px; font-size: 12px; font-weight: 600; display: flex; align-items: center; gap: 8px; z-index: 99; backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px); }
.banner-lora { background: rgba(255, 159, 10, 0.15); border-bottom: 1px solid rgba(255, 159, 10, 0.2); color: var(--lora); display: none; }
.banner-pin { background: rgba(0, 122, 255, 0.1); border-bottom: 1px solid rgba(0, 122, 255, 0.2); color: #fff; display: none; border-left: 4px solid var(--accent); cursor: pointer; }

/* ======== CHAT AREA ======== */
.chat-container { position: absolute; top: 60px; bottom: 70px; left: 0; right: 0; overflow-y: auto; padding: 16px 14px; display: flex; flex-direction: column; gap: 12px; scroll-behavior: smooth; }
.message-row { display: flex; width: 100%; align-items: flex-end; gap: 8px; }
.message-row.sent { justify-content: flex-end; }
.message-row.recv { justify-content: flex-start; }
.avatar { width: 28px; height: 28px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 12px; font-weight: 700; color: white; box-shadow: 0 2px 5px rgba(0,0,0,0.2); flex-shrink: 0; }
.message-row.sent .avatar { display: none; }

.bubble-wrap { display: flex; flex-direction: column; max-width: 78%; }
.message-row.sent .bubble-wrap { align-items: flex-end; }

.bubble { padding: 10px 14px; font-size: 15px; line-height: 1.4; word-wrap: break-word; position: relative; box-shadow: 0 2px 8px rgba(0,0,0,0.15); }
.message-row.sent .bubble { background: var(--sent); color: white; border-radius: 20px 20px 4px 20px; }
.message-row.recv .bubble { background: var(--recv); border: 1px solid var(--border); color: var(--text); border-radius: 20px 20px 20px 4px; }

.sender-name { font-size: 12px; font-weight: 700; margin-bottom: 4px; display: block; opacity: 0.9; }
.message-meta { font-size: 11px; margin-top: 4px; display: flex; align-items: center; gap: 6px; float: right; margin-left: 14px; color: rgba(255,255,255,0.6); }

/* Image & Bomb Attachments */
.chat-img { max-width: 220px; max-height: 280px; border-radius: 12px; margin-top: 4px; object-fit: cover; border: 1px solid rgba(255,255,255,0.1); cursor: pointer; }
.img-wrap { position: relative; display: inline-block; }
.del-btn { position: absolute; top: 8px; right: 8px; background: rgba(0,0,0,0.6); backdrop-filter: blur(5px); color: white; border: none; width: 28px; height: 28px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 14px; cursor: pointer; }

.bomb-card { background: rgba(255, 69, 58, 0.1); border: 1px solid rgba(255, 69, 58, 0.3); padding: 12px 16px; border-radius: 14px; border-left: 4px solid var(--danger); display: flex; align-items: center; gap: 12px; cursor: pointer; margin-top: 4px; }
.bomb-icon { font-size: 24px; animation: shake 2s infinite ease-in-out; display: inline-block; }
@keyframes shake { 0%, 100% { transform: rotate(0deg); } 25% { transform: rotate(10deg); } 75% { transform: rotate(-10deg); } }
.bomb-text { font-size: 15px; font-weight: 600; color: #ff8a80; }

.sd-badge { background: rgba(255, 69, 58, 0.2); padding: 2px 6px; border-radius: 6px; color: #ff8a80; font-weight: 700; font-size: 10px; border: 1px solid rgba(255,69,58,0.3); }
.sys-msg { text-align: center; margin: 12px 0; }
.sys-msg span { background: rgba(0,0,0,0.5); padding: 6px 12px; font-size: 12px; font-weight: 600; border-radius: 16px; color: var(--text-muted); border: 1px solid var(--border); }

/* ======== INPUT AREA ======== */
.input-area { position: fixed; bottom: 0; left: 0; width: 100%; background: rgba(22, 27, 34, 0.85); backdrop-filter: blur(25px); -webkit-backdrop-filter: blur(25px); padding: 10px 14px; padding-bottom: max(10px, env(safe-area-inset-bottom)); border-top: 1px solid var(--border); display: flex; align-items: center; flex-direction: column; z-index: 100; min-height: 70px; }
.typing-bar { width: 100%; font-size: 12px; color: var(--accent); font-weight: 600; padding: 0 4px 6px; height: 18px; display: flex; align-items: center; }

.input-row { display: flex; gap: 10px; width: 100%; align-items: flex-end; }
.fab-btn { width: 42px; height: 42px; border-radius: 50%; background: var(--recv); border: 1px solid var(--border); color: var(--text-muted); font-size: 24px; display: flex; align-items: center; justify-content: center; cursor: pointer; transition: all 0.2s; flex-shrink: 0; }
.fab-btn.active { transform: rotate(45deg); background: rgba(255,255,255,0.1); color: white; }

.input-box { flex: 1; min-height: 42px; background: rgba(0,0,0,0.4); border: 1px solid var(--border); border-radius: 21px; display: flex; align-items: center; padding: 0 16px; gap: 8px; }
.chat-input { flex: 1; background: transparent; border: none; color: white; font-size: 16px; padding: 10px 0; outline: none; }
.chat-input::placeholder { color: #6e7681; }

.send-btn { width: 42px; height: 42px; border-radius: 50%; background: var(--sent); border: none; color: white; display: flex; align-items: center; justify-content: center; cursor: pointer; transition: all 0.15s; font-size: 18px; box-shadow: 0 4px 12px var(--accent-glow); flex-shrink: 0; }
.send-btn:active { transform: scale(0.9); }

/* ======== iOS ATTACHMENT SHEET ======== */
.overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.4); z-index: 2000; opacity: 0; pointer-events: none; transition: opacity 0.3s; backdrop-filter: blur(5px); -webkit-backdrop-filter: blur(5px); }
.overlay.show { opacity: 1; pointer-events: auto; }
.action-sheet { position: fixed; bottom: 0; left: 0; width: 100%; background: rgba(30, 30, 30, 0.95); backdrop-filter: blur(25px); -webkit-backdrop-filter: blur(25px); border-top-left-radius: 20px; border-top-right-radius: 20px; padding: 24px 16px max(24px, env(safe-area-inset-bottom)); z-index: 2100; transform: translateY(100%); transition: transform 0.3s cubic-bezier(0.175, 0.885, 0.32, 1); box-shadow: 0 -10px 40px rgba(0,0,0,0.5); }
.action-sheet.show { transform: translateY(0); }
.sheet-row { display: flex; gap: 20px; justify-content: center; margin-bottom: 24px; }
.sheet-item { display: flex; flex-direction: column; align-items: center; gap: 8px; cursor: pointer; }
.sheet-icon { width: 60px; height: 60px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 28px; color: white; box-shadow: 0 8px 20px rgba(0,0,0,0.3); transition: transform 0.1s; border: 1px solid rgba(255,255,255,0.1); }
.sheet-item:active .sheet-icon { transform: scale(0.9); }
.sheet-icon.img { background: linear-gradient(135deg, #0a84ff, #0056b3); }
.sheet-icon.bomb { background: linear-gradient(135deg, #ff453a, #b91c1c); }
.sheet-icon.timer { background: linear-gradient(135deg, #32d74b, #00871d); }
.sheet-label { font-size: 13px; font-weight: 600; color: #f0f6fc; }

/* ======== SD TIMER PICKER ======== */
.timer-list { display: none; flex-direction: column; gap: 10px; width: 100%; }
.timer-opt { background: rgba(255,255,255,0.05); padding: 16px; border-radius: 14px; font-size: 16px; font-weight: 700; display: flex; gap: 12px; align-items: center; cursor: pointer; border: 1px solid var(--border); }
.timer-opt:active { background: rgba(255,255,255,0.1); }
.timer-opt.off { color: var(--danger); background: rgba(255,69,58,0.1); border-color: rgba(255,69,58,0.3); }

/* ======== BOMB VIEWER ======== */
#bombView { position: fixed; inset: 0; background: #000; z-index: 5000; display: none; flex-direction: column; align-items: center; justify-content: center; }
#bombImg { max-width: 100%; max-height: 80%; display: none; border-radius: 16px; }
.hold-circle { width: 100px; height: 100px; border-radius: 50%; border: 4px solid var(--danger); display: flex; align-items: center; justify-content: center; font-size: 40px; color: var(--danger); user-select: none; margin-top: 40px; animation: pulseRed 1.5s infinite; }
@keyframes pulseRed { 0% { box-shadow: 0 0 0 0 rgba(255,69,58,0.4); } 70% { box-shadow: 0 0 0 20px rgba(255,69,58,0); } 100% { box-shadow: 0 0 0 0 rgba(255,69,58,0); }}
.hold-hint { color: var(--text-muted); font-size: 14px; font-weight: 700; margin-top: 24px; text-transform: uppercase; letter-spacing: 1px; }

/* ======== CONTEXT MENU ======== */
#ctxMenu { position: fixed; background: rgba(30, 30, 30, 0.95); backdrop-filter: blur(20px); -webkit-backdrop-filter: blur(20px); border: 1px solid var(--border); border-radius: 14px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); z-index: 3000; display: none; min-width: 180px; padding: 6px; }
.ctx-item { padding: 12px 14px; font-size: 15px; font-weight: 600; color: white; display: flex; align-items: center; gap: 12px; border-radius: 8px; cursor: pointer; }
.ctx-item:active { background: rgba(255,255,255,0.1); }
</style>
</head>
<body>

<!-- Hidden File Inputs removed since they are now embedded directly in the action sheet -->

<div id="toasts"></div>

<!-- Login Screen -->
<div id="lm">
  <div class="login-box">
    <div class="logo-icon">👻</div>
    <div class="login-title">Ghost Network</div>
    <div class="login-sub">Secure • Decentralized • Anonymous</div>
    
    <div class="input-group">
      <input type="text" id="ni" class="login-input" placeholder="Alias Name" maxlength="20">
    </div>
    <div class="input-group">
      <input type="password" id="ki" class="login-input" placeholder="Room Password">
    </div>
    
    <button class="btn-primary" onclick="doLogin()">JOIN SECURELY</button>
  </div>
</div>

<!-- Header -->
<div class="header">
  <div class="h-left">
    <div class="room-avatar" id="ra">G</div>
    <div class="room-info">
      <div class="room-name"><span id="roomHVal">Connecting</span> <div class="status-dot" id="cdot"></div></div>
      <div class="status-text" id="hs">waiting...</div>
    </div>
  </div>
  <div class="h-right">
    <button class="icon-btn" onclick="showStat()" title="Stats">📊</button>
    <button class="icon-btn" onclick="clrChat()" title="Clear">🗑️</button>
    <button class="icon-btn" onclick="doLogout()" title="Leave">🚪</button>
  </div>
</div>

<div class="banner banner-pin" id="pinBar" onclick="unpin()">
  <span style="font-size:16px">📌</span>
  <span style="flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;" id="pc"></span>
</div>

<div class="banner banner-lora" id="lorabar">
  📡 <span>Off-grid Mesh Node Active (<span id="loratxt"></span> dBm)</span>
</div>

<!-- Chat Area -->
<div class="chat-container" id="chatbox"></div>

<!-- Input Area -->
<div class="input-area">
  <div class="typing-bar" id="tn"></div>
  <div class="input-row">
    <button class="fab-btn" id="fabBtn" onclick="toggleSheet()">+</button>
    <div class="input-box">
      <span id="sdbadge" class="sd-badge" style="display:none"></span>
      <input type="text" id="mi" class="chat-input" placeholder="Message..." oninput="onType()" onkeydown="if(event.key==='Enter'&&!event.shiftKey){event.preventDefault();sendMsg()}">
    </div>
    <button class="send-btn" onclick="sendMsg()">➤</button>
  </div>
</div>

<!-- Attachment Action Sheet (iOS Style) -->
<div class="overlay" id="overlay" onclick="closeSheet()"></div>
<div class="action-sheet" id="actionSheet">
  <!-- Default Options -->
  <div id="sheetOpts" class="sheet-row">
    
    <div class="sheet-item" style="position:relative;">
      <input type="file" id="fi" accept="image/*" style="position:absolute; inset:0; opacity:0; z-index:10; width:100%; height:100%; cursor:pointer;" onchange="handleUpload(false); closeSheet();">
      <div class="sheet-icon img">🖼️</div>
      <span class="sheet-label">Image</span>
    </div>

    <div class="sheet-item" style="position:relative;">
      <input type="file" id="vi" accept="image/*" style="position:absolute; inset:0; opacity:0; z-index:10; width:100%; height:100%; cursor:pointer;" onchange="handleUpload(true); closeSheet();">
      <div class="sheet-icon bomb">💣</div>
      <span class="sheet-label">Bomb Image</span>
    </div>

    <div class="sheet-item" onclick="showTimers()">
      <div class="sheet-icon timer">⏱️</div>
      <span class="sheet-label">Bomb Text</span>
    </div>
  </div>
  
  <!-- SD Timers -->
  <div id="timerList" class="timer-list">
    <div style="text-align:center;color:white;font-weight:700;margin-bottom:10px;font-size:18px">Self-Destruct Text</div>
    <div class="timer-opt off" onclick="setSdTimer(0)">❌ Turn Off</div>
    <div class="timer-opt" onclick="setSdTimer(30)">⏱️ 30 Seconds</div>
    <div class="timer-opt" onclick="setSdTimer(60)">⏱️ 1 Minute</div>
    <div class="timer-opt" onclick="setSdTimer(300)">⏱️ 5 Minutes</div>
  </div>
</div>

<!-- Bomb Viewer -->
<div id="bombView">
  <div style="font-size:24px;font-weight:800;color:var(--danger);margin-bottom:20px;">⚠️ VIEW ONCE</div>
  <img id="bombImg" src="">
  <div id="bombHold" class="hold-circle" ontouchstart="revealBomb()" ontouchend="burnBomb()" onmousedown="revealBomb()" onmouseup="burnBomb()">💣</div>
  <div class="hold-hint">Hold to Reveal · Release to Burn</div>
</div>

<!-- Context Menu -->
<div id="ctxMenu">
  <div class="ctx-item" onclick="pinMsg()">📌 Pin Message</div>
  <div class="ctx-item" onclick="copyMsg()">📋 Copy Text</div>
  <div class="ctx-item" onclick="sdReply()">💀 Reply with Bomb</div>
</div>

<script>
// ─── STATE ───────────────────────────────────────────────────
let myName='', myKey='', myTok='', roomHash='';
let sdTimer=0, uploading=false;
let delFiles=[], selTxt='', svFile='';
let lastType=0, lastHash=0, errs=0, msgCount=0;
const MG='GHO:';

// ─── UTILS ───────────────────────────────────────────────────
const gt=()=>{let d=new Date();return d.getHours().toString().padStart(2,'0')+':'+d.getMinutes().toString().padStart(2,'0')};
function avc(n){let h=0;for(let i=0;i<n.length;i++)h=((h<<5)-h)+n.charCodeAt(i);let c=(h&0xFFFFFF).toString(16).padStart(6,'0');return '#'+c;}
function ini(n){return(n||'??').substring(0,2).toUpperCase();}
function enc(t,k){let r='';for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return btoa(r);}
function dec(e,k){try{let t=atob(e),r='';for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return r;}catch{return'???';}}
function hsh(s){let h=0;for(let i=0;i<s.length;i++)h=((h<<5)-h)+s.charCodeAt(i);return Math.abs(h).toString(16).substring(0,6);}
function fastHash(s){let h=0;for(let i=0;i<s.length;i++){h=((h<<5)-h)+s.charCodeAt(i);h|=0;}return h;}
function fmtSD(s){if(s<60)return s+'s';return(s/60)+'m';}
function sdRemain(ep,dur){let rem=dur-Math.floor((Date.now()-ep)/1000);return rem>0?rem:0;}

// Pure JS Hash (Fixes the captive portal bug)
function genHash(pwd){
  let s=pwd+"ghost_salt_v5", h1=0xdeadbeef, h2=0x41c6ce57;
  for(let i=0;i<s.length;i++){
    let ch=s.charCodeAt(i);
    h1=Math.imul(h1^ch,2654435761); h2=Math.imul(h2^ch,1597334677);
  }
  h1=Math.imul(h1^(h1>>>16),2246822507)^Math.imul(h2^(h2>>>13),3266489909);
  h2=Math.imul(h2^(h2>>>16),2246822507)^Math.imul(h1^(h1>>>13),3266489909);
  return ((h1>>>0).toString(16).padStart(8,'0')+(h2>>>0).toString(16).padStart(8,'0')).substring(0,8);
}

function toast(msg, type='ok'){
  let c = document.getElementById('toasts');
  let t = document.createElement('div');
  t.className = 'toast '+type;
  t.innerHTML = (type==='ok'?'✅':'❌') + ' ' + msg;
  c.appendChild(t);
  setTimeout(()=>{ t.classList.add('out'); setTimeout(()=>t.remove(), 300); }, 3000);
}

// ─── LOGIN ───────────────────────────────────────────────────
function doLogin(){
  myName=document.getElementById('ni').value.trim().substring(0,20);
  myKey=document.getElementById('ki').value;
  if(!myName||!myKey){toast('Enter Alias and Password','err');return;}
  
  roomHash=genHash(myKey);
  toast('Verifying keys...', 'ok');
  
  fetch('/login?room='+roomHash+'&pass='+encodeURIComponent(myKey)+'&name='+encodeURIComponent(myName))
    .then(r=>r.text()).then(resp=>{
      if(resp.startsWith('WIPED')){alert('Wiped.');location.reload();}
      else if(resp.startsWith('TOK:')){
        myTok=resp.substring(4);
        document.getElementById('lm').style.display='none';
        document.getElementById('roomHVal').innerText = roomHash.substring(0,4).toUpperCase();
        document.getElementById('ra').innerText = myName.substring(0,1).toUpperCase();
        
        let jm=MG+'|SYS|'+gt()+'|👤 '+myName+' securely joined';
        fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(jm,myKey)));
        startPoll();setInterval(chkPin,4000);setInterval(chkLora,5000);
      }
      else{toast('Login Failed','err');}
    }).catch(()=>toast('Router unreachable','err'));
}
function doLogout(){
  try {
    fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(MG+'|SYS|'+gt()+'|👤 '+myName+' left',myKey)))
      .catch(()=>{})
      .finally(()=>{ window.location.href = "/"; });
  } catch(e) {
    window.location.href = "/";
  }
}

// ─── UI INTERACTIONS ─────────────────────────────────────────
function toggleSheet(){
  let fab=document.getElementById('fabBtn');
  let isAct = fab.classList.contains('active');
  if(!isAct){
    fab.classList.add('active');
    document.getElementById('overlay').classList.add('show');
    document.getElementById('actionSheet').classList.add('show');
    document.getElementById('sheetOpts').style.display='flex';
    document.getElementById('timerList').style.display='none';
  } else {
    closeSheet();
  }
}
function closeSheet(){
  document.getElementById('fabBtn').classList.remove('active');
  document.getElementById('overlay').classList.remove('show');
  document.getElementById('actionSheet').classList.remove('show');
}
function showTimers(){
  document.getElementById('sheetOpts').style.display='none';
  document.getElementById('timerList').style.display='flex';
}
function setSdTimer(sec){
  sdTimer=sec;
  closeSheet();
  let b=document.getElementById('sdbadge');
  if(sec>0){b.innerText='💀 '+fmtSD(sec); b.style.display='block';}
  else{b.style.display='none';}
}

// ─── UPLOADS ─────────────────────────────────────────────────
function handleUpload(isVO){
  let iid=isVO?'vi':'fi';
  let el=document.getElementById(iid);
  let file=el.files[0];
  if(!file)return;
  uploading=true;
  
  let mi=document.getElementById('mi'); 
  mi.placeholder='Compressing image...'; 
  mi.disabled=true;
  
  // Create File Reader for Captive Portal safety rather than createObjectURL
  let reader = new FileReader();
  reader.onload = function(e){
    let img=new Image();
    img.onload=function(){
      let cv=document.createElement('canvas');
      let cx=cv.getContext('2d');
      let MAX=400, w=img.width, h=img.height;
      if(w>h){if(w>MAX){h=h*MAX/w;w=MAX;}}else{if(h>MAX){w=w*MAX/h;h=MAX;}}
      cv.width=w; cv.height=h;
      cx.drawImage(img,0,0,w,h);
      
      cv.toBlob(blob=>{
        if(!blob){toast('Compression failed','err'); uploadEnd(iid,mi); return;}
        let fd=new FormData(), fn='i'+Date.now()+'.jpg';
        fd.append('file',blob,fn);
        mi.placeholder='Uploading payload...';
        
        fetch('/upload?tok='+myTok, {method:'POST', body:fd}).then(r=>{
           if(r.ok){
             let px = isVO?'VO:':'IMG:';
             let raw = MG+'|'+myName+'|'+gt()+'|'+px+fn;
             fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(raw,myKey))).then(()=>poll());
             toast('Delivered securely', 'ok');
           }else{toast('Server rejected file','err');}
           uploadEnd(iid,mi);
        }).catch(()=>{toast('Upload network error','err'); uploadEnd(iid,mi);});
      },'image/jpeg', 0.5);
    };
    img.src=e.target.result;
  };
  reader.readAsDataURL(file);
}
function uploadEnd(iid,mi){
  uploading=false;
  document.getElementById(iid).value='';
  mi.disabled=false;
  mi.placeholder='Message...';
}

function delImg(fn){
  if(!confirm('Delete globally?'))return;
  fetch('/delete_file?name='+fn+'&tok='+myTok);
  fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(MG+'|SYS|00:00|CMD:DEL:'+fn,myKey))).then(()=>poll());
}

// ─── BOMB VIEWING ────────────────────────────────────────────
function openVO(fn){
  svFile=fn;
  document.getElementById('bombImg').src='/download?name='+fn;
  document.getElementById('bombView').style.display='flex';
}
function revealBomb(){
  document.getElementById('bombImg').style.display='block';
  document.getElementById('bombHold').style.display='none';
}
function burnBomb(){
  document.getElementById('bombView').style.display='none';
  document.getElementById('bombImg').style.display='none';
  document.getElementById('bombHold').style.display='flex';
  
  if(svFile){
    fetch('/delete_file?name='+svFile+'&tok='+myTok);
    fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(MG+'|SYS|00:00|CMD:DEL:'+svFile,myKey)));
    svFile='';
    toast('Image burned globally', 'ok');
  }
}

// ─── MESSAGING ───────────────────────────────────────────────
function sendMsg(){
  let m=document.getElementById('mi').value.trim();
  if(!m)return;
  
  let epoch=sdTimer>0?Date.now():0;
  let content=sdTimer>0?'SD:'+sdTimer+':'+epoch+':'+m:m;
  let raw=MG+'|'+myName+'|'+gt()+'|'+content;
  fetch('/send?tok='+myTok+'&msg='+encodeURIComponent(enc(raw,myKey)))
    .then(()=>{document.getElementById('mi').value='';setSdTimer(0);poll();});
}
function onType(){if(Date.now()-lastType>2000){lastType=Date.now();fetch('/typing?name='+encodeURIComponent(myName)+'&tok='+myTok);}}

// ─── CONTEXT & REPLIES ───────────────────────────────────────
function showCtx(txt,e){
  selTxt=txt;
  let m=document.getElementById('ctxMenu');
  m.style.display='block';
  m.style.left=Math.max(10, Math.min(e.clientX, window.innerWidth-190))+'px';
  m.style.top=Math.max(10, Math.min(e.clientY, window.innerHeight-150))+'px';
  e.preventDefault();
}
document.addEventListener('click',e=>{
  if(!e.target.closest('#ctxMenu')) document.getElementById('ctxMenu').style.display='none';
});
function pinMsg(){fetch('/pin?tok='+myTok+'&msg='+encodeURIComponent(enc(selTxt,myKey)));}
function copyMsg(){navigator.clipboard?.writeText(selTxt).then(()=>toast('Copied','ok'));}
function sdReply(){setSdTimer(60);let i=document.getElementById('mi');i.value='re: '+selTxt.substring(0,25)+'... ';i.focus();}

// ─── POLLING & RENDER ────────────────────────────────────────
function clrChat(){if(confirm('Wipe room?')){fetch('/clear?tok='+myTok).then(()=>{document.getElementById('chatbox').innerHTML='';lastHash=0;});}}
function showStat(){fetch('/status').then(r=>r.text()).then(s=>{toast(s,'ok');});}

function chkPin(){
  fetch('/getpin?tok='+myTok).then(r=>r.text()).then(ep=>{
    let bar=document.getElementById('pinBar');
    if(ep==='NONE'||!ep)bar.style.display='none';
    else{let t=dec(ep,myKey);if(t&&t!=='???'){bar.style.display='flex';document.getElementById('pc').innerText=t;}}
  });
}
function unpin(){fetch('/pin?tok='+myTok+'&msg=CLEAR');document.getElementById('pinBar').style.display='none';}
function chkLora(){
  fetch('/lorastatus').then(r=>r.text()).then(s=>{
    let pts=s.split(':'), ok=(pts[0]==='OK');
    document.getElementById('lorabar').style.display=ok?'flex':'none';
    if(ok)document.getElementById('loratxt').innerText=pts[1];
  });
}

function poll(){
  if(uploading)return;
  fetch('/read?tok='+myTok).then(r=>r.text()).then(raw=>{
    errs=0;document.getElementById('cdot').style.background='var(--success)';
    document.getElementById('cdot').style.boxShadow='0 0 8px var(--success)';

    let olp=raw.split('###ONLINE:'), onlineN=olp.length>1?olp[1]:'0';
    let pts=olp[0].split('###TYPE:'), md=pts[0], td=pts.length>1?pts[1]:'';

    let tb=document.getElementById('tn');
    if(td.includes('|')){let tp=td.split('|');if(tp[0]!==myName)tb.innerText=tp[0]+' is typing...';else tb.innerText='';}
    else tb.innerText='';

    let msgs=md.split('|||').filter(m=>m.length>0);
    msgs.forEach(m=>{
      let c=dec(m,myKey); if(!c.startsWith(MG))return;
      let p=c.split('|'); if(p.length<4)return;
      let cnt=p.slice(3).join('|');
      if(cnt.startsWith('CMD:DEL:')){let f=cnt.substring(8);if(!delFiles.includes(f))delFiles.push(f);}
    });

    let html='';
    let rc=0;
    msgs.forEach(m=>{
      let c=m.startsWith('GHO:|LORA')?m:dec(m,myKey);
      if(!c.startsWith(MG))return;
      let p=c.split('|');if(p.length<4)return;
      let snd=p[1],tim=p[2],cnt=p.slice(3).join('|');
      if(cnt.startsWith('CMD:'))return;
      rc++;
      if(snd==='SYS'){html+='<div class="sys-msg"><span>'+cnt+'</span></div>';return;}

      let isSent=snd===myName;
      let sdActive=false,sdRem=0,actual=cnt;
      if(cnt.startsWith('SD:')){
        let sp=cnt.split(':');
        if(sp.length>=4){sdActive=true;actual=sp.slice(3).join(':');sdRem=sdRemain(parseInt(sp[2]),parseInt(sp[1]));}
      }
      if(sdActive&&sdRem<=0){html+='<div class="sys-msg"><span>💀 Text Burned</span></div>';return;}

      let bc='';
      if(actual.startsWith('IMG:')){
        let f=actual.substring(4);
        bc=delFiles.includes(f)?'<span style="opacity:0.5;font-style:italic">🗑️ Deleted</span>':'<div class="img-wrap"><img src="/download?name='+f+'" class="chat-img"><button class="del-btn" onclick="delImg(\''+f+'\')">✕</button></div>';
      }else if(actual.startsWith('VO:')){
        let f=actual.substring(3);
        bc=delFiles.includes(f)?'<div class="bomb-card" style="opacity:0.5"><span class="bomb-icon">💥</span><span class="bomb-text">Burned</span></div>':'<div class="bomb-card" onclick="openVO(\''+f+'\')"><span class="bomb-icon">💣</span><span class="bomb-text">Bomb Image</span></div>';
      }else bc=actual;

      let av='<div class="avatar" style="background:'+avc(snd)+'">'+ini(snd)+'</div>';
      let nm=!isSent?'<span class="sender-name" style="color:'+avc(snd)+'">'+snd+'</span>':'';
      let sdb=sdActive?'<span class="sd-badge">💀 '+sdRem+'s</span> ':'';
      let ctx=(!actual.startsWith('IMG:')&&!actual.startsWith('VO:'))?'data-ctx="'+btoa(unescape(encodeURIComponent(actual)))+'" oncontextmenu="showCtx(decodeURIComponent(escape(atob(this.dataset.ctx))),event);return false;"':'';

      html+='<div class="message-row '+(isSent?'sent':'recv')+'">'+av+'<div class="bubble-wrap"><div class="bubble" '+ctx+'>'+nm+bc+'<div class="message-meta">'+sdb+tim+(isSent?' <span style="color:var(--accent)">✓</span>':'')+'</div></div></div></div>';
    });

    let nh=fastHash(html);
    let bx=document.getElementById('chatbox');
    if(nh!==lastHash){
      lastHash=nh;
      let atBot=bx.scrollTop+bx.clientHeight>=bx.scrollHeight-50;
      bx.innerHTML=html;
      if(atBot)bx.scrollTop=bx.scrollHeight;
    }
    document.getElementById('hs').innerText=onlineN+' connected';
  }).catch(()=>{
    errs++;
    if(errs>3){
      document.getElementById('cdot').style.background='var(--danger)';
      document.getElementById('cdot').style.boxShadow='0 0 8px var(--danger)';
    }
  });
}
function startPoll(){poll();setInterval(poll,1500);}
</script>
</body>
</html>

)rawliteral";

#endif
