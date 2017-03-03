
var menuHeaderHeight=50;
var removeNgramButtonWidth=40;
var maxImgWidth=700;
var OK_THRESH=85;
var BAD_THRESH=-85;

var toBeInQueue=4;

var batches={};
var batchQueue=[]
var lastRemovedBatchInfo=[];
var lastRemovedEle=[];

var spinner;
var ondeck;
var theWindow;
var gradient;
var icons;
var showX, showCheck;
var instructions;
var instructionsText;

var swipeOn=true;
var tapMode=false;

var screenHeight;
var screenWidth;
var imgWidth=null;

var countUndos=0;
var lastUndo=false;
var allReceived=false;
var startTime = new Date().getTime();

var testingNum=0;
var trainingNum=0;

//pink,green, orange, pink, blue
var headerColors = ['#E0B0FF','#7fb13d','#CD7F32','#C5908E','#95B9C7'];
var spottingColors = ['#f3e5fb','#cad6bb','#eedece','#e7d6d5','#d7e6ec'];
var ondeckColors = ['#eed3ff','#b3d389','#ddb185','#dfb8b7','#bbcbd1'];
var colorIndex=0;

function menu() {
    var m = document.getElementById('menu');
    m.classList.toggle('hidden');
    m.classList.toggle('show');
}

function standardQuery(batchInfo) {
    var query='';
    if (trainingMode)
        query+='&trainingNum='+batchInfo.trainingNum;
    else if (timingTestMode)
        query+='&testingNum='+batchInfo.testingNum;
    if (save)
        query+='&save=1';
    return query;
}


function httpGetAsync(theUrl, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
        //else if (xmlHttp.readyState == 4)
        //    callback(xmlHttp.status);
            
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


function batchShiftAndSend(batchId,callback) {
    
    if (trainingMode) {
        swipeRightGif.hidden=true;
        swipeLeftGif.hidden=true;
        tapGif.hidden=true;
        swipeRightGif.toShow=false;
        swipeLeftGif.toShow=false;
        tapGif.toShow=false;
    }

    var info = batchQueue[0];
    if (info.type+info.id != batchId) {// && info.id != batchId.substr(1)) {
        console.log("ERROR, not matching ids: "+info.type+info.id+" "+batchId);
        console.log(batchQueue);
    }
    batchQueue=batchQueue.slice(1);
    if (batchQueue.length>0) {
        if (timingTestMode) {
            //This time preserves even if the user undos, thus giving a total time for completion
            batchQueue[0].startTime=new Date().getTime();
        }   
        var next = document.getElementById('s'+batchQueue[0].id);
        if (next)
            next.hidden=false;
            //document.getElementById('b'+batchQueue[0].id).classList.toggle('show');
    } else {
        
        spinner.hidden=false;
    }
    lastRemovedBatchInfo.push(info);

    if (info.type=='s') {
        var ngram = info.ngram;
        var resultsId = info.rid;
        
        
        var query='?type=spottings&resend='+batches[batchId].sent;
        query+=standardQuery(info);
        var labels = [];
        var ids = [];
        for (var id in batches[batchId].spottings){
            if (batches[batchId].spottings.hasOwnProperty(id)) {
                var label=batches[batchId].spottings[id];
                ids.push(id);
                labels.push(label);
            }
        }
        var toSend = {batchId:info.id,resultsId:resultsId,labels:labels,ids:ids}; //,undos:countUndos};
        if (timingTestMode) {
            toSend.ngram=ngram;
            toSend.prevNgramSame=false;
            if (lastRemovedBatchInfo.length>1 && lastRemovedBatchInfo[lastRemovedBatchInfo.length-2].ngram==ngram)
                toSend.prevNgramSame=true;
            toSend.batchTime=new Date().getTime()-info.startTime;
            toSend.undos=info.countUndos;
        }
        httpPostAsync('/app/submitBatch'+query,toSend,function (res){
            
            var jres=JSON.parse(res);
            //console.log(jres);
            if (timingTestMode && allReceived && !trainingMode) {
                if (batchQueue.length==0)
                    window.location.href = "/thankyou";
            } else (!timingTestMode || !allReceived)
                callback();
        });
        batches[batchId].sent=true;
    } else if (info.type=='t' || info.type=='m') {
        var query='?type=transcription';
        if (info.type=='m') {
            query+='Manual';
        }
        query+=standardQuery(info);
        var toSend = {batchId:info.id,label:info.transcription};
        if (timingTestMode && info.type=='t') {
            toSend.prevWasTrans=false;
            if (lastRemovedBatchInfo.length>1 && lastRemovedBatchInfo[lastRemovedBatchInfo.length-2].type=='t')
                toSend.prevWasTrans=true;
            toSend.numPossible=info.numPossible;
            toSend.positionCorrect=info.positionCorrect;
            toSend.batchTime=new Date().getTime()-info.startTime;
            toSend.undos=info.countUndos;
        } else if (timingTestMode && info.type=='m') {
            toSend.prevWasManual=false;
            if (lastRemovedBatchInfo.length>1 && lastRemovedBatchInfo[lastRemovedBatchInfo.length-2].type=='m')
                toSend.prevWasManual=true;
            toSend.batchTime=new Date().getTime()-info.startTime;
            toSend.undos=info.countUndos;
        }
        httpPostAsync('/app/submitBatch'+query,toSend,function (res){
            
            var jres=JSON.parse(res);
            //console.log(jres);
            if (timingTestMode && allReceived && !trainingMode) {
                if (batchQueue.length==0)
                    window.location.href = "/thankyou";
            } else (!timingTestMode || !allReceived)
                callback();
        });

    } else if (info.type=='e') {
        
        var query='?type=newExemplars&resend='+batches[batchId].sent;
        query+=standardQuery(info);
        var labels = [];
        for (var id in batches[batchId].spottings){
            if (batches[batchId].spottings.hasOwnProperty(id)) {
                var label=batches[batchId].spottings[id];
                labels.push(label);
            }
        }
        var toSend = {batchId:info.id,labels:labels};
        httpPostAsync('/app/submitBatch'+query,toSend,function (res){
            
            var jres=JSON.parse(res);
            //console.log(jres);
            if (timingTestMode && allReceived && !trainingMode) {
                if (batchQueue.length==0)
                    window.location.href = "/thankyou";
            } else (!timingTestMode || !allReceived)
                callback();
        });
        batches[batchId].sent=true;
    }
}



function handleTouchStart(evt) {
    if (evt.touches)                                     
        this.xDown = evt.touches[0].clientX;  
    else
           this.xDown = evt.clientX;                                 
         
    if (swipeOn) {
        theWindow.classList.remove('fadeGray');
        showX.classList.remove('fadeOut');
        showCheck.classList.remove('fadeOut');
    }
};


function handleTouchMove(evt) {
    if ( ! this.xDown || !swipeOn || tapMode) {
        return;
    }
    if (!ondeck)
        highlightLast();
    
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
        showX.style.opacity='1';
        showCheck.style.opacity='0';
    } else if (this.xDiff<0) {
        theWindow.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        showX.style.opacity=''+.7*(this.xDiff/(0.0+BAD_THRESH));
        showCheck.style.opacity='0';
    } else if (this.xDiff>OK_THRESH) {
        theWindow.style.background='hsl(130,100%,30%)';
        showCheck.style.opacity='1';
        showX.style.opacity='0';
    } else if (this.xDiff>0) {
        theWindow.style.background='hsl(130,'+(60*(this.xDiff/(0.0+OK_THRESH)))+'%,30%)';
        showCheck.style.opacity=''+.7*(this.xDiff/(0.0+OK_THRESH));
        showX.style.opacity='0';
    }                           
};


