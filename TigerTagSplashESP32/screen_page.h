// screen_page.h — the browser side of /screen, served from flash by the scale.
//
// A separate file rather than a raw string literal inside the .ino, and not by
// preference: the Arduino prototype generator scans raw string literals as if
// they were code, so `async function poll()` in this script became a generated
// C++ prototype at the top of the .ino and nothing compiled. Headers are not
// scanned. Same reason live_page.h is separate.
//
// Deliberately small and dependency-free: a diagnostic tool that needs a build
// step is one nobody reaches for. A click taps, a drag swipes, and the
// coordinates come off the image's own getBoundingClientRect() so the mapping
// survives CSS scaling on a phone.
//
// ASCII only, no emoji, per the repository rule.
#pragma once

static const char SCREEN_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TigerScale screen</title>
<style>
 body{margin:0;background:#0b0e14;color:#8b95a7;font:13px ui-monospace,Menlo,monospace;
      display:flex;flex-direction:column;align-items:center;gap:10px;padding:14px}
 img{width:min(96vw,720px);aspect-ratio:480/320;image-rendering:pixelated;
     border-radius:10px;border:1px solid #2e3646;touch-action:none;cursor:crosshair}
 #s{min-height:1.2em}
</style></head><body>
<img id="v" alt="scale screen">
<div id="s">connecting</div>
<script>
const v=document.getElementById('v'), s=document.getElementById('s');
let seq=-1, busy=false, down=null;
function at(e){const r=v.getBoundingClientRect();
  return [Math.round((e.clientX-r.left)/r.width*480),
          Math.round((e.clientY-r.top)/r.height*320)];}
function shot(){ if(busy) return; busy=true;
  // Load into a detached image and swap only once it has decoded. Assigning
  // src directly blanks the element for the second and a half the 450 KB
  // takes, which reads as "the screen went black".
  const n=new Image();
  n.onload=()=>{ v.src=n.src; busy=false; };
  n.onerror=()=>{ busy=false; };
  n.src='/screen.bmp?t='+Date.now(); }
async function poll(){
  try{ const r=await fetch('/screen.ver',{cache:'no-store'});
       const b=new Uint8Array(await r.arrayBuffer());
       const n=b[0]|b[1]<<8|b[2]<<16|b[3]<<24;
       if(n!==seq){ seq=n; shot(); s.textContent='frame '+n; } }
  catch(e){ s.textContent='offline'; }
}
v.addEventListener('pointerdown',e=>{down=at(e); v.setPointerCapture(e.pointerId);});
v.addEventListener('pointerup',async e=>{
  if(!down) return; const up=at(e), from=down; down=null;
  const far=Math.abs(up[0]-from[0])>12||Math.abs(up[1]-from[1])>12;
  const q=far?`x=${from[0]}&y=${from[1]}&x2=${up[0]}&y2=${up[1]}`
             :`x=${from[0]}&y=${from[1]}`;
  s.textContent=far?'swipe...':'tap...';
  try{ await fetch('/api/tap?'+q,{cache:'no-store'}); }catch(e){}
  poll();
});
setInterval(poll,700); poll();
</script></body></html>)HTML";
