var OK_THRESH=100;
var BAD_THRESH=-100;

//var lastRemovedParent=[];
var lastRemoved=[];
var lastRemovedOK=[];
var spinner;

var headerColors = ['#E0B0FF','#7fb13d','#CD7F32','#C5908E','#95B9C7'];
var spottingColors = ['#f3e5fb','#cad6bb','#eedece','#e7d6d5','#d7e6ec'];
//var ondeckColors = ['#eed3ff','#b3d389','#ddb185','#dfb8b7','#bbcbd1'];
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
    this.style.left = this.xDiff+'px';
    
    //this.getElementsByClassName('num')[0].innerHTML='( '+xUp+' )'
    if (this.xDiff<BAD_THRESH) {
        this.parentNode.style.background='hsl(350,100%,40%)';
    } else if (this.xDiff<0) {
        this.parentNode.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        //console.log((75*(this.xDiff/(0.0+OK_THRESH)))+'%');
    } else if (this.xDiff>OK_THRESH) {
        this.parentNode.style.background='hsl(130,100%,30%)';
    } else if (this.xDiff>0) {
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
}

function undo() {
    if (lastRemoved.length>0) {
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        lastRemoved.pop().hidden=false;
        //TODO do something with 
        lastRemovedOK.pop();
    }
    //console.log('UNDO');
}

function handleTouchEnd(evt) {
    //var xUp = evt.touches[0].clientX;    
    //this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
    
    this.xDown=null;
    this.style.left = '0px';
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
    for (var i = 0; i < windows.length; i++) {
        //initSlider(windows[i]);
        windows[i].addEventListener('touchstart', function(e){ e.preventDefault(); });
        windows[i].addEventListener('mousedown', function(e){ e.preventDefault(); });
    }
    begin();
}

function createSlider(im,id,batchId) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('spotting');
    genDiv.appendChild(im);
    genDiv.id=id;
    genDiv.batch=batchId;
    initSlider(genDiv);
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

var batches={};
var maxImgWidth=700;
var imgWidth=null;

var test_batchesToDo=3;

function begin() {
    while (!imgWidth)
        imgWidth=Math.min(document.defaultView.outerWidth,maxImgWidth);
    document.onresize=function(){imgWidth=Math.min(document.defaultView.outerWidth,maxImgWidth);};
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

function isBad(widget) {
    batches[widget.batch].spottings[widget.id]=false;
    isBatchDone(widget.batch,widget.parentNode);
}

function isGood(widget) {
    batches[widget.batch].spottings[widget.id]=true;
    isBatchDone(widget.batch,widget.parentNode);
}

function test() {
    httpGetAsync('/app/test_image?quality=9',function (res){
        var myImage = new Image(imgWidth);
        myImage.src=res;
        document.getElementById('testImageHere').appendChild(myImage);
    });
}
var batchQueue=[]
function getNextBatch(window,toload) {
    httpGetAsync('/app/nextBatch?width='+imgWidth,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                batches[jres.batchId]={sent:false, spottings:{}};
                
                var batchHeader = document.createElement("div");
                batchHeader.classList.toggle('spotting');
                batchHeader.classList.toggle('batchHeader');
                batchHeader.id='b'+jres.batchId
                batchHeader.innerHTML=jres.ngram;
		        if (batchQueue.length>0 && jres.ngram == batchQueue[batchQueue.length-1].ngram) {
                             batchHeader.hidden=true
		        } else {
		            colorIndex = (++colorIndex)%headerColors.length;
		            lastNgram=jres.ngram;
	            }
	            batchHeader.style.background=headerColors[colorIndex];
		        batchQueue.push({ngram:jres.ngram, id:jres.batchId})
                window.appendChild(batchHeader);
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
                    window.appendChild(widget);
                    //initSlider(widget);
                    batches[jres.batchId].spottings[i.id]=null;
                }
                spinner.hidden=true;
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(window,toload);
        }
        
        
    });
}

function isBatchDone(batchId,window) {
    //if (!batches[batchId].sent) {
        for (spottingId in batches[batchId].spottings)
            if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==null)
                return;
        batches[batchId].sent=true;
	    var ngram = batchQueue[0].ngram;
	    batchQueue=batchQueue.slice(1)
	    if (batchQueue.length>0 && ngram==batchQueue[0].ngram) {
                document.getElementById('b'+batchQueue[0].id).hidden=false;
	    } else if (batchQueue.length==0){
	        spinner.hidden=false;
	    }
        var header = document.getElementById('b'+batchId);
        window.removeChild(header);
        httpPostAsync('/app/submitBatch',{batchId:batchId,labels:batches[batchId].spottings},function (res){
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