function handleTouchEnd(evt) {
    if (!swipeOn)
        return;

    evt.stopPropagation();
    evt.preventDefault();
    if (tapMode) {
        var dif = this.xDown-(screenWidth/2);

        if (dif<BAD_THRESH) {
            classifySpotting(false);
        } else if (dif>OK_THRESH) {
            classifySpotting(true);
        }
    } else {
        //var xUp = evt.touches[0].clientX;    
        //this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
        theWindow.classList.add('fadeGray');
        showX.classList.add('fadeOut');
        showCheck.classList.add('fadeOut');
        theWindow.style.background='hsl(350,0%,35%)';
        showX.style.opacity='0';
        showCheck.style.opacity='0';
        
        this.xDown=null;
        ondeck.style.left = '0px';
        //var xDiff = this.xDown - xUp;
        if (this.xDiff>OK_THRESH) {
            removeSpotting(true);
            
        } else if (this.xDiff<BAD_THRESH) {
            removeSpotting(false);
        } else {
        
            
        }
        this.xDiff=0;
    }
}

function classifySpotting(yes) {
    theWindow.classList.remove('fadeGray');
    showX.classList.remove('fadeOut');
    showCheck.classList.remove('fadeOut');

    //setTimeout(function() {
        if (!yes) {
            theWindow.style.background='hsl(350,100%,40%)';
            showX.style.opacity='1';
            showCheck.style.opacity='0';
            removeSpotting(false);
        } else {
            theWindow.style.background='hsl(130,100%,30%)';
            showCheck.style.opacity='1';
            showX.style.opacity='0';
            removeSpotting(true);
        }
        setTimeout(function() {
            theWindow.classList.add('fadeGray');
            showX.classList.add('fadeOut');
            showCheck.classList.add('fadeOut');
            theWindow.style.background='hsl(350,0%,35%)';
            showX.style.opacity='0';
            showCheck.style.opacity='0';
        }, 95);
    //}, 15);
}

function handleKeyPress(evt) {
    var charCode = (evt.which) ? evt.which : event.keyCode;
    if (swipeOn && (charCode == 37 || charCode == 39)) { //left or right
        classifySpotting(charCode == 39);
    } else if (charCode==40) { //down (undo)
        undo();
    } else if (charCode==38) { //up (skip)
        pass();
    } 
}

