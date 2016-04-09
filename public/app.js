var batches={};
var maxImgWidth=700;
var imgWidth;

var test_batchesToDo=3;

function begin() {
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

function getNextBatch(window,toload) {
    httpGetAsync('/app/nextBatch?width='+imgWidth,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                //console.log("got batch "+jres.batchId);
                batches[jres.batchId]={sent:false, spottings:{}};
                //TODO jres.ngram
                var batchHeader = document.createElement("div");
                batchHeader.classList.toggle('spotting');
                batchHeader.classList.toggle('batchHeader');
                batchHeader.id=jres.batchId
                batchHeader.innerHTML=jres.ngram;
                window.appendChild(batchHeader);
                for (i of jres.spottings) {
                    var im = new Image();
                    im.src='data:image/png;base64,'+i.data;
                    var widget = document.createElement("div");
                    widget.classList.toggle('spotting');
                    widget.appendChild(im);
                    widget.id=i.id;
                    widget.batch=jres.batchId;
                    window.appendChild(widget);
                    initSlider(widget);
                    batches[jres.batchId].spottings[i.id]=null;
                }
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(window,toload);
        }
        
        
    });
}

function isBatchDone(batchId,window) {
    if (!batches[batchId].sent) {
        for (spottingId in batches[batchId].spottings)
            if (batches[batchId].spottings.hasOwnProperty(spottingId) && batches[batchId].spottings[spottingId]==null)
                return;
        batches[batchId].sent=true;
        var header = document.getElementById(batchId);
        window.removeChild(header);
        httpPostAsync('/app/submitBatch',{batchId:batchId,labels:batches[batchId].spottings},function (res){
            //if(--test_batchesToDo > 0) {
                getNextBatch(window);
            //} else {
            //    window.location.href = "/done";
            //}
        });
    } else {
        //TODO handle UNDO case
        assert(false);
    }
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
