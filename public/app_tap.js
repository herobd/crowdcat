var OK_THRESH=-100;
var BAD_THRESH=100;

var spinner;
var lastRemoved=[];
var lastRemovedInfo=[];

var headerColors = ['#E0B0FF','#7fb13d','#CD7F32','#C5908E','#95B9C7'];
var spottingColors = ['#f3e5fb','#cad6bb','#eedece','#e7d6d5','#d7e6ec'];
//var ondeckColors = ['#eed3ff','#b3d389','#ddb185','#dfb8b7','#bbcbd1'];
var colorIndex=0;

/*function handleTouchStart(evt) {
    if (evt.touches)                                     
        this.xDown = evt.touches[0].clientX;  
    else
           this.xDown = evt.clientX;                                 
    //yDown = evt.touches[0].clientY;                                      
};                                                

function handleTouchMove(evt) {
    if ( ! this.xDown) {
        return;
    }
    
                                
    //var yUp = evt.touches[0].clientY;

    this.xDiff = xUp-this.xDown;
    this.style.left = this.xDiff+'px';
    
    //this.getElementsByClassName('num')[0].innerHTML='( '+xUp+' )'
    if (this.xDiff>BAD_THRESH) {
        this.parentNode.style.background='hsl(350,100%,40%)';
    } else if (this.xDiff>0) {
        this.parentNode.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        //console.log((75*(this.xDiff/(0.0+OK_THRESH)))+'%');
    } else if (this.xDiff<OK_THRESH) {
        this.parentNode.style.background='hsl(130,100%,30%)';
    } else if (this.xDiff<0) {
        this.parentNode.style.background='hsl(130,'+(60*(this.xDiff/(0.0+OK_THRESH)))+'%,30%)';
    }                           
};

function removeSpotting(OK) {
    lastRemoved.push(this);
    //lastRemovedParent.push(this.parentNode);
    
    lastRemovedOK.push(OK);
    if (lastRemoved.length>10) {
        lastRemoved.shift();
        //lastRemovedParent.shift();
        lastRemovedOK.shift();
    }
    //this.parentNode.removeChild(this);
    this.hidden=true;
    
    if (OK)
        isGood(this);
    else
        isBad(this);
}*/

function undo() {
    if (lastRemoved.length>0) {
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        lastRemoved.pop().hidden=false;
        var container = document.getElementById('b'+batchQueue[0].id);
        container.hidden=true;
        batchQueue = [lastRemovedInfo.pop()].concat(batchQueue);
        //TODO do something with 
    }
    //console.log('UNDO');
}

function handleTouchEnd(evt) {
    //console.log(evt);
    toggleIsBad(this);
    this.classList.toggle('crossed');
}

function setup() {
    spinner = document.getElementById("spinner");
    var windows = document.getElementsByClassName('window');
    for (var i = 0; i < windows.length; i++) {
       //initSlider(windows[i]);
        //windows[i].addEventListener('touchstart', function(e){ e.preventDefault(); });
        //windows[i].addEventListener('mousedown', function(e){ e.preventDefault(); });
        var doneButton = document.createElement("button");
        doneButton.classList.toggle('donebutton');
        doneButton.innerHTML='Submit';
        doneButton.addEventListener('mouseup', batchDone, false);
        //doneButton.addEventListener('touchend', batchDone, false);
        windows[i].appendChild(doneButton);
    }
    /*var containers = document.getElementsByClassName('container');
    for (var i = 0; i < containers.length; i++) {
        var doneButton = document.createElement("button");
        doneButton.classList.toggle('donebutton');
        doneButton.innerHTML='Submit';
        doneButton.addEventListener('mouseup', batchDone, false);
        doneButton.addEventListener('touchend', batchDone, false);
        containers[i].appendChild(doneButton);
    }*/
    document.getElementById("instructions").innerHTML='Select incorrect images.';
    begin();
}

function createTapped(im,id,batchId) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('spotting');
    genDiv.appendChild(im);
    genDiv.id=id;
    genDiv.batch=batchId;
    initTapped(genDiv);
    
    var stikethrough = document.createElement("div");
    stikethrough.classList.toggle('stikethrough');
    stikethrough.hidden=true;
    genDiv.appendChild(stikethrough);
    genDiv.style.background=spottingColors[colorIndex];
    genDiv.colorIndex=colorIndex;
    return genDiv;
}

function initTapped(ele) {
    //ele.addEventListener('touchend', handleTouchEnd, false);
    
    ele.addEventListener('mouseup', handleTouchEnd, false);
    //ele.xDown=null;
}

//////////////////////

var batches={};
var maxImgWidth=700;
var imgWidth=null;
var numBatch;

var test_batchesToDo=3;