function removeSpotting(OK) {
    if (!ondeck)
        highlightLast();
    lastRemovedEle.push(ondeck);
    
    if (lastRemovedEle.length>10) {
        var removed=lastRemovedEle.shift();
        //console.log('removed, of batch: '+removed.batch);
        if (lastRemovedEle[0].batch!=lastRemovedBatchInfo[0].type+lastRemovedBatchInfo[0].id) {
            //console.log('removing batch info of: '+lastRemovedBatchInfo[0].type+lastRemovedBatchInfo[0].id);
            lastRemovedBatchInfo.shift();
            //console.log(lastRemovedBatchInfo.length)
        }
    }
    
    
    ondeck.classList.toggle('batchEl');
    ondeck.classList.toggle('collapser');
    
    if (batchQueue[0].type=='e') {
        var header = document.getElementById('h'+ondeck.id);
        if (header)
        {
            header.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
            header.addEventListener("animationend", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
            header.classList.toggle('batchHeader');
            header.classList.toggle('collapserH');
        }
    }

    if (batchQueue[0].type=='e' || batchQueue[0].type=='s') {
        lastUndo = ondeck.classList.contains('redo');

    }
    else
        lastUndo=false;
    
    batches[ondeck.batch].spottings[ondeck.id]=OK;

    isBatchDone(ondeck.batch);
    
    highlightLast();
}

function remover(rek) {return function() {icons.removeChild(rek);};}



function createInlineLabel(label) {
    var ret = document.createElement("LABEL");
    ret.innerHTML='redo: '+label;
    //ret.toggle
    return ret;
}

function undo() {
    if (trainingMode) {
        swipeRightGif.hidden=true;
        swipeLeftGif.hidden=true;
        tapGif.hidden=true;
        swipeRightGif.toShow=false;
        swipeLeftGif.toShow=false;
        tapGif.toShow=false;
    }
    if (!ondeck)
        highlightLast();
    if (lastRemovedEle.length>0) {
        //countUndos++;
        ondeck.classList.toggle('ondeck');
        if (batchQueue[0].type=='s') {
            var ondeckHeader = document.getElementById('s'+batchQueue[0].id)
            if (ondeckHeader)
                ondeckHeader.classList.toggle('ondeck');
        } else if (batchQueue[0].type=='e') {
            var ondeckHeader = document.getElementById('h'+ondeck.id)
            if (ondeckHeader)
                ondeckHeader.classList.toggle('ondeck');
        }


        ondeck=lastRemovedEle.pop();

        if (lastRemovedBatchInfo.length>0){
            var pastInfo= ondeck.batch;//lastRemovedBatchInfo[lastRemovedBatchInfo.length-1];
            //console.log(pastInfo +' ?= '+batchQueue[0].type+batchQueue[0].id)
            if (pastInfo!=batchQueue[0].type+batchQueue[0].id) {
                batchQueue = [lastRemovedBatchInfo.pop()].concat(batchQueue);
            }
            
        }   
        batchQueue[0].countUndos++;
        if (ondeck.childNodes.length<2) {
            if (batchQueue[0].type=='s')
                ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngram),ondeck.childNodes[0]);
            else if (batchQueue[0].type=='e')
                ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngrams[ondeck.id]),ondeck.childNodes[0]);
            ondeck.classList.toggle('redo');
            //colorIndex = (++colorIndex)%headerColors.length;
            ondeck.style.background=spottingColors[ondeck.colorIndex];
        }
        ondeck.classList.toggle('batchEl');
        ondeck.classList.toggle('collapser');
        typeSetup(ondeck);
        if (ondeck.classList.contains('spotting'))
            batches[ondeck.batch].spottings[ondeck.id]=null;
        //console.log(batches[ondeck.batch].spottings);
        if (ondeck.batch!=batchQueue[0].type+batchQueue[0].id)
                console.log('WARNING, batchQueue head not mathcing ondeck: '+ondeck.batch+' '+batchQueue[0].type+batchQueue[0].id)
        theWindow.appendChild(ondeck);
    }
    
}


function getRandomX() {
    return 10 + Math.floor(Math.random()*180);
}
function getRandomY(off) {
    return off + Math.floor(Math.random()*180);
}
function getRandomTime() {
    return 100 + Math.floor(Math.random()*320);
}

