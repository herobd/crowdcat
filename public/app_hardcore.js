

var OK_THRESH=85;
var BAD_THRESH=-85;


var lastRemovedEle=[];
var spinner;

var ondeck;
var theWindow;

var toBeInQueue=3;
var swipeOn=true;



function handleTouchStart(evt) {
    if (evt.touches)                                     
        this.xDown = evt.touches[0].clientX;  
    else
           this.xDown = evt.clientX;                                 
                                         
};                                                

function handleTouchMove(evt) {
    if ( ! this.xDown || !swipeOn) {
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
    

    this.xDiff = xUp-this.xDown;
    ondeck.style.left = this.xDiff+'px';
    
    
    if (this.xDiff<BAD_THRESH) {
        theWindow.style.background='hsl(350,100%,40%)';
    } else if (this.xDiff<0) {
        theWindow.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        
    } else if (this.xDiff>OK_THRESH) {
        theWindow.style.background='hsl(130,100%,30%)';
    } else if (this.xDiff>0) {
        theWindow.style.background='hsl(130,'+(60*(this.xDiff/(0.0+OK_THRESH)))+'%,30%)';
    }                           
};

function removeSpotting(OK) {
    lastRemovedEle.push(ondeck);
    
    if (lastRemovedEle.length>10) {
        var removed=lastRemovedEle.shift();
        if (lastRemovedEle[0].batch!=lastRemovedBatchInfo[0].id) {
            lastRemovedBatchInfo.shift();
            //console.log(lastRemovedBatchInfo)
        }
    }
    
    
    ondeck.classList.toggle('spotting');
    ondeck.classList.toggle('collapser');
    
    
    batches[ondeck.batch].spottings[ondeck.id]=OK;
    isBatchDone(ondeck.batch);
    
    highlightLast();
}

function createInlineLabel(label) {
    var ret = document.createElement("LABEL");
    ret.innerHTML='redo: '+label;
    //ret.toggle
    return ret;
}

function undo() {
    if (lastRemovedEle.length>0) {
        countUndos++;
        ondeck.classList.toggle('ondeck');
        var ondeckHeader = document.getElementById('b'+batchQueue[0].id)
            if (ondeckHeader)
                ondeckHeader.classList.toggle('ondeck');
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        ondeck=lastRemovedEle.pop();
        if (ondeck.childNodes.length<2) {
            ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngram),ondeck.childNodes[0]);
            ondeck.classList.toggle('redo');
            //colorIndex = (++colorIndex)%headerColors.length;
            ondeck.style.background=spottingColors[ondeck.colorIndex];
        }
        ondeck.classList.toggle('spotting');
        ondeck.classList.toggle('collapser');
        batches[ondeck.batch].spottings[ondeck.id]=null
        
        if (lastRemovedBatchInfo.length>0){
            var pastInfo= ondeck.batch;//lastRemovedBatchInfo[lastRemovedBatchInfo.length-1];
            //console.log(pastInfo +' ?= '+batchQueue[0].id)
            if (pastInfo!=batchQueue[0].id) {
                batchQueue = [lastRemovedBatchInfo.pop()].concat(batchQueue);
            }
            
        }   
        if (ondeck.batch!=batchQueue[0].id)
                console.log('ERROR, batchQueue head not mathcing ondeck: '+ondeck.batch+' '+batchQueue[0].id)
        theWindow.appendChild(ondeck);
    }
    
}

