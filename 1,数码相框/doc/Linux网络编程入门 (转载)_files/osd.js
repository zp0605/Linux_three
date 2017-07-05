(function(){var n=void 0,p=!0,q=null,r=!1,s=this,aa=function(a){var b=typeof a;if("object"==b)if(a){if(a instanceof Array)return"array";if(a instanceof Object)return b;var c=Object.prototype.toString.call(a);if("[object Window]"==c)return"object";if("[object Array]"==c||"number"==typeof a.length&&"undefined"!=typeof a.splice&&"undefined"!=typeof a.propertyIsEnumerable&&!a.propertyIsEnumerable("splice"))return"array";if("[object Function]"==c||"undefined"!=typeof a.call&&"undefined"!=typeof a.propertyIsEnumerable&&
!a.propertyIsEnumerable("call"))return"function"}else return"null";else if("function"==b&&"undefined"==typeof a.call)return"object";return b},ba=function(a,b,c){return a.call.apply(a.bind,arguments)},da=function(a,b,c){if(!a)throw Error();if(2<arguments.length){var f=Array.prototype.slice.call(arguments,2);return function(){var c=Array.prototype.slice.call(arguments);Array.prototype.unshift.apply(c,f);return a.apply(b,c)}}return function(){return a.apply(b,arguments)}},t=function(a,b,c){t=Function.prototype.bind&&
-1!=Function.prototype.bind.toString().indexOf("native code")?ba:da;return t.apply(q,arguments)},ea=function(a,b){var c=Array.prototype.slice.call(arguments,1);return function(){var b=Array.prototype.slice.call(arguments);b.unshift.apply(b,c);return a.apply(this,b)}};var v=document,fa=window;var w=function(a,b){this.width=a;this.height=b};w.prototype.floor=function(){this.width=Math.floor(this.width);this.height=Math.floor(this.height);return this};w.prototype.round=function(){this.width=Math.round(this.width);this.height=Math.round(this.height);return this};function x(a){return"function"==typeof encodeURIComponent?encodeURIComponent(a):escape(a)}var y=function(a,b,c){a.addEventListener?a.addEventListener(b,c,r):a.attachEvent&&a.attachEvent("on"+b,c)},z=function(a,b,c){a.removeEventListener?a.removeEventListener(b,c,r):a.detachEvent&&a.detachEvent("on"+b,c)},ga=function(){var a=A;try{return!!a.location.href||""===a.location.href}catch(b){return r}};var ha=!!window.google_async_iframe_id,A=ha&&window.parent||window,B=function(){if(ha&&!ga()){for(var a="."+v.domain;2<a.split(".").length&&!ga();)v.domain=a=a.substr(a.indexOf(".")+1),A=window.parent;ga()||(A=window)}return A};var C=function(a,b){this.x=a!==n?a:0;this.y=b!==n?b:0};var D,ia,E,ja,ka=function(){return s.navigator?s.navigator.userAgent:q};ja=E=ia=D=r;var F;if(F=ka()){var la=s.navigator;D=0==F.indexOf("Opera");ia=!D&&-1!=F.indexOf("MSIE");E=!D&&-1!=F.indexOf("WebKit");ja=!D&&!E&&"Gecko"==la.product}var G=D,H=ia,K=ja,L=E,ma;
a:{var M="",N;if(G&&s.opera)var na=s.opera.version,M="function"==typeof na?na():na;else if(K?N=/rv\:([^\);]+)(\)|;)/:H?N=/MSIE\s+([^\);]+)(\)|;)/:L&&(N=/WebKit\/(\S+)/),N)var oa=N.exec(ka()),M=oa?oa[1]:"";if(H){var pa,qa=s.document;pa=qa?qa.documentMode:n;if(pa>parseFloat(M)){ma=String(pa);break a}}ma=M}
var ra=ma,sa={},ta=function(a){var b;if(!(b=sa[a])){b=0;for(var c=String(ra).replace(/^[\s\xa0]+|[\s\xa0]+$/g,"").split("."),f=String(a).replace(/^[\s\xa0]+|[\s\xa0]+$/g,"").split("."),d=Math.max(c.length,f.length),e=0;0==b&&e<d;e++){var g=c[e]||"",h=f[e]||"",l=RegExp("(\\d*)(\\D*)","g"),i=RegExp("(\\d*)(\\D*)","g");do{var j=l.exec(g)||["","",""],k=i.exec(h)||["","",""];if(0==j[0].length&&0==k[0].length)break;b=((0==j[1].length?0:parseInt(j[1],10))<(0==k[1].length?0:parseInt(k[1],10))?-1:(0==j[1].length?
0:parseInt(j[1],10))>(0==k[1].length?0:parseInt(k[1],10))?1:0)||((0==j[2].length)<(0==k[2].length)?-1:(0==j[2].length)>(0==k[2].length)?1:0)||(j[2]<k[2]?-1:j[2]>k[2]?1:0)}while(0==b)}b=sa[a]=0<=b}return b},ua={},O=function(a){return ua[a]||(ua[a]=H&&!!document.documentMode&&document.documentMode>=a)};var va;!H||O(9);!K&&!H||H&&O(9)||K&&ta("1.9.1");H&&ta("9");var S=function(a){return a?new ya(R(a)):va||(va=new ya)},R=function(a){return 9==a.nodeType?a:a.ownerDocument||a.document},ya=function(a){this.q=a||s.document||document};ya.prototype.createElement=function(a){return this.q.createElement(a)};var za=function(a){var b=a.q;a=!L&&"CSS1Compat"==b.compatMode?b.documentElement:b.body;b=b.parentWindow||b.defaultView;return new C(b.pageXOffset||a.scrollLeft,b.pageYOffset||a.scrollTop)};var T=function(a,b,c,f){this.top=a;this.right=b;this.bottom=c;this.left=f};var U=function(a,b){var c;a:{c=R(a);if(c.defaultView&&c.defaultView.getComputedStyle&&(c=c.defaultView.getComputedStyle(a,q))){c=c[b]||c.getPropertyValue(b)||"";break a}c=""}return c||(a.currentStyle?a.currentStyle[b]:q)||a.style&&a.style[b]},Aa=function(a){var b=a.getBoundingClientRect();H&&(a=a.ownerDocument,b.left-=a.documentElement.clientLeft+a.body.clientLeft,b.top-=a.documentElement.clientTop+a.body.clientTop);return b},Ba=function(a){if(H&&!O(8))return a.offsetParent;var b=R(a),c=U(a,"position"),
f="fixed"==c||"absolute"==c;for(a=a.parentNode;a&&a!=b;a=a.parentNode)if(c=U(a,"position"),f=f&&"static"==c&&a!=b.documentElement&&a!=b.body,!f&&(a.scrollWidth>a.clientWidth||a.scrollHeight>a.clientHeight||"fixed"==c||"absolute"==c||"relative"==c))return a;return q},Ca=function(a){var b,c=R(a),f=U(a,"position"),d=K&&c.getBoxObjectFor&&!a.getBoundingClientRect&&"absolute"==f&&(b=c.getBoxObjectFor(a))&&(0>b.screenX||0>b.screenY),e=new C(0,0),g;b=c?R(c):document;if(g=H)if(g=!O(9))g="CSS1Compat"!=S(b).q.compatMode;
g=g?b.body:b.documentElement;if(a==g)return e;if(a.getBoundingClientRect)b=Aa(a),a=za(S(c)),e.x=b.left+a.x,e.y=b.top+a.y;else if(c.getBoxObjectFor&&!d)b=c.getBoxObjectFor(a),a=c.getBoxObjectFor(g),e.x=b.screenX-a.screenX,e.y=b.screenY-a.screenY;else{d=a;do{e.x+=d.offsetLeft;e.y+=d.offsetTop;d!=a&&(e.x+=d.clientLeft||0,e.y+=d.clientTop||0);if(L&&"fixed"==U(d,"position")){e.x+=c.body.scrollLeft;e.y+=c.body.scrollTop;break}d=d.offsetParent}while(d&&d!=a);if(G||L&&"absolute"==f)e.y-=c.body.offsetTop;
for(d=a;(d=Ba(d))&&d!=c.body&&d!=g;)if(e.x-=d.scrollLeft,!G||"TR"!=d.tagName)e.y-=d.scrollTop}return e},Da=/matrix\([0-9\.\-]+, [0-9\.\-]+, [0-9\.\-]+, [0-9\.\-]+, ([0-9\.\-]+)p?x?, ([0-9\.\-]+)p?x?\)/;var V=function(a,b,c,f,d,e){this.k=a;this.w=(a.bottom-a.top)*(a.right-a.left);this.l=this.h=-1;this.b=[0,0,0,0,0];this.e=[0,0,0,0,0];this.d=[0,0,0,0,0];this.zoom=[0,0,0,0,0];this.m="";this.o=r;this.v=p;this.z=c;this.i=this.a=-1;this.s=b;this.f=0;this.g=f;this.t=d||"";this.u=e||"";this.r=function(){};this.element=q;this.n=0;this.c=-1;this.j=-1!=b.indexOf("39482001");this.A=-1!=b.indexOf("39482011");this.B=0},Ea=new T(0,0,0,0);
V.prototype.update=function(a,b,c){var f=this.k,d=a-this.z||1;this.z=a;var e=Math.max(f.top,b.top),g=Math.min(f.bottom,b.bottom),h=-1,l=-1;if(e<=g){var i=Math.max(f.left,b.left),f=Math.min(f.right,b.right);i<=f&&(e=(g-e)*(f-i)/this.w,h=1<=e?0:0.75<=e?1:0.5<=e?2:0.25<=e?3:4,0>this.h&&(this.h=a),this.l=a,0.5<=e&&(l=100*this.w/((b.bottom-b.top)*(b.right-b.left)),l=20<=l?0:10<=l?1:5<=l?2:2.5<=l?3:4))}-1!=this.a&&(this.b[this.a]+=d,2>=this.a&&-1!=this.i&&(this.zoom[this.i]+=d));for(a=this.a;0<=a&&4>=a;a++)if(this.d[a]+=
d,c||-1==h||a<h)this.d[a]>this.e[a]&&(this.e[a]=this.d[a]),this.d[a]=0;this.a=c?-1:h;this.i=l;this.r(this,b)};V.prototype.C=function(a){this.c=a()};V.prototype.p=function(a){a=a();this.n+=a-this.c;this.c=-1};var W=function(a){return(a.j||a.A)&&!!a.element&&!!a.element.contentWindow&&1>a.B};var Fa=function(a){var b=fa;a&&b.top!=b&&(b=b.top);try{var c;if(b.document&&!b.document.body)c=new w(-1,-1);else{var f=(b||window).document,d="CSS1Compat"==f.compatMode?f.documentElement:f.body;c=new w(d.clientWidth,d.clientHeight)}return c}catch(e){return new w(-12245933,-12245933)}};var Y=function(){Ga(X,r)},Ga=function(a,b){if(!(Ha||0==a.length)){var c=Fa(p);b||(Ia=c);if(!(-1==c.width||-1==c.height||-12245933==c.width||-12245933==c.height)){var f=q;try{f=za(S((window.top||s||window).document))}catch(d){return}for(var c=new T(f.y,f.x+c.width,f.y+c.height,f.x),f=Z(),e=0;e<a.length;e++)a[e].update(f,c,b);Ja+=Z()-f;Ka++}}},La=function(){var a;a=fa.document;a={visible:1,hidden:2,prerender:3,preview:4}[a.webkitVisibilityState||a.mozVisibilityState||""]||0;Ga(X,!(1==a||0==a))},X=[],
Ha=r,Ma=(new Date).getTime(),Qa=-1,Ia=q,Z=function(){return(new Date).getTime()-Ma},Ra=0,Sa=0,Ja=0,Ka=0,Ta=-1,Va=function(){for(var a=12E4,b=Z(),c=0;c<X.length;++c){var f;f=X[c];if(W(f)){var d;d=f.b[2]+f.b[1]+f.b[0];2>=f.a&&-1!=f.a&&(d+=b-f.l);if(0<=f.c||12E4>d)f=12E4-d;else{if(W(f)){d=n;b:{if(d=f.s)if((d=d.match("eid=([^&]+)"))&&2==d.length){d=d[1];break b}d=""}var e=f.j?"39482002":"39482011";f.j&&(d=d.replace("39482001",e));e={"0":"autorefresh"};e[1]=d;try{f.element.contentWindow.postMessage(e,
"*"),f.B++}catch(g){f.j=r,f.A=r}}f=12E4}}else f=12E4;a=Math.min(a,f)}Ua()&&(a=Math.max(a,5E3),window.setTimeout(Va,a))},Ua=function(){for(var a=0;a<X.length;++a)if(W(X[a]))return p;return r};var $,Xa=function(){if(2==Wa()||Ua())return p;for(var a=X,b=0;b<a.length;b++)if(!a[b].o)return p;return r},db=function(){try{var a=B(),b=Z();Qa=b;Ia=Fa(p);var c;var f=B().document;!f.body||!f.body.getBoundingClientRect||"function"!=typeof Goog_AdSense_getAdAdapterInstance?c=r:($=Goog_AdSense_getAdAdapterInstance(),c=p);if(c){Ra=$.numBlocks();var d=[];Sa=0;$.getBlocks(function(a,c,e){Sa++;var f=a.getBoundingClientRect();try{var g=window.top,h=new C(0,0),i=R(a)?R(a).parentWindow||R(a).defaultView:window,
j=a;do{var l;if(i==g)l=Ca(j);else{var k=j,m=new C;if(1==k.nodeType){if(k.getBoundingClientRect){var J=Aa(k);m.x=J.left;m.y=J.top}else{var Oa=za(S(k)),Pa=Ca(k);m.x=Pa.x-Oa.x;m.y=Pa.y-Oa.y}if(K&&!ta(12)){var u=m,ca;var I=n;H?I="-ms-transform":L?I="-webkit-transform":G?I="-o-transform":K&&(I="-moz-transform");var P=n;I&&(P=U(k,I));P||(P=U(k,"transform"));if(P){var wa=P.match(Da);ca=!wa?new C(0,0):new C(parseFloat(wa[1]),parseFloat(wa[2]))}else ca=new C(0,0);m=new C(u.x+ca.x,u.y+ca.y)}}else{var fb="function"==
aa(k.D),u=k;k.targetTouches?u=k.targetTouches[0]:fb&&k.D().targetTouches&&(u=k.D().targetTouches[0]);m.x=u.clientX;m.y=u.clientY}l=m}u=l;h.x+=u.x;h.y+=u.y}while(i&&i!=g&&(j=i.frameElement)&&(i=i.parent));var gb=new T(Math.round(h.y),Math.round(f.right-f.left+h.x),Math.round(f.bottom-f.top+h.y),Math.round(h.x)),Q=new V(gb,c,b,e);Q.element=a;Q.r=Ya;Q.m=Za($a,c);var xa=c.match(/[&\?](?:adk)=([0-9]+)/);Q.f=xa&&2==xa.length?parseInt(xa[1],10):0;d.push(Q)}catch(rb){d.push(new V(Ea,c,b,e))}});X=d;var e,
g;v.mozVisibilityState?g="mozvisibilitychange":v.webkitVisibilityState&&(g="webkitvisibilitychange");(e=g)&&y(v,e,La);La();for(c=0;c<X.length;++c)if(W(X[c])){window.setTimeout(Va,12E4);break}if(2!=Wa()){y(a,"message",ab);c=X;for(f=0;f<c.length;++f){var h=c[f];if(h.element&&h.element.contentWindow){e={"0":"goog_get_override"};try{h.element.contentWindow.postMessage(e,"*")}catch(l){}}}a.setTimeout(bb,500)}var i=2==$.getOseId();y(a,"scroll",Y);y(a,"resize",Y);if(i)for(var j,a=0;a<X.length;++a)if(j=X[a],
j.element){var k=t(j.C,j,Z);y(j.element,"mouseover",k);var m=t(j.p,j,Z);y(j.element,"mouseout",m)}window.setTimeout(function(){cb("t")},36E5);Ta=Z()-b}else cb("c")}catch(J){X=[],cb("x")}},cb=function(a){var b=B(),c=b.document;$||($=Goog_AdSense_getAdAdapterInstance());if(!Ha){if(2==Wa()){var f=0<=Qa?Z()-Qa:-1;"u"==a&&-1==Ta&&(a="l",X=[],f=Z());var d;d||(d="http");d=[[d,"://pagead2.googlesyndication.com/pagead/gen_204?id=osd"].join("")];try{var e=X;if(0<e.length){Ga(e,p);for(c=0;c<e.length;c++)if(0<
e[c].f){0<e[c].c&&e[c].p(Z);var g=e[c],h=g.k,l=["p:",h.top,h.left,h.bottom,h.right];l.push("tos:",g.b.join(","));l.push("mtos:",g.e.join(","));l.push("rs:",g.g);5!=g.g&&(l.push("zoom:",g.zoom.join(",")),l.push("ht:",g.n));0<=g.h&&l.push("tfs:",g.h,"tls:",g.l);g.m&&l.push("fp:",g.m);5==g.g&&(g.u&&l.push("ord:",g.u),g.t&&l.push("amd:",g.t,";"));d.push("adk"+e[c].f+"="+x(l.join(",")))}g=p;h=Ia;d.push("bs="+h.width+","+h.height);var i;h=fa;g&&h.top!=h&&(h=h.top);try{var j=h.document;i=!h.scrollY&&"CSS1Compat"!=
j.compatMode?new T(0,j.body.scrollWidth,j.body.scrollHeight,0):new T(0,j.body.offsetWidth,j.body.offsetHeight,0)}catch(k){i=new T(0,-12245933,-12245933,0)}d.push("ps="+i.right+","+i.bottom);b.screen&&d.push("ss="+b.screen.width+","+b.screen.height);var m=Za(hb,e[0].s);if(m&&("&"==m.charAt(0)||"?"==m.charAt(0)))m=m.slice(1);d.push("fp="+x(m))}else d.push("url="+x(b.location.href)),c.referrer&&d.push("referrer="+x(c.referrer)),$&&(d.push("correlator="+$.getCorrelator()),d.push("eid="+$.getOseExpId()),
d.push("oid="+Wa()));d.push("tt="+f);d.push("pt="+Qa);d.push("deb="+x([1,Ra,Sa,Ja,Ka,Ta].join("-")));d.push("r="+a);ib&&d.push("ovr=t");if(b.top!=b){d.push("iframe_loc="+x(b.location.href));var J=Fa(r);d.push("is="+J.width+","+J.height)}}catch(pb){d.push("error")}try{var eb=d.join("&");b.google_image_requests||(b.google_image_requests=[]);var Na=b.document.createElement("img");Na.src=eb;b.google_image_requests.push(Na)}catch(qb){}}Ha=p}},$a=/[&\?](?:slotname|dt|ifi|adx|ady|format)=[^&]+/g,hb=/[&\?](?:client|correlator|url|ifk|oid|eid)=[^&]+/g,
Za=function(a,b){var c=b.match(a);return c?c.join(""):""},ib=r,jb=0,kb=function(a,b){if(b){a(b);var c=b.frames;if(c){var f=c.length,d;for(d=0;d<f;++d)kb(a,c[d])}}},lb=function(a,b){try{b.postMessage(a,"*")}catch(c){}},Ya=function(a,b){if(a&&!a.o){var c=1E3<=a.e[2];if(c||a.v){var f=c?"1":"0";a.v=r;var d=$.getCorrelator(),e=a.k,f=["{vi:",f,",cl:",d,",adk:",a.f,",rs:",a.g,",pl:",e.left,",pr:",e.right,",pt:",e.top,",pb:",e.bottom,",vl:",b.left,",vr:",b.right,",vt:",b.top,",vb:",b.bottom,"}"].join(""),
e=a.element;try{var d=[],g=e.contentWindow||(e.contentDocument||e.contentWindow.document).parentWindow||(e.contentDocument||e.contentWindow.document).defaultView;if(g)d=[g];else{var h;var l,i=document,i=e||i;h=i.querySelectorAll&&i.querySelector?i.querySelectorAll("IFRAME"):l=i.getElementsByTagName("IFRAME");for(i=0;i<h.length;++i)(g=h[i].contentWindow||(h[i].contentDocument||h[i].contentWindow.document).parentWindow||(h[i].contentDocument||h[i].contentWindow.document).defaultView)&&d.push(g)}var j=
d.length;if(0<j)for(var k=ea(lb,f),i=0;i<j;++i)kb(k,d[i])}catch(m){}if(c&&(a.o=p,!Xa()&&(c=2==$.getOseId(),g=B(),z(g,"scroll",Y),z(g,"resize",Y),c)))for(g=0;g<X.length;++g)c=X[g],c.element&&(h=t(c.C,c,Z),z(c.element,"mouseover",h),h=t(c.p,c,Z),z(c.element,"mouseout",h))}}},ab=function(a){a.data&&(a=a.data,"goog_provide_override"==a[0]&&(a=a[1],0<a&&(1>=a&&a>jb)&&(jb=a)))},bb=function(){if(0<jb){var a;a:{a=[2];var b=jb;if(!(1E-4>Math.random())){var c=Math.random();if(c<b){a=a[Math.floor(c/b*a.length)];
break a}}a=q}2==a&&(ib=p)}},Wa=function(){return ib?2:$?$.getOseId():0};var mb=B();y(mb,"unload",function(){cb("u")});var nb=B();if("complete"==nb.document.readyState||nb.google_onload_fired)db();else{var ob=B();y(ob,"load",function(){window.setTimeout(db,100)})};})();