function setup() {
    //from http://stackoverflow.com/a/30811794/1018830
    //Prevents swipe navigation on browers that to that
    window.addEventListener( "touchmove", function ( event ) {
        var touch = event.originalEvent.touches[ 0 ];
        oldX = xPos;
        oldY = yPos;
        xPos = touch.pageX;
        yPos = touch.pageY;
        if ( oldX == null && oldY == null ) {
            return false;
        }
        else {
            if ( Math.abs( oldX-xPos ) > Math.abs( oldY-yPos ) ) {
                event.preventDefault();
                return false;
            }
        }
    } );
    window.addEventListener("keydown", handleKeyPress);
    spinner = document.getElementById("spinner");
    var windows = document.getElementsByClassName('window');
    theWindow=windows[0];
    gradient = document.getElementById("overlay");
    icons = document.getElementById("icons");
    showX = document.getElementById("showX");
    showCheck = document.getElementById("showCheck");
    if (trainingMode) {
        swipeRightGif = document.getElementById("swipeRightGif");
        swipeleftGif = document.getElementById("swipeLeftGif");
        tapGif = document.getElementById("tapGif");
        swipeRightGif.toShow=true;
        instructions=document.getElementById('instructions'); 
        instructionsText=document.getElementById('instructionsText');
    } 
    
    var containers = document.getElementsByClassName('container');
    initSlider(containers[0]);

    var w = window,
    d = document,
    e = d.documentElement,
    g = d.getElementsByTagName('body')[0],
    x = w.innerWidth || e.clientWidth || g.clientWidth,
    y = w.innerHeight|| e.clientHeight|| g.clientHeight;
    screenHeight=y;
    screenWidth=x;
    if (screenHeight>700)
        toBeInQueue=7;
    else
        toBeInQueue=4;
    while (!imgWidth)
        imgWidth=Math.min(x,maxImgWidth);
    //console.log(imgWidth);
    window.onresize=function(){
        var e = d.documentElement,
        g = d.getElementsByTagName('body')[0],
        x = w.innerWidth || e.clientWidth || g.clientWidth,
        y = w.innerHeight|| e.clientHeight|| g.clientHeight;
        var dif = y-screenHeight;
        screenHeight=y;
        screenWidth=x;
        if (screenHeight>700)
            toBeInQueue=7;
        else
            toBeInQueue=4;
        imgWidth=Math.min(x,maxImgWidth);
        //console.log('resize: '+x+', '+y);
        //SHould this be for all transcriptions, and not just ondeck?
        var trans = theWindow.getElementsByClassName('transcription');
        for (var i=0; i<trans.length; i++) {
            trans[i].style['max-height']=(screenHeight-menuHeaderHeight)+'px';
            selectionsDiv = trans[i].getElementsByClassName('selections')[0];
            selectionsDiv.height += dif;
            selectionsDiv.style.height = selectionsDiv.height+'px';
        }
    };
    getNextBatch();


    // create an observer instance
    var observer = new MutationObserver(function(mutations) {
      mutations.forEach(function(mutation) {
        //console.log(mutation.type);
        for (var n of mutation.addedNodes) {
            var off=80;
            if (lastUndo)
                off=0;
            setTimeout( function() {
            if (n.classList.contains('red')) {
                n.style.left=getRandomX()+'px';
                n.style.bottom=getRandomY(off)+'px';
            } else {
                n.style.right=getRandomX()+'px';
                n.style.bottom=getRandomY(off)+'px';
            }
            },10);
         }
       
      });    
    });

    // configuration of the observer:
    var config = { attributes: false, childList: true, characterData: true }

    // pass in the target node, as well as the observer options
    observer.observe(icons, config);
    
    
    if (trainingMode) {
        instructions.ready=true;
        instructions.addEventListener('mouseup', closeInstructions, false);
        //instructions.addEventListener('touchstart', closeInstructions, false);
    }


}

function closeInstructions(e) {
            e.preventDefault(); 
            e.stopPropagation();
            if (instructions.ready) {

                startTime = new Date().getTime();
                //this.parentNode.removeChild(this);
                this.hidden=true;
                this.style.display='none';

                if (swipeRightGif.toShow)
                    swipeRightGif.hidden=false;
                if (swipeLeftGif.toShow)
                    swipeLeftGif.hidden=false;
                if (tapGif.toShow)
                    tapGif.hidden=false;

                if (batchQueue[0].type=='m') {
                    var input = document.getElementById('in_'+ondeck.batch);
                    input.focus();
                }
                instructions.ready=false;
            }
}



function createSlider(im,id,batchId) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('spotting');
    genDiv.classList.toggle('batchEl');
    genDiv.appendChild(im);
    genDiv.id=id;
    genDiv.batch=batchId;
    //initSlider(genDiv);
    genDiv.style.background=spottingColors[colorIndex];
    genDiv.colorIndex=colorIndex;
    genDiv.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapse') theWindow.removeChild(this);}, false);
    genDiv.addEventListener("animationend", function(e){if(e.animationName=='collapse') theWindow.removeChild(this);}, false);
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







var batchQueue=[]
//var lastNgram='';

function handleSpottingsBatch(jres) {
    //console.log("got batch "+jres.batchId);
    batches['s'+jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};

    
    var lastBatch = batchQueue[batchQueue.length-1]; 
    if (batchQueue.length>0 && jres.ngram == lastBatch.ngram) {// || 
                //lastBatch.type=='e' && batches[lastBatch.type+lastBatch.id].) {
        //batchHeader.hidden=true
        var lastHeader = document.getElementById(lastBatch.type+lastBatch.id);
        lastHeader.id='s'+jres.batchId;
    
    } else {
        colorIndex = (++colorIndex)%headerColors.length;
        lastNgram=jres.ngram;


        var batchHeader = document.createElement("div");
        //batchHeader.classList.toggle('spotting');
        batchHeader.classList.toggle('batchHeader');
        batchHeader.id='s'+jres.batchId;
        batchHeader.classList.toggle('s'+jres.batchId);
        batchHeader.innerHTML='<div>'+jres.ngram+'</div>';
        batchHeader.style.background=headerColors[colorIndex];
        theWindow.insertBefore(batchHeader,theWindow.childNodes[0]);
    }
    var debugInfo;
    if (jres.debug!=undefined)
        debugInfo = jres.debug;
    else
        debugInfo={precAtPull:-2, precAcceptT:-2, precRejectT:-2, precBetweenT:-2};    
    if (jres.resultsId!=='X') {
        batchQueue.push({type:'s', ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId, debugInfo:debugInfo});
        //console.log("got "+jres.resultsId)
    } else if (jres.batchId=='R') {
        location.reload(true);
    } else {
        allReceived=true;
        //console.log("all recieved")
    }
    //console.log('gott '+allReceived);
    
    for (var index=0; index<jres.spottings.length; index++) {
        var i=jres.spottings[index];
        var im = new Image();
        im.src='data:image/png;base64,'+i.data;
        var widget = createSlider(im,i.id,'s'+jres.batchId);
        theWindow.insertBefore(widget,theWindow.childNodes[0]);
        //initSlider(widget);
        batches['s'+jres.batchId].spottings[i.id]=null;
    }
    spinner.hidden=true;
}

