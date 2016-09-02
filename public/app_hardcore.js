

var OK_THRESH=85;
var BAD_THRESH=-85;


var lastRemovedEle=[];
var spinner;

var ondeck;
var theWindow;
var gradient;
var icons;

var toBeInQueue=3;
var swipeOn=true;
var screenHeight;

var lastUndo=false;
var menuHeaderHeight=50;

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
    
    
    ondeck.classList.toggle('batchEl');
    ondeck.classList.toggle('collapser');
    
    if (batchQueue[0].type=='e') {
        var header = document.getElementById('h'+ondeck.id);
        header.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
        header.addEventListener("animationend", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
        header.classList.toggle('batchHeader');
        header.classList.toggle('collapserH');
    }

    if (batchQueue[0].type=='e' || batchQueue[0].type=='s') {
        lastUndo = ondeck.classList.contains('redo');
        sparks(OK);

    }
    else
        lastUndo=false;
    
    batches[ondeck.batch].spottings[ondeck.id]=OK;

    isBatchDone(ondeck.batch);
    
    highlightLast();
}

/*function Spark(right) {

        this.spark = document.createElement("DIV");
        spark.classList.toggle('spark');
        if (right)
            spark.classList.toggle('green');
        else
            spark.classList.toggle('red');
        var remove = function(){icons.removeChild(this.spark);}
        setTimeout(remove,getRandomTime());
        icons.appendChild(spark);
}*/
function remover(rek) {return function() {icons.removeChild(rek);};}

function sparks(right) {

    for (var i=0; i<29; i++) {
        var spark = document.createElement("DIV");
        spark.classList.toggle('spark');
        if (right)
            spark.classList.toggle('green');
        else
            spark.classList.toggle('red');

        if  (lastUndo)
            spark.style.bottom='90px';
        /*spark.addEventListener("load", function(event) {
        if (right) {
            spark.style.bottom='100px';
            spark.style.right='100px';
        } else {
            spark.style.bottom='100px';
            spark.style.left='100px';
        }
        });*/
        //TODO webkit
        //spark.addEventListener("animationend", function(e){theWindow.removeChild(this);}, false);
        //spark.addEventListener("transitionend", function(e){if(e.propertyName=='bottom') icons.removeChild(this);}, false);
        
        setTimeout(remover(spark),getRandomTime());
        icons.appendChild(spark);
    }

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
        if (batchQueue[0].type=='s') {
            var ondeckHeader = document.getElementById('b'+batchQueue[0].id)
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
            console.log(pastInfo +' ?= '+batchQueue[0].id)
            if (pastInfo!=batchQueue[0].id) {
                batchQueue = [lastRemovedBatchInfo.pop()].concat(batchQueue);
            }
            
        }   
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        if (ondeck.childNodes.length<2) {
            if (batchQueue[0].type=='s')
                ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngram),ondeck.childNodes[0]);
            else if (batchQueue[0].type=='e')
                ondeck.insertBefore(createInlineLabel(batches[ondeck.batch].ngrams[ondeck.id].ngram),ondeck.childNodes[0]);
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
        if (ondeck.batch!=batchQueue[0].id)
                console.log('ERROR, batchQueue head not mathcing ondeck: '+ondeck.batch+' '+batchQueue[0].id)
        theWindow.appendChild(ondeck);
    }
    
}

