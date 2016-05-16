

var OK_THRESH=100;
var BAD_THRESH=-100;

//var lastRemovedParent=[];
var lastRemoved=[];
var lastRemovedEle=[];
var lastRemovedOK=[];
var spinner;

var ondeck;
var theWindow;

var batches={};
var maxImgWidth=700;
var imgWidth=null;

var test_batchesToDo=3;

//pink,green, orange, pink, blue
var headerColors = ['#E0B0FF','#7fb13d','#CD7F32','#C5908E','#95B9C7'];
var spottingColors = ['#f3e5fb','#cad6bb','#eedece','#e7d6d5','#d7e6ec'];
var ondeckColors = ['#eed3ff','#b3d389','#ddb185','#dfb8b7','#bbcbd1'];
var colorIndex=0;

function handleTouchStart(evt) {
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
    
    var xUp;
    if (evt.touches)
        xUp = evt.touches[0].clientX; 
    else {
        if (evt.buttons%2!=1) {
            handleTouchEnd.call(this,evt);
            return;
        }
        xUp = evt.clientX; 
    }                              
    //var yUp = evt.touches[0].clientY;

    this.xDiff = xUp-this.xDown;
    ondeck.style.left = this.xDiff+'px';
    
    //this.getElementsByClassName('num')[0].innerHTML='( '+xUp+' )'
    if (this.xDiff<BAD_THRESH) {
        theWindow.style.background='hsl(350,100%,40%)';
    } else if (this.xDiff<0) {
        theWindow.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        //console.log((75*(this.xDiff/(0.0+OK_THRESH)))+'%');
    } else if (this.xDiff>OK_THRESH) {
        theWindow.style.background='hsl(130,100%,30%)';
    } else if (this.xDiff>0) {
        theWindow.style.background='hsl(130,'+(60*(this.xDiff/(0.0+OK_THRESH)))+'%,30%)';
    }                           
};

function removeSpotting(OK) {
    lastRemoved.push(this);
    lastRemovedEle.push(ondeck);
    //lastRemovedParent.push(this.parentNode);
    
    lastRemovedOK.push(OK);
    if (lastRemoved.length>10) {
        lastRemoved.shift();
        //lastRemovedParent.shift();
        lastRemovedOK.shift();
        lastRemovedEle.shift();
    }
    //this.parentNode.removeChild(this);
    //this.hidden=true;
    theWindow.removeChild(ondeck);
    
    
    if (OK)
        isGood();
    else
        isBad();
    
    highlightLast();
}

function createInlineLabel(label) {
    var ret = document.createElement("LABEL");
    ret.innerHTML='redo: '+label;
    //ret.toggle
    return ret;
}

function undo() {
    if (lastRemoved.length>0) {
        ondeck.classList.toggle('ondeck');
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        lastRemoved.pop().hidden=false;
        ondeck=lastRemovedEle.pop();
        if (ondeck.childNodes.length<2) {
            ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngram),ondeck.childNodes[0]);
            ondeck.classList.toggle('redo');
            //colorIndex = (++colorIndex)%headerColors.length;
            ondeck.style.background=spottingColors[ondeck.colorIndex];
        }
            
        theWindow.appendChild(ondeck);
        //TODO do something with 
        lastRemovedOK.pop();
    }
    //console.log('UNDO');
}

function handleTouchEnd(evt) {
    //var xUp = evt.touches[0].clientX;    
    //this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
    
    this.xDown=null;
    ondeck.style.left = '0px';
    //var xDiff = this.xDown - xUp;
    if (this.xDiff>OK_THRESH) {
        removeSpotting.call(this,true);
        
    } else if (this.xDiff<BAD_THRESH) {
        removeSpotting.call(this,false);
    } else {
    
        
    }
    this.xDiff=0;
}

function setup() {
    spinner = document.getElementById("spinner");
    var windows = document.getElementsByClassName('window');
    theWindow=windows[0];
    //for (var i = 0; i < windows.length; i++) {
    //    //initSlider(windows[i]);
    windows[0].addEventListener('touchstart', function(e){ e.preventDefault(); });
    windows[0].addEventListener('mousedown', function(e){ e.preventDefault(); });
        
    //}
    var containers = document.getElementsByClassName('container');
    initSlider(containers[0]);
    containers[0].addEventListener('touchstart', function(e){ e.preventDefault(); });
    containers[0].addEventListener('mousedown', function(e){ e.preventDefault(); });
    
    while (!imgWidth)
        imgWidth=Math.min(document.defaultView.outerWidth,maxImgWidth);
    document.onresize=function(){imgWidth=Math.min(document.defaultView.outerWidth,maxImgWidth);};
    getNextBatch(3,function() { 
        highlightLast();
        var headers = theWindow.getElementsByClassName('batchHeader');
        headers[headers.length-1].classList.toggle('ondeck');
    });
    
}