function handleTranscriptionBatch(jres) {
    //batches[jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};
        
    if (jres.batchId!=='R' && jres.batchId!=='X') {
        batchQueue.push({type:'t', id:jres.batchId, transcription:'#'});
        if (timingTestMode) {
            batchQueue[batchQueue.length-1].numPossible=jres.possibilities.length;
            batchQueue[batchQueue.length-1].positionCorrect=jres.possibilities.indexOf(jres.correct.toLowerCase());
            //console.log('correct: '+jres.correct.toLowerCase());
            //console.log(jres.possibilities);
        }
        //console.log("got "+jres.resultsId)
    } else if (jres.batchId=='R') {
        location.reload(true);
    } 
    
    var wordImg = new Image();
    wordImg.src='data:image/png;base64,'+jres.wordImg;

    theWindow.insertBefore(createTranscriptionSelector('t'+jres.batchId,wordImg,jres.ngrams,jres.possibilities),theWindow.childNodes[0]);
    spinner.hidden=true;
}
function handleManualBatch(jres) {
        
    if (jres.batchId!=='R' && jres.batchId!=='X') {
        batchQueue.push({type:'m', id:jres.batchId, transcription:'#'});
    } else if (jres.batchId=='R') {
        location.reload(true);
    } 
    
    var wordImg = new Image();
    wordImg.src='data:image/png;base64,'+jres.wordImg;
    theWindow.insertBefore(createManualTranscriptionSelector('m'+jres.batchId,wordImg,jres.ngrams,jres.possibilities),theWindow.childNodes[0]);
    spinner.hidden=true;
}


function handleNewExemplarsBatch(jres) {
    //console.log("got batch "+jres.batchId);
    
        //console.log("got "+jres.resultsId)
    if (jres.batchId=='R') {
        location.reload(true);
    } else if (jres.batchId!=='R' && jres.batchId!=='X') {
        batchQueue.push({type:'e', id:jres.batchId });
        batches['e'+jres.batchId]={sent:false, spottings:{}, ngrams:{}};
        
        
        for (var index=0; index<jres.exemplars.length; index++) {
            var i=jres.exemplars[index];
            var id = index+'e'+jres.batchId;
            
            var batchHeader = document.createElement("div");
            batchHeader.classList.toggle('batchHeader');
            batchHeader.id='h'+id
            batchHeader.innerHTML='<div>'+i.ngram+'</div>';
            colorIndex = (++colorIndex)%headerColors.length;
            lastNgram=i.ngram;
            batchHeader.style.background=headerColors[colorIndex];
            theWindow.insertBefore(batchHeader,theWindow.childNodes[0]);
            
            var im = new Image();
            im.src='data:image/png;base64,'+i.data;
            var widget = createSlider(im,id,'e'+jres.batchId);
            theWindow.insertBefore(widget,theWindow.childNodes[0]);
            batches['e'+jres.batchId].spottings[id]=null;
            batches['e'+jres.batchId].ngrams[id]=i.ngram;
        }
        spinner.hidden=true;
    }
}

function getNextBatch() { 
    //console.log('getting '+allReceived);
    if (allReceived) {
        console.log('ERROR allrecieved, but sending for more')
        return
    }
    var query='';
    var prevNgram='~';
    var currentNum;
    //if (batchQueue.length>0)
    //    prevNgram=batchQueue[batchQueue.length-1].ngram;

    if (trainingMode) {
        currentNum=trainingNum;
        query+='&trainingNum='+trainingNum;
        trainingNum+=1;
    }
    else if (timingTestMode) {
        currentNum=testingNum;
        query+='&testingNum='+testingNum;
        testingNum+=1;
    }

    if (save)
        query+='&save=1';

    httpGetAsync('/app/nextBatch?width='+imgWidth+'&color='+colorIndex+'&prevNgram='+prevNgram+query,function (res){
        var jres=JSON.parse(res);
        var fromEmpty = batchQueue.length==0;
        if (jres.err==null) {
            var cont = false;
            if (jres.batchType=='spottings') {
                handleSpottingsBatch(jres);                
                cont=true;
            } else if (jres.batchType=='transcription') {
                handleTranscriptionBatch(jres);
                cont=true;
            } else if (jres.batchType=='newExemplars') {
                handleNewExemplarsBatch(jres);
                cont=true;
            } else if (jres.batchType=='manual') {
                handleManualBatch(jres);
                cont=true;
            } else if (jres.batchType=='EMPTY' && timingTestMode) {
                allReceived=true;
            }
            else {
                console.log('bad batch '+jres.batchType);
            }

            if (jres.batchType!='EMPTY') {
                if (trainingMode) {
                    batchQueue[batchQueue.length-1].instructions=jres.instructions;
                    batchQueue[batchQueue.length-1].lastTraining=jres.lastTraining;
                    batchQueue[batchQueue.length-1].correct=jres.correct.toLowerCase();
                    batchQueue[batchQueue.length-1].trainingNum=currentNum;

                    if (fromEmpty)
                        instructionsText.innerHTML=jres.instructions;
                    if (jres.lastTraining)
                        theWindow.children[0].classList.add('starter');
                } else if (timingTestMode) {
                    batchQueue[batchQueue.length-1].countUndos=0;
                    batchQueue[batchQueue.length-1].startTime=null;
                    batchQueue[batchQueue.length-1].testingNum=currentNum;
                    if (fromEmpty) {
                        batchQueue[batchQueue.length-1].startTime=new Date().getTime();
                    }
                }
            }
            if (batchQueue.length<toBeInQueue && cont && !allReceived)
                getNextBatch(); //callback);

            if (fromEmpty)
                highlightLast();
            
            
        } else {
            console.log('nextBatch ERROR: '+jres.err);
        }
        
        
    });
}

