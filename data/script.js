let eB=s=>document.getElementById(s);
let to;
function dak(datei){
    clearTimeout(to);
    to=setTimeout(()=>{
        dak();
        dho(datei);
    },3000);
}
function dho(datei){
    let x=new XMLHttpRequest();
    x.overrideMimeType('application/json');
    x.open('GET','/' + datei,true);
    x.onreadystatechange=()=>{
        if(x.readyState==4&&x.status==200){
            sD(x.responseText);
        }
    };
    x.send();
}
