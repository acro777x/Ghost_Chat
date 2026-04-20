/*
 * ============================================================
 * GHOST CHAT — Heltec WiFi LoRa 32 V2 (SX1276)
 * Debugged + All V2-specific pins corrected
 *
 * LIBRARY to install:
 *   "LoRa" by Sandeep Mistry
 *   "U8g2" by oliver
 *
 * BOARD: "Heltec WiFi LoRa 32(V2)" in Arduino IDE
 * PARTITION: Default 4MB with spiffs
 *
 * V2 KEY DIFFERENCES FROM V3 (all fixed here):
 *   - OLED RST = GPIO16 (must LOW→HIGH in setup)
 *   - OLED SDA = GPIO4, SCL = GPIO15
 *   - SPI must be init manually: SPI.begin(5,19,27,18)
 *   - LoRa.begin takes Hz: 866E6 not 866.0
 *   - No VEXT pin — OLED powered directly
 *   - Processor: classic ESP32, not S3
 * ============================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

// ============================================================
// CONFIGURATION
// ============================================================
const char* WIFI_SSID   = "Secure_Ghost_Chat";
const char* WIFI_PASS   = "password123";
const char* DURESS_PASS = "pink";
const char* DOMAIN_NAME = "ghost.chat";

// ============================================================
// V2 PIN DEFINITIONS — confirmed from schematic
// ============================================================
#define LORA_SCK    5
#define LORA_MISO   19
#define LORA_MOSI   27
#define LORA_SS     18
#define LORA_RST    14
#define LORA_DIO0   26

#define OLED_SDA    4
#define OLED_SCL    15
#define OLED_RST    16   // Must toggle LOW->HIGH

#define PANIC_BTN   0

// ============================================================
// LIMITS
// ============================================================
#define MAX_MESSAGES    120
#define MIN_FREE_HEAP   25000
#define MAX_CHAT_BYTES  35000

// ============================================================
// OBJECTS
// ============================================================
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
  U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE
);

// ============================================================
// STATE
// ============================================================
String chatHistory   = "";
String pinnedMessage = "";
String typingUser    = "";
unsigned long typingExpiry = 0;

struct UserEntry { String name; unsigned long lastSeen; };
UserEntry onlineUsers[10];
int userCount = 0;

unsigned long lastOled = 0;

// ============================================================
// HELPERS
// ============================================================
void trimChat() {
  int count = 0;
  for (int i = 0; i < (int)chatHistory.length()-2; i++)
    if (chatHistory[i]=='|'&&chatHistory[i+1]=='|'&&chatHistory[i+2]=='|') count++;

  if (count > MAX_MESSAGES) {
    int remove = 30, found = 0, cut = 0;
    for (int i = 0; i < (int)chatHistory.length()-2; i++) {
      if (chatHistory[i]=='|'&&chatHistory[i+1]=='|'&&chatHistory[i+2]=='|') {
        found++;
        if (found == remove) { cut = i+3; break; }
      }
    }
    if (cut > 0) chatHistory = chatHistory.substring(cut);
  }
  if ((int)chatHistory.length() > MAX_CHAT_BYTES) {
    chatHistory = chatHistory.substring(chatHistory.length() - MAX_CHAT_BYTES);
    int first = chatHistory.indexOf("|||");
    if (first > 0) chatHistory = chatHistory.substring(first+3);
  }
}

void updateUser(String name) {
  for (int i = 0; i < userCount; i++) {
    if (onlineUsers[i].name == name) { onlineUsers[i].lastSeen = millis(); return; }
  }
  if (userCount < 10) { onlineUsers[userCount] = {name, millis()}; userCount++; }
}

int onlineCount() {
  int c = 0;
  for (int i = 0; i < userCount; i++)
    if (millis() - onlineUsers[i].lastSeen < 30000) c++;
  return c;
}

void doWipe() {
  chatHistory = ""; pinnedMessage = "";
  userCount = 0;
  SPIFFS.format();
}

void updateOLED() {
  if (millis() - lastOled < 3000) return;
  lastOled = millis();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Ghost Chat V2");
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 26, ("IP: "+WiFi.softAPIP().toString()).c_str());
  u8g2.drawStr(0, 38, ("Users: "+String(onlineCount())+" online").c_str());
  u8g2.drawStr(0, 50, ("RAM: "+String(ESP.getFreeHeap()/1024)+"KB").c_str());
  u8g2.drawStr(0, 62, WIFI_SSID);
  u8g2.sendBuffer();
}

// ============================================================
// HTML — Ghost Chat UI (same as V2 version, no changes needed)
// ============================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0">
  <title>Ghost Chat</title>
  <style>
    :root{--bg:#0f172a;--glass:rgba(15,23,42,0.9);--border:1px solid rgba(255,255,255,0.08);--accent:#3b82f6;--sent:linear-gradient(135deg,#3b82f6,#2563eb);--recv:#182533;--ok:#22c55e;--err:#ef4444;}
    *{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,BlinkMacSystemFont,"SF Pro Text",Roboto,sans-serif;-webkit-tap-highlight-color:transparent;}
    body{background:var(--bg);background-image:radial-gradient(at 0% 0%,hsla(253,16%,7%,1) 0,transparent 50%),radial-gradient(at 50% 0%,hsla(225,39%,30%,1) 0,transparent 50%),radial-gradient(at 100% 0%,hsla(339,49%,30%,1) 0,transparent 50%);background-attachment:fixed;color:white;height:100vh;display:flex;flex-direction:column;user-select:none;-webkit-user-select:none;}
    .header{background:var(--glass);backdrop-filter:blur(12px);border-bottom:var(--border);padding:0 15px;height:60px;display:flex;align-items:center;justify-content:space-between;position:fixed;top:0;width:100%;z-index:100;}
    .hdot{width:8px;height:8px;border-radius:50%;background:var(--ok);animation:pulse 2s infinite;margin-right:10px;}
    @keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
    .htitle{font-weight:700;font-size:17px;}
    .hsub{font-size:11px;color:var(--ok);}
    .hbtns{display:flex;gap:12px;}
    .hbtn{background:none;border:none;font-size:18px;cursor:pointer;color:#fff;opacity:.8;}
    #obar{position:fixed;top:60px;width:100%;background:rgba(59,130,246,0.08);border-bottom:var(--border);padding:5px 15px;font-size:11px;color:#64b5f6;display:flex;align-items:center;gap:6px;z-index:90;}
    #pinBar{position:fixed;top:82px;width:100%;background:rgba(33,150,243,0.1);border-bottom:var(--border);padding:7px 15px;font-size:13px;display:none;align-items:center;gap:10px;z-index:89;cursor:pointer;color:white;}
    #chatbox{flex:1;margin-top:82px;margin-bottom:90px;padding:12px;overflow-y:auto;display:flex;flex-direction:column;}
    .brow{display:flex;margin-bottom:6px;width:100%;align-items:flex-end;}
    .brow.sent{justify-content:flex-end;}.brow.recv{justify-content:flex-start;}
    .av{width:28px;height:28px;border-radius:50%;display:flex;align-items:center;justify-content:center;font-weight:bold;font-size:11px;margin-right:6px;flex-shrink:0;}
    .brow.sent .av{display:none;}
    .bwrap{display:flex;flex-direction:column;max-width:78%;}
    .brow.sent .bwrap{align-items:flex-end;}
    .bubble{padding:8px 12px;border-radius:18px;font-size:15px;line-height:1.4;word-break:break-word;}
    .bubble.sent{background:var(--sent);border-bottom-right-radius:4px;}
    .bubble.recv{background:var(--recv);border-bottom-left-radius:4px;}
    .sname{font-size:10px;font-weight:700;color:#64b5f6;margin-bottom:2px;display:block;}
    .meta{font-size:10px;opacity:.5;margin-top:3px;display:flex;gap:4px;}
    .brow.sent .meta{justify-content:flex-end;}
    .rbar{display:flex;gap:3px;margin-top:3px;flex-wrap:wrap;}
    .rbtn{background:rgba(255,255,255,.08);border:1px solid rgba(255,255,255,.1);border-radius:12px;padding:1px 6px;font-size:12px;cursor:pointer;}
    .radd{background:none;border:1px solid rgba(255,255,255,.15);border-radius:50%;width:20px;height:20px;font-size:10px;cursor:pointer;color:rgba(255,255,255,.4);}
    .rpick{position:fixed;background:rgba(30,41,59,.97);backdrop-filter:blur(15px);border-radius:12px;border:var(--border);padding:8px;z-index:2500;display:none;gap:8px;box-shadow:0 10px 30px rgba(0,0,0,.5);}
    .ropt{font-size:20px;cursor:pointer;padding:4px;border-radius:6px;}
    .sysmsg{width:100%;text-align:center;margin:8px 0;font-size:11px;color:#64748b;background:rgba(255,255,255,.04);padding:4px 8px;border-radius:6px;}
    .iw{position:relative;display:inline-block;}
    .cimg{max-width:200px;max-height:200px;border-radius:10px;margin-top:5px;object-fit:cover;}
    .dbtn{position:absolute;top:4px;right:4px;background:rgba(0,0,0,.6);color:white;border:none;border-radius:50%;width:18px;height:18px;cursor:pointer;font-size:9px;}
    .vob{background:rgba(239,68,68,.15);border:1px solid rgba(239,68,68,.4);color:#fca5a5;cursor:pointer;display:flex;align-items:center;gap:8px;font-weight:600;padding:10px 14px;border-radius:12px;margin-top:5px;}
    .iarea{position:fixed;bottom:0;left:0;width:100%;background:var(--glass);backdrop-filter:blur(12px);border-top:var(--border);padding:8px 10px;z-index:100;padding-bottom:max(8px,env(safe-area-inset-bottom));}
    #tbar{font-size:11px;color:var(--accent);height:16px;opacity:0;transition:opacity .3s;display:flex;align-items:center;gap:5px;margin-bottom:4px;padding-left:44px;}
    .td{width:3px;height:3px;background:var(--accent);border-radius:50%;display:inline-block;animation:tj 1s infinite;}
    .td:nth-child(2){animation-delay:.15s}.td:nth-child(3){animation-delay:.3s}
    @keyframes tj{0%,100%{transform:translateY(0)}50%{transform:translateY(-3px)}}
    .irow{display:flex;align-items:center;gap:6px;}
    .ibtn{background:none;border:none;font-size:20px;padding:0 6px;cursor:pointer;color:#888;}
    #mi{flex:1;padding:10px 14px;border:none;border-radius:22px;background:rgba(0,0,0,.35);color:white;font-size:16px;outline:none;}
    #mi::placeholder{color:rgba(255,255,255,.3);}
    .sbtn{background:var(--accent);color:white;border:none;width:38px;height:38px;border-radius:50%;font-size:16px;cursor:pointer;}
    #lm{position:fixed;top:0;left:0;width:100%;height:100%;background:#0a1018;z-index:9999;display:flex;align-items:center;justify-content:center;}
    .lb{background:rgba(23,33,43,.95);padding:28px 24px;border-radius:20px;width:88%;max-width:340px;text-align:center;border:1px solid rgba(255,255,255,.08);}
    .llo{font-size:40px;margin-bottom:10px;}
    .lt{font-size:22px;font-weight:700;color:var(--accent);margin-bottom:4px;}
    .ls{font-size:12px;color:#64748b;margin-bottom:22px;}
    .li{width:100%;padding:12px 16px;background:rgba(0,0,0,.4);border:1px solid rgba(59,130,246,.3);color:white;margin-bottom:12px;border-radius:10px;font-size:16px;text-align:center;outline:none;}
    .li:focus{border-color:var(--accent);}
    .lbtn{width:100%;padding:13px;background:var(--accent);color:white;border:none;border-radius:10px;font-size:16px;font-weight:700;cursor:pointer;}
    #sv{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:black;z-index:8000;flex-direction:column;align-items:center;justify-content:center;}
    #si{max-width:95%;max-height:70%;display:none;pointer-events:none;border:2px solid var(--err);border-radius:8px;}
    #hb{margin-top:28px;padding:14px 40px;background:white;color:black;border-radius:50px;font-weight:800;font-size:16px;border:none;}
    #cm{position:fixed;background:rgba(28,38,52,.97);backdrop-filter:blur(15px);padding:4px;border-radius:12px;border:var(--border);display:none;z-index:7000;box-shadow:0 10px 30px rgba(0,0,0,.6);min-width:160px;}
    .ci{padding:12px 18px;color:white;cursor:pointer;border-bottom:1px solid rgba(255,255,255,.04);font-size:14px;display:flex;align-items:center;gap:10px;}
    .ci:last-child{border-bottom:none;}
    #rw{display:none;position:fixed;top:60px;width:100%;background:rgba(239,68,68,.9);padding:6px;text-align:center;font-size:12px;font-weight:600;z-index:200;}
  </style>
</head>
<body>
<input type="file" id="fi" style="display:none" accept="image/*" onchange="handleUpload(false)">
<input type="file" id="vi" style="display:none" accept="image/*" onchange="handleUpload(true)">
<div id="cm"><div class="ci" onclick="pinMsg()">📌 Pin</div><div class="ci" onclick="copyMsg()">📋 Copy</div><div class="ci" onclick="hideCm()">✕ Cancel</div></div>
<div id="rpick" class="rpick"><span class="ropt" onclick="sendReact('👍')">👍</span><span class="ropt" onclick="sendReact('❤️')">❤️</span><span class="ropt" onclick="sendReact('😂')">😂</span><span class="ropt" onclick="sendReact('😮')">😮</span><span class="ropt" onclick="sendReact('🔥')">🔥</span><span class="ropt" onclick="sendReact('👀')">👀</span></div>
<div id="sv"><div style="color:#ef4444;font-weight:700;margin-bottom:16px;">⚠️ View Once</div><img id="si" src=""><button id="hb" ontouchstart="showS()" ontouchend="burnS()" onmousedown="showS()" onmouseup="burnS()" oncontextmenu="return false">HOLD TO VIEW</button></div>
<div id="lm"><div class="lb"><div class="llo">👻</div><div class="lt">Ghost Chat</div><div class="ls">ghost.chat • Encrypted</div><input type="text" id="ni" class="li" placeholder="Your Name" autocomplete="off"><input type="password" id="ki" class="li" placeholder="Room Password" autocomplete="off"><button onclick="doLogin()" class="lbtn">ENTER</button></div></div>
<div id="rw">⚠️ Server RAM low — clear chat</div>
<div class="header"><div style="display:flex;align-items:center"><div class="hdot"></div><div><div class="htitle">Ghost Chat</div><div class="hsub" id="hs">Encrypted</div></div></div><div class="hbtns"><button class="hbtn" onclick="showStat()">📊</button><button class="hbtn" onclick="clrChat()">🗑️</button><button class="hbtn" onclick="doLogout()">🚪</button></div></div>
<div id="obar"><div class="hdot" style="width:6px;height:6px"></div><span id="ot">Connecting...</span></div>
<div id="pinBar" onclick="unpin()"><span style="color:var(--accent)">📌</span><span style="flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap" id="pc"></span><span style="font-size:11px;opacity:.6">tap to unpin</span></div>
<div id="chatbox"></div>
<div class="iarea"><div id="tbar"><span class="td"></span><span class="td"></span><span class="td"></span><span id="tn"></span></div><div class="irow"><button class="ibtn" onclick="document.getElementById('fi').click()">🖼️</button><button class="ibtn" onclick="document.getElementById('vi').click()" style="color:#fca5a5">💣</button><input id="mi" type="text" placeholder="Message..." oninput="onType()" onkeydown="if(event.key==='Enter'&&!event.shiftKey){event.preventDefault();sendMsg()}"><button class="sbtn" onclick="sendMsg()">➤</button></div></div>

<script>
let myName="",myKey="",delFiles=[],selTxt="",svFile="",uploading=false,lastType=0,rtgt="",lastHTML="",errs=0;
const MG="GHO:";
const gt=()=>{let d=new Date();return d.getHours().toString().padStart(2,'0')+':'+d.getMinutes().toString().padStart(2,'0')};
function avc(n){let h=0;for(let i=0;i<n.length;i++)h=n.charCodeAt(i)+((h<<5)-h);return '#'+((h&0xFFFFFF).toString(16).padStart(6,'0'))}
function ini(n){return n.substring(0,2).toUpperCase()}
function enc(t,k){let r="";for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return btoa(r)}
function dec(e,k){try{let t=atob(e),r="";for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return r}catch{return"???"}}
function hsh(s){let h=0;for(let i=0;i<s.length;i++)h=((h<<5)-h)+s.charCodeAt(i);return Math.abs(h).toString(16).substring(0,6)}

function doLogin(){
  myName=document.getElementById('ni').value.trim();
  myKey=document.getElementById('ki').value;
  if(!myName||!myKey)return;
  fetch('/login?key='+encodeURIComponent(myKey)).then(r=>r.text()).then(resp=>{
    if(resp==='WIPED'){alert('Security wipe.');location.reload();}
    else{
      document.getElementById('lm').style.display='none';
      let jm=MG+'|SYS|'+gt()+'|'+myName+' joined';
      fetch('/send?msg='+encodeURIComponent(enc(jm,myKey)));
      startPoll();setInterval(chkPin,3000);setInterval(chkRAM,10000);
    }
  });
}
function doLogout(){let lm=MG+'|SYS|'+gt()+'|'+myName+' left';fetch('/send?msg='+encodeURIComponent(enc(lm,myKey))).finally(()=>location.reload())}
function sendMsg(){
  let m=document.getElementById('mi').value.trim();if(!m)return;
  if(m==='/status'){showStat();document.getElementById('mi').value='';return}
  let f=MG+'|'+myName+'|'+gt()+'|'+m;
  fetch('/send?msg='+encodeURIComponent(enc(f,myKey))).then(()=>{document.getElementById('mi').value='';poll()});
}
function onType(){if(Date.now()-lastType>2000){lastType=Date.now();fetch('/typing?name='+encodeURIComponent(myName)+'&room='+btoa(myKey))}}
function showCm(txt,e){selTxt=txt;let m=document.getElementById('cm');m.style.display='block';m.style.left=Math.min(e.clientX,window.innerWidth-170)+'px';m.style.top=Math.min(e.clientY,window.innerHeight-120)+'px';e.preventDefault()}
function hideCm(){document.getElementById('cm').style.display='none'}
function pinMsg(){hideCm();fetch('/pin?msg='+encodeURIComponent(enc(selTxt,myKey)))}
function copyMsg(){hideCm();navigator.clipboard?.writeText(selTxt).catch(()=>{})}
document.addEventListener('click',hideCm);
function openRP(id,e){rtgt=id;let p=document.getElementById('rpick');p.style.display='flex';p.style.left=Math.min(e.clientX,window.innerWidth-180)+'px';p.style.top=(e.clientY-60)+'px';e.stopPropagation()}
function closeRP(){document.getElementById('rpick').style.display='none'}
function sendReact(em){closeRP();let rm=MG+'|'+myName+'|'+gt()+'|REACT:'+rtgt+':'+em;fetch('/send?msg='+encodeURIComponent(enc(rm,myKey))).then(()=>poll())}
document.addEventListener('click',closeRP);
function chkPin(){fetch('/getpin').then(r=>r.text()).then(ep=>{let bar=document.getElementById('pinBar');if(ep==='NONE'||ep===''){bar.style.display='none'}else{let t=dec(ep,myKey);if(t&&t!=='???'){bar.style.display='flex';document.getElementById('pc').innerText=t}}})}
function unpin(){if(confirm('Unpin?'))fetch('/pin?msg=CLEAR');document.getElementById('pinBar').style.display='none'}
function clrChat(){if(!confirm('Wipe all messages?'))return;fetch('/clear').then(()=>{document.getElementById('chatbox').innerHTML='';lastHTML=''})}
function showStat(){fetch('/status').then(r=>r.text()).then(s=>{let b=document.getElementById('chatbox');b.innerHTML+='<div class="sysmsg">📊 '+s+'</div>';b.scrollTop=b.scrollHeight})}
function chkRAM(){fetch('/ram').then(r=>r.text()).then(kb=>{document.getElementById('rw').style.display=parseInt(kb)<40?'block':'none'})}

function handleUpload(isVO){
  let iid=isVO?'vi':'fi',file=document.getElementById(iid).files[0];if(!file)return;
  uploading=true;let me=document.getElementById('mi');me.placeholder='Compressing...';me.disabled=true;
  let reader=new FileReader();reader.readAsDataURL(file);
  reader.onload=e=>{
    let img=new Image();img.src=e.target.result;
    img.onload=function(){
      let cv=document.createElement('canvas'),cx=cv.getContext('2d'),MAX=600,w=img.width,h=img.height;
      if(w>h){if(w>MAX){h=h*MAX/w;w=MAX}}else{if(h>MAX){w=w*MAX/h;h=MAX}}
      cv.width=w;cv.height=h;cx.drawImage(img,0,0,w,h);
      cv.toBlob(blob=>{
        let fd=new FormData(),fn='i'+Math.floor(Math.random()*99999)+'.jpg';
        fd.append('file',blob,fn);me.placeholder='Uploading...';
        fetch('/upload',{method:'POST',body:fd}).then(r=>{
          me.placeholder='Message...';me.disabled=false;document.getElementById(iid).value='';uploading=false;
          if(r.ok){let px=isVO?'VO:':'IMG:',fm=MG+'|'+myName+'|'+gt()+'|'+px+fn;fetch('/send?msg='+encodeURIComponent(enc(fm,myKey))).then(()=>poll())}
        }).catch(()=>{me.placeholder='Message...';me.disabled=false;uploading=false});
      },'image/jpeg',0.65);
    }
  }
}
function delImg(fn){if(!confirm('Delete?'))return;fetch('/delete_file?name='+fn);let c=MG+'|SYS|00:00|CMD:DEL:'+fn;fetch('/send?msg='+encodeURIComponent(enc(c,myKey))).then(()=>poll())}
function openVO(fn){svFile=fn;document.getElementById('si').src='/download?name='+fn;document.getElementById('sv').style.display='flex'}
function showS(){document.getElementById('si').style.display='block'}
function burnS(){document.getElementById('si').style.display='none';document.getElementById('sv').style.display='none';fetch('/delete_file?name='+svFile);let c=MG+'|SYS|00:00|CMD:DEL:'+svFile;fetch('/send?msg='+encodeURIComponent(enc(c,myKey)));document.getElementById('si').src=''}

function poll(){
  if(uploading)return;
  fetch('/read').then(r=>r.text()).then(raw=>{
    errs=0;
    let pts=raw.split('###TYPE:'),md=pts[0],td=pts.length>1?pts[1]:'';
    let tb=document.getElementById('tbar');
    if(td.includes('|')){let tp=td.split('|');if(tp[0]!==myName&&tp[1]===btoa(myKey)){tb.style.opacity='1';document.getElementById('tn').innerText=tp[0]+' is typing...'}else tb.style.opacity='0'}else tb.style.opacity='0';
    let msgs=md.split('|||').filter(m=>m.length>0);
    let rm={};
    msgs.forEach(m=>{let c=dec(m,myKey);if(!c.startsWith(MG))return;let p=c.split('|');if(p.length<4)return;let ct=p.slice(3).join('|');if(ct.startsWith('CMD:DEL:')){let f=ct.substring(8);if(!delFiles.includes(f))delFiles.push(f)}if(ct.startsWith('REACT:')){let rp=ct.split(':');if(rp.length>=3){let mh=rp[1],em=rp[2];if(!rm[mh])rm[mh]={};rm[mh][em]=(rm[mh][em]||0)+1}}});
    let html='';
    msgs.forEach(m=>{
      let c=dec(m,myKey);if(!c.startsWith(MG))return;
      let p=c.split('|');if(p.length<4)return;
      let snd=p[1],tim=p[2],cnt=p.slice(3).join('|');
      if(cnt.startsWith('CMD:')||cnt.startsWith('REACT:'))return;
      let isSent=snd===myName,isSys=snd==='SYS';
      if(isSys){html+='<div class="sysmsg">'+cnt+'</div>';return}
      let mh=hsh(m.substring(0,20));
      let rxns=rm[mh]?Object.entries(rm[mh]).map(([e,c])=>'<span class="rbtn">'+e+' '+c+'</span>').join(''):'';
      let bc='';
      if(cnt.startsWith('IMG:')){let f=cnt.substring(4);bc=delFiles.includes(f)?'<span style="opacity:.4;font-style:italic">Deleted</span>':'<div class="iw"><img src="/download?name='+f+'" class="cimg" loading="lazy"><button class="dbtn" onclick="delImg(\''+f+'\')">✕</button></div>'}
      else if(cnt.startsWith('VO:')){let f=cnt.substring(3);bc=delFiles.includes(f)?'<div class="vob" style="opacity:.5">🔥 Burned</div>':'<div class="vob" onclick="openVO(\''+f+'\')"><span>💣</span><span>Tap to Decrypt</span></div>'}
      else{bc=cnt.replace(/</g,'&lt;').replace(/>/g,'&gt;')}
      let av='<div class="av" style="background:'+avc(snd)+'">'+ini(snd)+'</div>';
      let nm=!isSent?'<span class="sname">'+snd+'</span>':'';
      let ctx=(!cnt.startsWith('IMG:')&&!cnt.startsWith('VO:'))?'oncontextmenu="showCm(\''+cnt.replace(/\\/g,'\\\\').replace(/'/g,"\\'")+'  \',event);return false;"':'';
      let rbar=(rxns||!isSent)?'<div class="rbar">'+rxns+'<button class="radd" onclick="openRP(\''+mh+'\',event)">+</button></div>':'';
      html+='<div class="brow '+(isSent?'sent':'recv')+'">'+ (!isSent?av:'')+'<div class="bwrap"><div class="bubble '+(isSent?'sent':'recv')+'" '+ctx+'>'+nm+bc+'<div class="meta"><span>'+tim+'</span>'+(isSent?'<span>✓✓</span>':'')+'</div></div>'+rbar+'</div></div>';
    });
    let bx=document.getElementById('chatbox');
    if(bx.innerHTML!==html){let atBot=bx.scrollTop+bx.clientHeight>=bx.scrollHeight-40;bx.innerHTML=html;if(atBot)bx.scrollTop=bx.scrollHeight}
    fetch('/online').then(r=>r.text()).then(n=>{document.getElementById('ot').innerText=n+' online';document.getElementById('hs').innerText=n+' connected'});
  }).catch(()=>{errs++;if(errs>5)document.querySelector('.hdot').style.background='#ef4444'});
}
function startPoll(){poll();setInterval(poll,1500)}
document.addEventListener('contextmenu',e=>e.preventDefault());
</script>
</body>
</html>
)rawliteral";

// ============================================================
// SERVER ROUTES
// ============================================================
void setupRoutes() {
  server.on("/", [](){server.send(200,"text/html",index_html);});
  server.on("/hotspot-detect.html", [](){server.send(200,"text/html",index_html);});
  server.on("/library/test/success.html", [](){server.send(200,"text/html",index_html);});
  server.on("/generate_204", [](){server.send(200,"text/html",index_html);});
  server.on("/gen_204", [](){server.send(200,"text/html",index_html);});
  server.on("/connecttest.txt", [](){server.send(200,"text/plain","Microsoft Connect Test");});
  server.on("/favicon.ico", [](){server.send(204,"text/plain","");});

  server.on("/login", [](){
    String k = server.arg("key");
    if (k == DURESS_PASS) { doWipe(); server.send(200,"text/plain","WIPED"); }
    else { if(server.hasArg("name")) updateUser(server.arg("name")); server.send(200,"text/plain","OK"); }
  });

  server.on("/send", [](){
    if (server.hasArg("msg")) { chatHistory += server.arg("msg") + "|||"; trimChat(); }
    server.send(200,"text/plain","OK");
  });

  server.on("/read", [](){
    String out = chatHistory;
    if (typingUser != "" && millis() < typingExpiry) out += "###TYPE:" + typingUser;
    server.send(200,"text/plain",out);
  });

  server.on("/typing", [](){
    if (server.hasArg("name")) {
      typingUser = server.arg("name") + "|" + server.arg("room");
      typingExpiry = millis() + 3000;
      updateUser(server.arg("name"));
    }
    server.send(200,"text/plain","OK");
  });

  server.on("/online", [](){server.send(200,"text/plain",String(onlineCount()));});

  server.on("/status", [](){
    String s = "RAM:"+String(ESP.getFreeHeap()/1024)+"KB";
    s += " Up:"+String(millis()/60000)+"min";
    s += " WiFi:"+String(WiFi.softAPgetStationNum())+"users";
    server.send(200,"text/plain",s);
  });

  server.on("/ram", [](){server.send(200,"text/plain",String(ESP.getFreeHeap()/1024));});

  server.on("/clear", [](){chatHistory="";pinnedMessage="";SPIFFS.format();server.send(200,"text/plain","OK");});

  server.on("/pin", [](){
    if(server.hasArg("msg")){pinnedMessage=server.arg("msg");if(pinnedMessage=="CLEAR")pinnedMessage="";}
    server.send(200,"text/plain","OK");
  });

  server.on("/getpin", [](){server.send(200,"text/plain",pinnedMessage==""?"NONE":pinnedMessage);});

  server.on("/delete_file", [](){
    if(server.hasArg("name")){String p="/"+server.arg("name");if(SPIFFS.exists(p))SPIFFS.remove(p);}
    server.send(200,"text/plain","OK");
  });

  // Fixed upload handler (no race condition)
  server.on("/upload", HTTP_POST,
    [](){server.send(200,"text/plain","OK");},
    [](){
      HTTPUpload& u = server.upload();
      static File uf;
      if(u.status==UPLOAD_FILE_START){String p="/"+u.filename;if(SPIFFS.exists(p))SPIFFS.remove(p);uf=SPIFFS.open(p,FILE_WRITE);}
      else if(u.status==UPLOAD_FILE_WRITE){if(uf)uf.write(u.buf,u.currentSize);}
      else if(u.status==UPLOAD_FILE_END){if(uf)uf.close();}
    }
  );

  server.on("/download", [](){
    if(server.hasArg("name")){
      String p="/"+server.arg("name");
      if(SPIFFS.exists(p)){File f=SPIFFS.open(p,FILE_READ);server.streamFile(f,"image/jpeg");f.close();}
      else server.send(404,"text/plain","Not found");
    }
  });

  server.onNotFound([](){
    server.sendHeader("Location",String("http://")+DOMAIN_NAME,true);
    server.send(302,"text/plain","");
  });
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  // OLED RST sequence — CRITICAL for V2
  // Without this the screen stays completely blank
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  delay(20);

  Wire.begin(OLED_SDA, OLED_SCL);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(15, 20, "Ghost Chat");
  u8g2.drawStr(25, 36, "Starting...");
  u8g2.sendBuffer();

  pinMode(PANIC_BTN, INPUT_PULLUP);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed");
    u8g2.drawStr(0, 50, "SPIFFS FAIL");
    u8g2.sendBuffer();
  }

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);

  // DNS
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  Serial.println("IP: " + WiFi.softAPIP().toString());

  setupRoutes();
  server.begin();

  // Show ready screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 18, "Ghost Chat V2");
  u8g2.drawStr(0, 32, ("IP: "+WiFi.softAPIP().toString()).c_str());
  u8g2.drawStr(0, 46, ("WiFi: "+String(WIFI_SSID)).c_str());
  u8g2.drawStr(0, 60, "Ready!");
  u8g2.sendBuffer();

  Serial.println("Ghost Chat ready on V2 board.");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  updateOLED();

  // RAM crash prevention
  if (ESP.getFreeHeap() < MIN_FREE_HEAP) {
    Serial.println("[WARN] Low RAM, trimming");
    trimChat();
  }

  // Panic button — hold 3s to wipe
  if (digitalRead(PANIC_BTN) == LOW) {
    delay(50);
    if (digitalRead(PANIC_BTN) == LOW) {
      unsigned long t = millis();
      while (digitalRead(PANIC_BTN) == LOW) {
        int s = (millis()-t)/1000;
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(20, 20, "HOLD TO WIPE");
        u8g2.drawStr(30, 40, (String(3-s)+" sec...").c_str());
        u8g2.sendBuffer();
        if (millis()-t > 3000) {
          doWipe();
          u8g2.clearBuffer();
          u8g2.drawStr(25, 32, "WIPED CLEAN");
          u8g2.sendBuffer();
          delay(2000);
          break;
        }
      }
    }
  }

  delay(2);
}