function handleTouchEnd(evt) {
    if (!swipOn)
        return;
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
    //windows[0].addEventListener('touchstart', function(e){ e.preventDefault(); });
    //windows[0].addEventListener('mousedown', function(e){ e.preventDefault(); });
    //initSlider(theWindow);
    
    
    //}
    var containers = document.getElementsByClassName('container');
    initSlider(containers[0]);
    containers[0].addEventListener('touchstart', function(e){ e.preventDefault(); });
    containers[0].addEventListener('mousedown', function(e){ e.preventDefault(); });
    
    while (!imgWidth)
        imgWidth=Math.min(document.defaultView.innerWidth,maxImgWidth);
    console.log(imgWidth);
    document.onresize=function(){imgWidth=Math.min(document.defaultView.innerWidth,maxImgWidth);};
    getNextBatch(toBeInQueue,function() { 
        highlightLast();
        var headers = theWindow.getElementsByClassName('batchHeader');
        headers[headers.length-1].classList.toggle('ondeck');
    });
    
    //document.getElementById('leftIcon').addEventListener('mouseup', function(e){removeSpotting(false); e.preventDefault();}, false);
    //document.getElementById('rightIcon').addEventListener('mouseup', function(e){removeSpotting(true); e.preventDefault();}, false);
    
    if (testMode) {
        document.getElementById('instructions').addEventListener('mouseup', function(e){
            e.preventDefault(); 
            startTime = new Date().getTime();
            this.parentNode.removeChild(this);
        }, false);
    }
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
    genDiv.addEventListener("webkitAnimationEnd", function(e){theWindow.removeChild(this);}, false);
    genDiv.addEventListener("animationend", function(e){theWindow.removeChild(this);}, false);
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
    //console.log('getting '+allReceived);
    if (allReceived) {
        console.log('ERROR allrecieved, but sending for more')
        return
    }
    var query='';
    var prevNgram='.';
    if (batchQueue.length>0)
        prevNgram=batchQueue[batchQueue.length-1].ngram;
    if (testMode) {
        query+='&test='+testNum;
        if (toload==toBeInQueue)
            query+='&reset=true';
    }
    httpGetAsync('/app/nextBatch?width='+imgWidth+'&color='+colorIndex+'&prevNgram='+prevNgram+query,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                //console.log("got batch "+jres.batchId);
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
	            
	            if (jres.resultsId!=='X') {
	                batchQueue.push({ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
	                //console.log("got "+jres.resultsId)
                } else if (jres.batchId=='R') {
                    location.reload(true);
                } else {
                    allReceived=true;
                    //console.log("all recieved")
                }
                //console.log('gott '+allReceived);
                
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
                
            } else if (jres.batchType=='transcription') {
                //batches[jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};
                
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
	            
	        if (jres.batchId!=='R' && jres.batchId!=='X') {
	            batchQueue.push({ngram:'#', id:jres.batchId, transcription:'#'});
	            //console.log("got "+jres.resultsId)
                } else if (jres.batchId=='R') {
                    location.reload(true);
                } 
                
                var wordImg = new Image();
                wordImg.src='data:image/png;base64,'+jres.wordImg;
                var ngramImg = new Image();
                ngramImg.src='data:image/png;base64,'+jres.ngramImg;
                theWindow.insertBefore(createTranscriptionSelector(jres.batchId,wordImg,ngramImg,jres.possibilities),theWindow.childNodes[0]);
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
    if (ondeck) {
        ondeck.classList.toggle('ondeck');
    
        ondeck.style.background=ondeckColors[ondeck.colorIndex];
    }
}

function isBatchDone(batchId) {
    
    if (batcheQueue[0].ngram == '#')
        for (spottingId in batches[batchId].spottings)
            if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==null)
                return;
    
    //base
    batchShiftAndSend(batchId,function(){if (batchQueue.length<toBeInQueue) getNextBatch();});
    //base
    var nextElement=null;
    if (batchQueue.length>0) {
        nextElement=document.getElementById('b'+batchQueue[0].id);
    }
    if (nextElement) {
        nextElement.classList.toggle('ondeck');
        if (batchQueue[0].ngram=='#') {
            //TODO hide gradient
        }
    }
    var oldElement = document.getElementById('b'+batchId);
    if (oldElement) {
        if (lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].ngram!='#') {
            if (lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].ngram == batchQueue[0].ngram) {
                theWindow.removeChild(oldElement);
                
                //we shift the header past the collapsing element to provide the illusion of a smooth transition
                if (nextHeader) {
                    theWindow.removeChild(nextHeader);
                    theWindow.appendChild(nextHeader);
                }
            } else {
                oldElement.addEventListener("webkitAnimationEnd", function(e){theWindow.removeChild(this);}, false);
                oldElement.addEventListener("animationend", function(e){theWindow.removeChild(this);}, false);
                oldElement.classList.toggle('batchHeader');
                oldElement.classList.toggle('collapserH');
            }
        } else {
            //TODO fix gradient
            oldElement.addEventListener("webkitAnimationEnd", function(e){theWindow.removeChild(this);}, false);
            oldElement.addEventListener("animationend", function(e){theWindow.removeChild(this);}, false);
            oldElement.classList.toggle('collapser');
        }
    }
}

function classify(id,word) {
    return function(ele) {
        batchQueue[0].transcription=word;
        isBatchDone(id);
    };
}

function createTranscriptionSelector(id,wordImg,ngramImg,possibilities)
{
    
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('transcription');
    genDiv.appendChild(wordImg);
    //ngramImg.hidden=true;
    ngramImg.classList.toggle('meat');
    genDiv.appendChild(ngramImg);
    genDiv.id='b'+id;
    var selectionDiv = document.createElement("div");
    selectionDiv.classList.toggle('selections');
    selectionDiv.classList.toggle('meat');
    //selectionDiv.hidden=true;
    for (var word of possibilities) {
        var wordDiv = document.createElement("div");
        wordDiv.classList.toggle('selection');
        wordDiv.innerHTML=word;
        wordDiv.addEventListener('mouseup', classify(id,word), false);
        selectionDiv.appendChild(wordDiv);
    }
    genDiv.appendChild(selectionDiv);

    return genDiv;
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
