
var spinner;
var lastRemoved=[];

var toBeInQueue=3;

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
        batchQueue = [lastRemovedBatchInfo.pop()].concat(batchQueue);
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
    console.log("SETUP")
    spinner = document.getElementById("spinner");
    var windows = document.getElementsByClassName('window');
    for (var i = 0; i < windows.length; i++) {
       //initSlider(windows[i]);
        //windows[i].addEventListener('touchstart', function(e){ e.preventDefault(); });
        //windows[i].addEventListener('mousedown', function(e){ e.preventDefault(); });
        var doneButton = document.createElement("button");
        doneButton.classList.toggle('donebutton');
        doneButton.innerHTML='Next';
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
    var innerDiv = document.createElement("div");
    innerDiv.classList.toggle('centerer');
    innerDiv.appendChild(im);
    
    genDiv.id=id;
    genDiv.batch=batchId;
    initTapped(genDiv);
    
    var stikethrough = document.createElement("div");
    stikethrough.classList.toggle('strikethrough');
    stikethrough.hidden=true;
    innerDiv.appendChild(stikethrough);
    genDiv.appendChild(innerDiv);
    genDiv.style.background=spottingColors[colorIndex];
    genDiv.style.height=spottingHeight+'px';
    genDiv.colorIndex=colorIndex;
    
    console.log('height: '+genDiv.style.height)
    return genDiv;
}

function initTapped(ele) {
    //ele.addEventListener('touchend', handleTouchEnd, false);
    
    ele.addEventListener('mouseup', handleTouchEnd, false);
    //ele.xDown=null;
}

//////////////////////


var numBatch;
var min_spotting_height=102;
var spottingHeight;

function begin() {
    var w = window,
    d = document,
    e = d.documentElement,
    g = d.getElementsByTagName('body')[0],
    x = w.innerWidth || e.clientWidth || g.clientWidth,
    y = w.innerHeight|| e.clientHeight|| g.clientHeight;
    //var x= document.getElementsByClassName('window')[0].clientWidth;
    //var y= document.getElementsByClassName('window')[0].clientHeight;
    while (!imgWidth)
        imgWidth=Math.min(x,maxImgWidth);
    
    var windowHeight = y-document.getElementById('title').offsetHeight-document.getElementsByClassName('donebutton')[0].offsetHeight-32;
    
    numBatch=Math.floor(windowHeight/min_spotting_height);//Math.floor((y-document.getElementById('title').offsetHeight-50)/84)-1;
    spottingHeight=Math.floor(windowHeight/numBatch) -2;
    console.log ('numBatch='+numBatch+' full='+windowHeight+' spottingH='+spottingHeight);
    document.onresize=function(){imgWidth=Math.min(x,maxImgWidth);};
    var windows = document.getElementsByClassName('window');
    for (var i=0; i<windows.length; i++) {
        getNextBatch(windows[i],toBeInQueue);
        
    }
    
    
}



function toggleIsBad(widget) {
    batches[widget.batch].spottings[widget.id]=!batches[widget.batch].spottings[widget.id];
    //isBatchDone(widget.batch,widget.parentNode.parentNode);
    /*for(var i in widget.childNodes) {
        if (widget.childNodes.hasOwnProperty(i)) {
            //var stikethrough=widget.childNodes[i];
            //if (stikethrough.classList.contains('strikethrough'))
            //    stikethrough.hidden=!stikethrough.hidden;
            
        }
    }*/
    var stikethrough = widget.getElementsByClassName('strikethrough')[0];
            stikethrough.hidden=!stikethrough.hidden;
}




var lastNgram='';
function getNextBatch(windowEle,toload) {
    if (allReceived) {
        console.log('ERROR allrecieved, but sending for more')
        return
    }
    query='';
    var prevNgram='.';
    if (batchQueue.length>0)
        prevNgram=batchQueue[batchQueue.length-1].ngram;
    if (testMode)
        query='&test='+testNum; 
    httpGetAsync('/app/nextBatch?num='+numBatch+'&width='+imgWidth+'&color='+colorIndex+'&prevNgram='+prevNgram+query,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                //console.log("got batch "+jres.batchId);
                batches[jres.batchId]={sent:false, spottings:{}};
                var batchContainer = document.createElement("div");
                batchContainer.id='b'+jres.batchId;
                batchContainer.classList.toggle('batchContainer');
                
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
                    batches[jres.batchId].spottings[i.id]=true;//assume good
                }
                
                windowEle.insertBefore(batchContainer,windowEle.childNodes[0]);
                if (batchQueue.length>0) {
                    batchContainer.hidden=true;
	            } 
                /*if (batchQueue.length==0) {
                    batchContainer.classList.toggle('show');
	            }*/ 
	            if (jres.resultsId!=='X')
                    batchQueue.push({ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
                else
                    allReceived=true;
                spinner.hidden=true;
                
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(windowEle,toload);
        }
        
        
    });
}

function batchDone(evt) {
    if (batchQueue.length==0)
        return;
    var batchId=batchQueue[0].id;
    //console.log(batchId);
    var windowEle = this.parentNode;
    
    //base
    batchShiftAndSend(batchId,function(){if (batchQueue.length<toBeInQueue) getNextBatch(windowEle);});
    //base
    var container = document.getElementById('b'+batchId);
        
    container.hidden=true;
    //container.classList.toggle('show');
    //container.classList.toggle('hide');
    lastRemoved.push(container);
    
    if (lastRemoved.length>2) {
        var removed = lastRemoved.shift();
        lastRemovedBatchInfo.shift();
        windowEle.removeChild(removed);
    }
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