function highlightLast() {
    var spottings = theWindow.getElementsByClassName('batchEl');
    ondeck=spottings[spottings.length-1];
    if (ondeck) {
        ondeck.classList.toggle('ondeck');
        typeSetup(ondeck);
        if (batchQueue[0].type=='e') {
            var header = document.getElementById('h'+ondeck.id);
            if (header && !header.classList.contains('ondeck'))
                header.classList.toggle('ondeck');
        }
        else if (batchQueue[0].type=='s') {
            var header_s = document.getElementsByClassName(ondeck.batch);
            if (header_s && header_s.length>0 &&!header_s[0].classList.contains('ondeck'))
                header_s[0].classList.toggle('ondeck');
            console.log('Batch SpottingResults info:');
            console.log('Prec at pull:   \t'+batchQueue[0].debugInfo.precAtPull);
            console.log('Prec accept['+batchQueue[0].debugInfo.countAcceptT+']:\t'+batchQueue[0].debugInfo.precAcceptT);
            console.log('Prec between['+batchQueue[0].debugInfo.countBetweenT+']:\t'+batchQueue[0].debugInfo.precBetweenT);
            console.log('Prec reject['+batchQueue[0].debugInfo.countRejectT+']:\t'+batchQueue[0].debugInfo.precRejectT);
            //console.log(batchQueue[0].debugInfo);
            
        }
        else if (batchQueue[0].type=='m') {
            if (!trainingMode) {
                var input = document.getElementById('in_'+ondeck.batch);
                input.focus();
            }
        }
    }
}

function typeSetup(ele) {
    if (ele.classList.contains('spotting')) {
        ele.style.background=ondeckColors[ele.colorIndex];
        gradient.hidden=false;
        icons.hidden=false;
        swipeOn=true;
    } else {
        gradient.hidden=true;
        icons.hidden=true;
        swipeOn=false;
    }
}

function isBatchDone(batchId) {
    
    if (batchQueue[0].type == 's' || batchQueue[0].type == 'e')
        for (spottingId in batches[batchId].spottings)
            if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==null)
                return;
    
    //base
    batchShiftAndSend(batchId,getNextBatch);
    //base
    var oldElement = document.getElementById(batchId);
    if (oldElement) {
        oldElement.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
        oldElement.addEventListener("animationend", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
        oldElement.classList.toggle('batchHeader');
        oldElement.classList.toggle('collapserH');
    }

    if (trainingMode && batchQueue.length>0) {
        var right=false;
        if (lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].type=='s') {
            for (spottingId in batches[batchId].spottings) {
                if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].correct) {
                    right = true;
                }
            }
        } else if (lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].type=='t' || lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].type=='m') {
                right =  lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].transcription.toLowerCase() == lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].correct;
        }
        
        if (batchQueue[0].lastTraining) {
            trainingMode=false;
            allReceived=false;
            batchQueue[0].type='x';
            document.getElementById('instructionsButton').hidden=true;
        }
        if (right || batchQueue[0].lastTraining) {
            if (batchQueue[0].instructions.length>0) {
                instructionsText.innerHTML=batchQueue[0].instructions;
                instructions.hidden=false;
                instructions.style.display='flex';
                setTimeout(function(){instructions.ready=true;},100);
            }


            //This is ridged, but was easy
            if (batchQueue[0].trainingNum==0) {
                swipeRightGif.toShow=true;
            } else if (batchQueue[0].trainingNum==1) {
                //swipeLeftGif.toShow=true;
                swipeLeftGif.hidden=false;
            } else if (batchQueue[0].trainingNum==8) {
                tapGif.toShow=true;
            }
        } else {
            instructionsText.innerHTML='<p>Opps, that was incorrect. :(</p><p>Use the <b>undo</b> button to try again.</p>';

            instructions.hidden=false;
            instructions.style.display='flex';
            setTimeout(function(){instructions.ready=true;},100);
        }
    }
}