function handleTouchEnd(evt) {
    if (!swipeOn)
        return;
    //var xUp = evt.touches[0].clientX;    
    //this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
    
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
    spinner = document.getElementById("spinner");
    var windows = document.getElementsByClassName('window');
    theWindow=windows[0];
    gradient = document.getElementById("overlay");
    icons = document.getElementById("icons");
    
    
    var containers = document.getElementsByClassName('container');
    initSlider(containers[0]);
    /*containers[0].addEventListener('touchstart', function(e){ e.preventDefault(); });
    containers[0].addEventListener('mousedown', function(e){ e.preventDefault(); });
    */
    var w = window,
    d = document,
    e = d.documentElement,
    g = d.getElementsByTagName('body')[0],
    x = w.innerWidth || e.clientWidth || g.clientWidth,
    y = w.innerHeight|| e.clientHeight|| g.clientHeight;
    screenHeight=y;
    while (!imgWidth)
        imgWidth=Math.min(x,maxImgWidth);
    console.log(imgWidth);
    window.onresize=function(){
        var e = d.documentElement,
        g = d.getElementsByTagName('body')[0],
        x = w.innerWidth || e.clientWidth || g.clientWidth,
        y = w.innerHeight|| e.clientHeight|| g.clientHeight;
        var dif = y-screenHeight;
        screenHeight=y;
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
    getNextBatch(toBeInQueue,function() { 
        highlightLast();
        //var headers = theWindow.getElementsByClassName('batchHeader');
        //if (headers.length>0)
        //    headers[headers.length-1].classList.toggle('ondeck');
    });


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
            batchQueue.push({type:'s', ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
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
}

function handleTranscriptionBatch(jres) {
    //batches[jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};
        
    if (jres.batchId!=='R' && jres.batchId!=='X') {
        batchQueue.push({type:'t', id:jres.batchId, transcription:'#'});
        //console.log("got "+jres.resultsId)
    } else if (jres.batchId=='R') {
        location.reload(true);
    } 
    
    var wordImg = new Image();
    wordImg.src='data:image/png;base64,'+jres.wordImg;
    //var ngramImg = new Image();
    //ngramImg.src='data:image/png;base64,'+jres.ngramImg;
    theWindow.insertBefore(createTranscriptionSelector(jres.batchId,wordImg,jres.ngrams,jres.possibilities),theWindow.childNodes[0]);
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
    theWindow.insertBefore(createManualTranscriptionSelector(jres.batchId,wordImg,jres.ngrams,jres.possibilities),theWindow.childNodes[0]);
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
        //colorIndex = (++colorIndex)%headerColors.length;
        //batchHeader.style.background=headerColors[colorIndex];
        
        
        //theWindow.insertBefore(batchHeader,theWindow.childNodes[0]);
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
            //var widget = document.createElement("div");
            //widget.classList.toggle('spotting');
            //widget.appendChild(im);
            //widget.id=i.id;
            //widget.batch=jres.batchId;
            var widget = createSlider(im,id,'e'+jres.batchId);
            theWindow.insertBefore(widget,theWindow.childNodes[0]);
            //initSlider(widget);
            batches['e'+jres.batchId].spottings[id]=null;
            batches['e'+jres.batchId].ngrams[id]=i.ngram;
        }
        spinner.hidden=true;
    }
}

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
                handleSpottingsBatch(jres);                
            } else if (jres.batchType=='transcription') {
                handleTranscriptionBatch(jres);
            } else if (jres.batchType=='newExemplars') {
                handleNewExemplarsBatch(jres);
            } else if (jres.batchType=='manual') {
                handleManualBatch(jres);
            }
            /*if (colorIndex==1)
                colorIndex=-1;
            else
                colorIndex=2;*/
            if (toload!==undefined && --toload>0)
                getNextBatch(toload,callback);
            else if (callback!==undefined)
                callback();
            
            
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
            var header = document.getElementById('b'+ondeck.batch);
            if (header &&!header.classList.contains('ondeck'))
                header.classList.toggle('ondeck');
        }
        else if (batchQueue[0].type=='m') {
            var input = document.getElementById('mt'+ondeck.batch);
            input.focus();
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
    batchShiftAndSend(batchId,function(){if (batchQueue.length<toBeInQueue) getNextBatch();});
    //base
    var nextElement=null;
    if (batchQueue.length>0) {
        if (batchQueue[0].type=='s')
            nextElement=document.getElementById('b'+batchQueue[0].id);
    }
    /*
    if (nextElement) {
        nextElement.classList.toggle('ondeck');
        //if (batchQueue[0].ngram=='#') {
            //TODO hide gradient
            //gradient.hidden=true;
        //}
    }*/
    //highlightLast();
    var oldElement = document.getElementById('b'+batchId);
    if (oldElement) {
        var typeLastRemoved = lastRemovedBatchInfo[lastRemovedBatchInfo.length-1].type;
        if (typeLastRemoved!='t' && typeLastRemoved!='m') {
            if (batchQueue.length > 0 && typeLastRemoved.ngram == batchQueue[0].ngram) {
                theWindow.removeChild(oldElement);
                
                //we shift the header past the collapsing element to provide the illusion of a smooth transition
                if (nextElement) {
                    theWindow.removeChild(nextElement);
                    theWindow.appendChild(nextElement);
                }
            } else {
                oldElement.addEventListener("webkitAnimationEnd", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
                oldElement.addEventListener("animationend", function(e){if(e.animationName=='collapseH') theWindow.removeChild(this);}, false);
                oldElement.classList.toggle('batchHeader');
                oldElement.classList.toggle('collapserH');
            }
        } else {
            //TODO fix gradient
            //gradient.hidden=false;
            /*oldElement.addEventListener("webkitAnimationEnd", function(e){theWindow.removeChild(this);}, false);
            oldElement.addEventListener("animationend", function(e){theWindow.removeChild(this);}, false);
            oldElement.classList.toggle('collapser');*/
        }
    }
}

function classify(id,word) {
    return classifyR(id,function(){return word;});
}
function classifyR(id,vFunc) {
    return function(ele) {
        lastRemovedEle.push(ondeck);
        
        if (lastRemovedEle.length>10) {
            var removed=lastRemovedEle.shift();
            if (lastRemovedEle[0].batch!=lastRemovedBatchInfo[0].id) {
                lastRemovedBatchInfo.shift();
                //console.log(lastRemovedBatchInfo)
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


function makeTranscriptionDiv(id,wordImg,ngrams) {
    var genDiv = document.createElement("div");
    genDiv.classList.toggle('transcription');
    genDiv.classList.toggle('batchEl');
    genDiv.appendChild(wordImg);
    //ngramImg.classList.toggle('meat');
    //genDiv.appendChild(ngramImg);
    if (ngrams.length>0) {
        var ngramDiv = document.createElement("div");
        ngramDiv.classList.toggle('meat');
        ngramDiv.classList.toggle('spotteds');
        for (var ngramP of ngrams) {
            var d = document.createElement("div");
            d.classList.toggle('spotted');
            d.innerHTML=ngramP.ngram+'<br>';
            d.style.color='#'+ngramP.color;
            d.style.left=ngramP.x+'px';
            var closeImg = new Image();
            closeImg.src='/removeNgram.png';
            d.appendChild(closeImg);
            d.addEventListener('mouseup', classify(id,'$REMOVE:'+ngramP.id+'$'), false);
            d.addEventListener('touchup', classify(id,'$REMOVE:'+ngramP.id+'$'), false);
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

//assumes possibilities is sorted lexigraphically, case-insensitive
function inputBoxFunc(self,possibilities,possDiv,id,onEnter) {
    var limit = 20;
    var lcPossibilities=[];
    for (var i=0; i<possibilities.length; i++) {
        lcPossibilities.push(possibilities[i].toLowerCase());
    }
    var ret = function(event) {
        if (event.keyCode == 13) {
            event.preventDefault();
            onEnter();
        }
        else if ((event.keyCode>=65 && event.keyCode<=90) || event.keyCode==8 || event.keyCode==46 || event.keyCode==32)
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
                    if (comp==0)
                        break;
                    else if (comp<0)
                        ub=i;
                    else
                        lb=i+1;
                    if (ub-lb < 2) {
                        if (comp<0)
                            i=lb;
                        else
                            i=ub;
                        break;
                    }
                }
                console.log('Found starting index: '+i);
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
                        if (match) {
                            matchList.push(possibilities[i]);
                            if (matchList.length>=limit)
                                break;
                        }
                        else
                            break; //because its sorted
                    }
                }
                if (matchList.length<limit) {
                    console.log('adding:')
                    console.log(matchList)
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
    input.id='mt'+id;
    input.autocomplete="off"
    input.classList.toggle('transInput');
    input.type = "text";
    input.onkeyup = inputBoxFunc(input,possibilities,possDiv,id,classifyR(id,function(){return input.value;}));
    //submit button
    var submit = document.createElement("div");
    submit.classList.toggle('transSubmit');
    submit.innerHTML='&gt;';
    submit.addEventListener('mouseup', classifyR(id,function(){return input.value;}), false);
    submit.addEventListener('touchup', classifyR(id,function(){return input.value;}), false);
    
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
        wordDiv.addEventListener('touchup', classify(id,words[i]), false);
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
    errDiv.innerHTML="<div>Error (something's not right)</div>";
    errDiv.addEventListener('mouseup', classify(id,'$ERROR$'), false);
    errDiv.addEventListener('touchup', classify(id,'$ERROR$'), false);
    selectionDiv.appendChild(errDiv);
    
    genDiv.appendChild(selectionDiv);

    return genDiv;
}

function pass() {
    if (ondeck.classList.contains('transcription')) {
        classify(batchQueue[0].id,'$PASS$')();
    } else if (ondeck.classList.contains('spotting')) {
        removeSpotting(-1);//-1 means pass to spottingAddon
    }
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
