(function(){var q=window,d=q.bShareUtil,b=q.bShare,a=b.config,r=b.whost,z=b.imageBasePath,B=[],C=["bsharesync","xinhuamb"],j=document,s=j.documentElement,t=j.body,u=Math.max,k=0,x=0,g=a.lang==="en",y=function(){var e=j.getElementById("bsPanelHolder");if(!e)e=j.createElement("div"),e.id="bsPanelHolder";var h=e;d.loadStyle("a.bsSiteLink{text-decoration:none;color:"+a.poptxtc+";}a.bshareDiv{overflow:hidden;height:16px;line-height:18px;font-size:14px;color:#333;padding-left:0;}a.bshareDiv:hover{text-decoration:none;}a.bsSiteLink:hover{text-decoration:underline;}div.buzzButton{cursor:pointer;}div.bsRlogo,div.bsRlogoSel{width:60px;float:left;margin:0 4px;padding:2px 0;}div.bsLogo,div.bsLogoSel{float:left;width:111px;text-align:left;height:auto;padding:2px 4px;margin:2px 0;white-space:nowrap;overflow:hidden;}div.bsLogoSel,div.bsRlogoSel{border:1px solid #ddd;background:#f1f1f1;}div.bsLogo,div.bsRlogo{border:1px solid #fff;background:#fff;}div.bsLogo a,div.bsLogoSel a{display:block;height:16px;line-height:16px;padding:0 0 0 24px;text-decoration:none;}div.bsLogoSel a,div.bsRlogoSel a{color:#000;border:none;}div.bsLogo a,div.bsRlogo a{color:#666;border:none;}div.bsLogoLink{width:121px;overflow:hidden;background:#FFF;float:left;margin:3px 0;}#bsLogin{float:right;text-align:right;overflow:hidden;height:100%;}#bsPanel{position:absolute;z-index:100000000;font-size:12px;width:"+
(111*a.popHCol+(a.popHCol-1)*10+26+(d.isIe&&!d.isSt?6:0))+"px;background:url("+b.shost+"/frame/images/background-opaque-dark."+(d.isIe6?"gif":"png")+");padding:6px;-moz-border-radius:5px;-webkit-border-radius:5px;border-radius:5px;}div.bsClear{clear:both;height:0;line-height:0;font-size:0;overflow:hidden;}");var i,m=g?"More...":"\u66f4\u591a\u5e73\u53f0...",n=g?"More":"\u66f4\u591a...";i=g?"Register a bShare account":"\u6ce8\u518c\u6210\u4e3abShare\u7528\u6237";var k=g?"Login to bShare":"\u767b\u5f55bShare",q=g?"Register":"\u6ce8\u518c",s=g?"Signup":"\u6ce8\u518c",t=g?"Login":"\u767b\u5f55",u='<div style="padding:0 8px;border-bottom:1px solid #e8e8e8;color:'+
a.poptxtc+";background:"+a.popbgc+';text-align:left;"><a style="float:left;height:20px;line-height:20px;font-weight:bold;" class="bsSiteLink" target="_blank" href="'+r+'/intro">'+(g?"Share To...":"\u5206\u4eab\u5230...")+'</a><a class="bsSiteLink" style="cursor:pointer;float:right;height:20px;line-height:20px;font-weight:bold;" onclick="document.getElementById(\'bsPanel\').style.display=\'none\';">X</a><div class="bsClear"></div></div><div class="bsClear"></div>',l="",v,f,c,o;if(a.popHCol==2){l+='<div style="height:47px;border-bottom:1px #ccc solid;padding:4px 0 4px 16px;margin-right:8px;_padding-left:12px;">';
for(f=0;f<3&&f<a.bps.length;f++)c=a.bps[f],b.pnMap[c]!==void 0&&(l+='<div class="bsRlogo" onmouseover="javascript:this.className=\'bsRlogoSel\'" onmouseout="javascript:this.className=\'bsRlogo\'"><a href="javascript:void(0);" onclick="javascript:bShare.share(event,\''+c+'\');return false;" style="text-decoration:none;line-height:120%;"><div style="cursor:pointer;width:24px;height:24px;margin:0 18px 2px;background:url('+z,o=b.pnMap[c][0],l+="/logos/m2/"+c+'.gif) no-repeat;"></div><div style="cursor:pointer;text-align:center;width:60px;overflow:hidden;color:inherit;white-space:nowrap;line-height:120% !important;">'+
o+"</div></a></div>");l+="</div>"}var w=[],A,p;for(c=0;c<a.popHCol;c++)w.push("<div class='bsLogoLink'>");A=a.popHCol<2&&a.bps.length>6?6:a.bps.length;for(f=0,v=a.popHCol==2?3:0;v<A;v++)c=a.bps[v],b.pnMap[c]!==void 0&&(o=b.pnMap[c][0],p=(d.hasElem(B,c)?"font-weight:bold;":"")+(d.hasElem(C,c)?"color:red;":""),w[f%a.popHCol]+='<div class="bsLogo" onmouseover="javascript:this.className=\'bsLogoSel\'" onmouseout="javascript:this.className=\'bsLogo\'"><a href="javascript:void(0);" onclick="javascript:bShare.share(event,\''+
c+'\');return false;" style="'+p+"background:url("+z,p=b.pnMap[c][1]*-18,w[f%a.popHCol]+=(p?"/slogos_sprite8."+(d.isIe?"gif":"png")+") no-repeat 0 "+p+'px;">'+o+"</a>":"/logos/s4/"+c+(d.isIe?".gif":".png")+') no-repeat;">'+o+"</a>")+"</div>",f++);x=116+26*Math.ceil(f/a.popHCol)-(a.popHCol!=2?56:0);for(c=0;c<a.popHCol;c++)l+=w[c]+"</div>";l+="<div class='bsClear'></div>";c=0;for(var y in b.pnMap)b.pnMap.hasOwnProperty(y)&&c++;m='<div style="height:20px;line-height:20px;padding:0 8px;border-top:1px solid #e8e8e8;color:'+
a.poptxtc+";background:"+a.popbgc+';"><div class="buzzButton" style="float:left;">'+(a.popHCol==1?n:m+' <font style="font-weight:normal;">('+c+")</font>")+'</div><div id="bsLogin" style="'+(a.popHCol<2?"width:80px;":"")+'">';a.logo&&(m+=b.isLite?'<a class="bsSiteLink" href="'+r+'" target="_blank"><span style="font-size:10px;vertical-align:bottom;line-height:24px;"><span style="color:#ff5c00;">b</span>Share</span></a>':b.user?'<a class="bsSiteLink" href="'+r+'/userProfile" title="'+b.user+'" target="_blank" style="width:120px;display:block;overflow:hidden;">'+
b.user+"</a>":'<a class="bsSiteLink" href="'+r+'/register" title="'+i+'" target="_blank">'+(a.popHCol==1?s:q)+'</a>&nbsp;&nbsp;<a class="bsSiteLink" title="'+k+'" href="javascript:void(0);" onclick="javascript:bShare.share(event,\'login\');return false;">'+t+"</a>");m+="</div></div>";i=u+"<div style='padding-left:8px;background:#fff;*height:"+(26*Math.ceil(f/a.popHCol)+6+(a.popHCol==2?56:0))+"px;'>"+l+"</div>"+m;h.innerHTML='<div id="bsPanel" style="display:none;">'+i+"</div>";j.body.appendChild(e);
return e};b.ready=function(){y();var a=j.getElementById("bsPanel");d.getElem(j,"a","bshareDiv",function(h,i){d.getElem(h,"div","buzzButton",function(d){d.onmouseover=function(a){return function(){b.hover(a)}}(i);d.onmouseout=function(){k=setTimeout(function(){a.style.display="none"},1E3)}})});d.getElem(a,"div","buzzButton",function(a,d){a.onclick=function(a){b.more(a,d);return!1}});a.onmouseout=function(){k=setTimeout(function(){a.style.display="none"},600)};a.onmouseover=function(){k!==0&&clearTimeout(k)}};
!a.bps||a.bps.length==0?setTimeout(function(){if(!a.bps||a.bps.length==0)a.bps="bsharesync,sinaminiblog,qqmb,renren,qzone,sohuminiblog,douban,kaixin001,baiduhi,qqxiaoyou,neteasemb,ifengmb,email,facebook,twitter,tianya,clipboard".split(","),b.ready()},2E3):b.ready();b.hover=function(e){k!==0&&clearTimeout(k);e||(e=0);var h=d.getElem(j,"a","bshareDiv")[e],i=d.getOffset(h).y,g=d.getOffset(h).x,h=h.offsetHeight,n=j.getElementById("bsPanel");n.style.left=g+"px";d.getWH();n.style.top=a.pop==2||a.pop!=1&&
i-{t:u(s.scrollTop,t.scrollTop),l:u(s.scrollLeft,t.scrollLeft)}.t+x+h>d.getWH().h?i-x-2+"px":i+h+2+"px";n.style.display="";b.prepare(e);b.click()}})();