function classify(id,word) {
    return classifyR(id,function(){return word;});
}
function classifyR(id,vFunc) {
    return function(ele) {
        if (!ondeck)
            highlightLast();
        lastRemovedEle.push(ondeck);
        
        if (lastRemovedEle.length>10) {
            var removed=lastRemovedEle.shift();
            //console.log('removed, of batch: '+removed.batch);
            if (lastRemovedEle[0].batch!=lastRemovedBatchInfo[0].type+lastRemovedBatchInfo[0].id) {
                //console.log('removing batch info of: '+lastRemovedBatchInfo[0].type+lastRemovedBatchInfo[0].id);
                lastRemovedBatchInfo.shift();
                //console.log(lastRemovedBatchInfo.length)
            }
        }
        if (batchQueue[0].type!='t' && batchQueue[0].type!='m') {
            console.log('ERROR, wrong inst in batchQueue');
            console.log(batchQueue);
        }
        batchQueue[0].transcription=vFunc();
        ondeck.classList.toggle('batchEl');
        ondeck.classList.toggle('collapser');
        isBatchDone(id);
        highlightLast();
    };
}

function adjustChain(chain) {
    if (chain.length>1) {
        //console.log('adjust chain:');
        //console.log(chain);
        var mid=0;
        for (var c=0; c<chain.length; c++) {
            mid+=chain[c].x;
        }
        mid/=chain.length;
        for (var c=0; c<chain.length; c++) {
            var fromCenter=c-((chain.length-1)/2.0);
            chain[c].x=mid + fromCenter*removeNgramButtonWidth;
        }
        //console.log(mid);
        //console.log(chain);
    }
}

function makeTranscriptionDiv(id,wordImg,ngrams) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('transcription');
    genDiv.classList.toggle('batchEl');
    genDiv.appendChild(wordImg);
    if (ngrams.length>0) {
        for (var i=0; i<ngrams.length; i++) {
            //if (!ngrams[i].scale)
            //    ngrams[i].scale=1;
            ngrams[i].x=Math.floor(parseInt(ngrams[i].x)*ngrams[i].scale);
        }

        //The remove buttons are too hard to press if they're close together
        //This spaces them out.
        var chain=[ngrams[0]];
        for (var i=1; i<ngrams.length; i++) {
            if (ngrams[i].x-ngrams[i-1].x < removeNgramButtonWidth) {
                chain.push(ngrams[i]);
            }
            else
            {
                adjustChain(chain);

                chain=[ngrams[i]];
            }
        }
        adjustChain(chain);

        var ngramDiv = document.createElement("div");
        ngramDiv.classList.toggle('meat');
        ngramDiv.classList.toggle('spotteds');
        for (var i=0; i<ngrams.length; i++) {
            var ngramP = ngrams[i];
            var d = document.createElement("div");
            d.classList.toggle('spotted');
            d.innerHTML=ngramP.ngram+'<br>';
            d.style.color='#'+ngramP.color;
            d.style.left=ngramP.x+'px';
            var closeImg = new Image();
            closeImg.src='/removeNgram.png';
            d.appendChild(closeImg);
            d.addEventListener('mouseup', classify(id,'$REMOVE:'+ngramP.id+'$'), false);
            //d.addEventListener('touchend', classify(id,'$REMOVE:'+ngramP.id+'$'), false);
            ngramDiv.appendChild(d);
        }
        genDiv.appendChild(ngramDiv);
    }
    genDiv.id='bt'+id;
    genDiv.batch=id;
    genDiv.style['max-height']=(screenHeight-menuHeaderHeight)+'px';
    genDiv.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapse') theWindow.removeChild(this);}, false);
    genDiv.addEventListener("animationend", function(e){if(e.animationName=='collapse') theWindow.removeChild(this);}, false);
    return genDiv;
}


//autocomplete
//assumes possibilities is sorted lexigraphically, case-insensitive
function inputBoxFunc(self,possibilities,possDiv,id,onEnter) {
    var limit = 20;
    var lcPossibilities=[];
    for (var i=0; i<possibilities.length; i++) {
        lcPossibilities.push(possibilities[i].toLowerCase());
    }
    var ret = function(event) {
        //console.log(event.keyCode);
        if (event.keyCode == 13) {
            event.preventDefault();
            onEnter();
        }
        else if ((event.keyCode>=65 && event.keyCode<=90) || event.keyCode==8 || event.keyCode==46 || event.keyCode==32 || event.keyCode==229 || event.keyCode==0)
        {
            var cur = self.value.toLowerCase();
            if (cur.length>0) {
                //binary search for starting index
                var lb=0;
                var ub=lcPossibilities.length;
                var i;
                while (true) {
                    i=Math.floor(lb +(ub-lb)/2);
                    var comp = cur.localeCompare(lcPossibilities[i]);
                    console.log(i +' '+comp);
                    if (comp==0)
                        break;
                    else if (comp<0)
                        ub=i;
                    else
                        lb=i+1;
                    if (ub-lb < 2) {
                        if (comp<0 && cur.localeCompare(lcPossibilities[lb])<=0)
                            i=lb;
                        else
                            i=ub;
                        break;
                    }
                }
                //console.log('Found starting index: '+i);
                //var comp = cur.localeCompare(lcPossibilities[i]);
                //if (comp>0)
                //    i+=1;
                    
                var matchList = [];
                for (; i<possibilities.length; i++) {
                    if (cur.length<=possibilities[i].length) {
                        match=true;
                        for (var c=0; c<cur.length; c++) {
                            if (cur[c] != lcPossibilities[i][c]) {
                                match=false;
                            }
                        }
                        //console.log(lcPossibilities[i]+' '+match);
                        if (match) {
                            matchList.push(possibilities[i]);
                            if (matchList.length>=limit)
                                break;
                        }
                        else
                            break; //because its sorted
                    }
                }
                //console.log('matchList:');
                //console.log(matchList);
                if (matchList.length<limit) {
                    //console.log('adding:')
                    //console.log(matchList)
                    possDiv.innerHTML='';//clear
                    addWordButtons(possDiv,matchList,id);
                }
            }
        }
    };
    return ret;
}