function begin() {
    var w = window,
    d = document,
    e = d.documentElement,
    g = d.getElementsByTagName('body')[0],
    x = w.innerWidth || e.clientWidth || g.clientWidth,
    y = w.innerHeight|| e.clientHeight|| g.clientHeight;
    while (!imgWidth)
        imgWidth=Math.min(x,maxImgWidth);
    numBatch=Math.floor((y-document.getElementById('title').offsetHeight-50)/84)-1;
    console.log ('numBatch='+numBatch+' full='+y+' top='+document.getElementById('title').offsetHeight);
    document.onresize=function(){imgWidth=Math.min(x,maxImgWidth);};
    var windows = document.getElementsByClassName('window');
    for (var i=0; i<windows.length; i++) {
        getNextBatch(windows[i],3);
        
    }
    
    
}
function httpGetAsync(theUrl, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
        else if (xmlHttp.readyState == 4)
            callback(xmlHttp.status);
            
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}
function httpPostAsync(theUrl, theData, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
        else if (xmlHttp.readyState == 4)
            callback(xmlHttp.status);
            
    }
    xmlHttp.open("POST", theUrl, true); // true for asynchronous 
    xmlHttp.setRequestHeader("Content-type", 'application/json');
    xmlHttp.send(JSON.stringify(theData));
}

function toggleIsBad(widget) {
    batches[widget.batch].spottings[widget.id]=!batches[widget.batch].spottings[widget.id];
    //isBatchDone(widget.batch,widget.parentNode.parentNode);
    for(var i in widget.childNodes) {
        if (widget.childNodes.hasOwnProperty(i)) {
            stikethrough=widget.childNodes[i];
            stikethrough.hidden=!stikethrough.hidden;
        }
    }
}


function test() {
    httpGetAsync('/app/test_image?quality=9',function (res){
        var myImage = new Image(imgWidth);
        myImage.src=res;
        document.getElementById('testImageHere').appendChild(myImage);
    });
}



var batchQueue=[];
var lastNgram='';
function getNextBatch(window,toload) {
    httpGetAsync('/app/nextBatch?num='+numBatch+'&width='+imgWidth,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                //console.log("got batch "+jres.batchId);
                batches[jres.batchId]={sent:false, spottings:{}};
                var batchContainer = document.createElement("div");
                batchContainer.id='b'+jres.batchId;
                
                var batchHeader = document.createElement("div");
                batchHeader.classList.toggle('spotting');
                batchHeader.classList.toggle('batchHeader');
                batchHeader.innerHTML=jres.ngram;
                if (lastNgram!=jres.ngram) {
		            colorIndex = (++colorIndex)%headerColors.length;
		            lastNgram=jres.ngram;
	            }
		        batchHeader.style.background=headerColors[colorIndex];
		        
		        batchContainer.appendChild(batchHeader);
                
                for (var index=0; index<jres.spottings.length; index++) {
                    var i=jres.spottings[index];
                    var im = new Image();
                    im.src='data:image/png;base64,'+i.data;
                    var widget = createTapped(im,i.id,jres.batchId);
                    batchContainer.appendChild(widget);
                    batches[jres.batchId].spottings[i.id]=1;//assume good
                }
                
                window.insertBefore(batchContainer,window.childNodes[0]);
                if (batchQueue.length>0) {
                             batchContainer.hidden=true
		        }
                batchQueue.push({ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
                spinner.hidden=true;
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(window,toload);
        }
        
        
    });
}

function batchDone(evt) {
    if (batchQueue.length==0)
        return;
    var batchId=batchQueue[0].id;
    //console.log(batchId);
    var window = this.parentNode;
    //if (!batches[batchId].sent) {
        batches[batchId].sent=true;
        var info = batchQueue[0];
	    var ngram = info.ngram;
	    var resultsId = info.rid;
	    batchQueue=batchQueue.slice(1)
	    if (batchQueue.length>0) {
                document.getElementById('b'+batchQueue[0].id).hidden=false;
	    } else {
	        spinner.hidden=false;
	    }
        var container = document.getElementById('b'+batchId);
        //window.removeChild(container);
        container.hidden=true;
        lastRemoved.push(container);
        lastRemovedInfo.push(info);
        if (lastRemoved.length>2) {
            lastRemoved.shift();
            lastRemovedInfo.shift();
        }
        httpPostAsync('/app/submitBatch',{batchId:batchId,resultsId:resultsId,labels:batches[batchId].spottings},function (res){
            //if(--test_batchesToDo > 0) {
                getNextBatch(window);
            //} else {
            //    window.location.href = "/done";
            //}
        });
    //} else {
    //    //TODO handle UNDO case
    //    assert(false);
    //}
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
