var OK_THRESH=-100;
var BAD_THRESH=100;

var spinner;
var lastRemoved=[];
var lastRemovedInfo=[];
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
        doneButton.addEventListener('touchend', batchDone, false);
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
    return genDiv;
}

function initTapped(ele) {
    //ele.addEventListener('touchstart', handleTouchStart, false);        
    //ele.addEventListener('touchmove', handleTouchMove, false);
    ele.addEventListener('touchend', handleTouchEnd, false);
    
    //ele.addEventListener('mousedown', handleTouchStart, false);        
    //ele.addEventListener('mousemove', handleTouchMove, false);
    ele.addEventListener('mouseup', handleTouchEnd, false);
    //ele.xDown=null;
}

//////////////////////

var batches={};
var maxImgWidth=700;
var imgWidth;
var numBatch;

var test_batchesToDo=3;

function begin() {
    imgWidth=Math.min(document.defaultView.outerWidth,maxImgWidth);
    numBatch=Math.floor((document.defaultView.outerHeight-document.getElementById('title').offsetHeight-50)/86)-1;
    console.log ('numBatch='+numBatch+' full='+document.defaultView.outerHeight+' top='+document.getElementById('title').offsetHeight);
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



var batchQueue=[]
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
		        
		        batchContainer.appendChild(batchHeader);
                
                for (i of jres.spottings) {
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
                batchQueue.push({ngram:jres.ngram, id:jres.batchId});
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
    console.log(batchId);
    var window = this.parentNode;
    //if (!batches[batchId].sent) {
        batches[batchId].sent=true;
        var info = batchQueue[0];
	    var ngram = info.ngram;
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