//I'm going to make the assumption that possibilites is sorted lexigraphically, case-insensitive
function createManualTranscriptionSelector(id,wordImg,ngrams,possibilities) {
    var genDiv = makeTranscriptionDiv(id,wordImg,[]);//ngrams); The ngrams panel makes the window too short when typing on mobile keyboard
    
    var totalHeight = wordImg.height 
    //if (ngrams.length>0)
    //    totalHeight += 75;//ngramDiv.offsetHeight;//ngramImg.height;
    var possDiv = document.createElement("div");
    possDiv.classList.toggle('selections');
    possDiv.classList.toggle('meat');
    //type box
    var input = document.createElement('input');
    input.id='in_'+id;
    input.autocomplete="off"
    input.classList.toggle('transInput');
    input.type = "text";
    input.onkeyup = inputBoxFunc(input,possibilities,possDiv,id,classifyR(id,function(){return input.value;}));
    //submit button
    var submit = document.createElement("div");
    submit.classList.toggle('transSubmit');
    submit.innerHTML='&gt;';
    submit.addEventListener('mouseup', classifyR(id,function(){return input.value;}), false);
    //submit.addEventListener('touchstart', classifyR(id,function(){return input.value;}), false);
    
    genDiv.appendChild(input);
    genDiv.appendChild(submit);
    genDiv.appendChild(possDiv);
    possDiv.height = (screenHeight - totalHeight - menuHeaderHeight - Math.max(40,submit.offsetHeight,input.offsetHeight));
    possDiv.style.height = possDiv.height+'px';
    //error button
    /*var errDiv = document.createElement("div");
    errDiv.classList.toggle('selection');
    errDiv.classList.toggle('error');
    errDiv.innerHTML="<div>Error (something's not right)</div>";
    errDiv.addEventListener('mouseup', classify(id,'$ERROR$'), false);
    errDiv.addEventListener('touchup', classify(id,'$ERROR$'), false);
    genDiv.appendChild(errDiv);
    */
    return genDiv;
}

function addWordButtons(dest,words,id) {
    for (var i=0; i<words.length; i++) {
        var wordDiv = document.createElement("div");
        wordDiv.classList.toggle('selection');
        wordDiv.innerHTML='<div>'+words[i]+'</div>';
        wordDiv.addEventListener('mouseup', classify(id,words[i]), false);
        //wordDiv.addEventListener('touchstart', classify(id,words[i]), false);
        dest.appendChild(wordDiv);
    }
}

function createTranscriptionSelector(id,wordImg,ngrams,possibilities) {
    
    var genDiv = makeTranscriptionDiv(id,wordImg,ngrams);

    var totalHeight = wordImg.height + 75;//ngramDiv.offsetHeight;//ngramImg.height;
    var selectionDiv = document.createElement("div");
    selectionDiv.classList.toggle('selections');
    selectionDiv.classList.toggle('meat');
    selectionDiv.style.height = (screenHeight - totalHeight - menuHeaderHeight)+'px';
    addWordButtons(selectionDiv,possibilities,id);


    var errDiv = document.createElement("div");
    errDiv.classList.toggle('selection');
    errDiv.classList.toggle('error');
    errDiv.innerHTML="<div>[None / Error]</div>";
    errDiv.addEventListener('mouseup', classify(id,'$ERROR$'), false);
    //errDiv.addEventListener('touchstart', classify(id,'$ERROR$'), false);
    selectionDiv.appendChild(errDiv);
    
    genDiv.appendChild(selectionDiv);

    return genDiv;
}

function pass() {
    if (!ondeck)
        highlightLast();
    if (ondeck.classList.contains('transcription')) {
        classify(batchQueue[0].type+batchQueue[0].id,'$PASS$')();
    } else if (ondeck.classList.contains('spotting')) {
        removeSpotting(-1);//-1 means pass to spottingAddon
    }
}

function exit() {
    allReceived=true;
    toBeInQueue=0;
    while (batchQueue.length>0){
        pass();
    }
    history.go(-1);
}

function toggleMode() {
    tapMode=!tapMode;
    document.getElementById("modeButton").classList.toggle("tapModeOn");
    if (tapMode) {
        document.getElementById("tapInstructions").hidden=false;
    } else {
        document.getElementById("tapInstructions").hidden=true;
    }
}

function showInstructions() {
    instructions.hidden=false;
    instructions.style.display='flex';
    setTimeout(function(){instructions.ready=true;},100);
}
/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