function createSlider(im,id,batchId) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('spotting');
    genDiv.appendChild(im);
    genDiv.id=id;
    genDiv.batch=batchId;
    //initSlider(genDiv);
    genDiv.style.background=spottingColors[colorIndex];
    genDiv.colorIndex=colorIndex;
    return genDiv;
}

function initSlider(ele) {
    ele.addEventListener('touchstart', handleTouchStart, false);        
    ele.addEventListener('touchmove', handleTouchMove, false);
    ele.addEventListener('touchend', handleTouchEnd, false);
    
    ele.addEventListener('mousedown', handleTouchStart, false);        
    ele.addEventListener('mousemove', handleTouchMove, false);
    ele.addEventListener('mouseup', handleTouchEnd, false);
    ele.xDown=null;
}

//////////////////////




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

function isBad() {
    batches[ondeck.batch].spottings[ondeck.id]=false;
    isBatchDone(ondeck.batch);
    //console.log('is bad');
}

function isGood() {
    batches[ondeck.batch].spottings[ondeck.id]=true;
    isBatchDone(ondeck.batch);
    //console.log('is good');
}

function test() {
    httpGetAsync('/app/test_image?quality=9',function (res){
        var myImage = new Image(imgWidth);
        myImage.src=res;
        document.getElementById('testImageHere').appendChild(myImage);
    });
}
var batchQueue=[]
//var lastNgram='';
function getNextBatch(toload,callback) {
    httpGetAsync('/app/nextBatch?width='+imgWidth,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                console.log("got batch "+jres.batchId);
                batches[jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};
                
                var batchHeader = document.createElement("div");
                //batchHeader.classList.toggle('spotting');
                batchHeader.classList.toggle('batchHeader');
                batchHeader.id='b'+jres.batchId
                batchHeader.innerHTML='<div>'+jres.ngram+'</div>';
		        if (batchQueue.length>0 && jres.ngram == batchQueue[batchQueue.length-1].ngram) {
                    batchHeader.hidden=true
                    
		        } else {
		            colorIndex = (++colorIndex)%headerColors.length;
		            lastNgram=jres.ngram;
	            }
		        /*if (lastNgram!=jres.ngram) {
		            colorIndex = (++colorIndex)%headerColors.length;
		            lastNgram=jres.ngram;
	            }*/
		        batchHeader.style.background=headerColors[colorIndex];
		        
		        batchQueue.push({ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
                theWindow.insertBefore(batchHeader,theWindow.childNodes[0]);
                for (var index=0; index<jres.spottings.length; index++) {
                    var i=jres.spottings[index];
                    var im = new Image();
                    im.src='data:image/png;base64,'+i.data;
                    //var widget = document.createElement("div");
                    //widget.classList.toggle('spotting');
                    //widget.appendChild(im);
                    //widget.id=i.id;
                    //widget.batch=jres.batchId;
                    var widget = createSlider(im,i.id,jres.batchId);
                    theWindow.insertBefore(widget,theWindow.childNodes[0]);
                    //initSlider(widget);
                    batches[jres.batchId].spottings[i.id]=null;
                }
                spinner.hidden=true;
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(toload,callback);
            else if (callback!==undefined)
                callback();
            
            
        }
        
        
    });
}

function highlightLast() {
    var spottings = theWindow.getElementsByClassName('spotting');
    ondeck=spottings[spottings.length-1];
    ondeck.classList.toggle('ondeck');
    //console.log('toggle');
    ondeck.style.background=ondeckColors[ondeck.colorIndex];
}

function isBatchDone(batchId) {
    if (!batches[batchId].sent) {
        for (spottingId in batches[batchId].spottings)
            if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==null)
                return;
        batches[batchId].sent=true;
	    var ngram = batchQueue[0].ngram;
	    var resultsId = batchQueue[0].rid;
	    batchQueue=batchQueue.slice(1)
	    if (batchQueue.length>0 && ngram==batchQueue[0].ngram) {
                document.getElementById('b'+batchQueue[0].id).hidden=false;
	    } else if (batchQueue.length==0){
	        spinner.hidden=false;
	    }
	    document.getElementById('b'+batchQueue[0].id).classList.toggle('ondeck');
        var header = document.getElementById('b'+batchId);
        if (header)
            theWindow.removeChild(header);
        httpPostAsync('/app/submitBatch',{batchId:batchId,resultsId:resultsId,labels:batches[batchId].spottings},function (res){
            //if(--test_batchesToDo > 0) {
                getNextBatch();
            //} else {
            //    window.location.href = "/done";
            //}
        });
    } else {
        //TODO handle UNDO case
    //    assert(false);
    }
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
