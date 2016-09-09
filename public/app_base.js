var batches={};
var batchQueue=[]
var maxImgWidth=700;
var imgWidth=null;
var allReceived=false;
var countUndos=0;
var startTime = new Date().getTime();
function menu() {
    var m = document.getElementById('menu');
    m.classList.toggle('hidden');
    m.classList.toggle('show');
}
/*var batchQueue=[];
var lastNgram='';
function getNextBatch(window,toload) {
    query='';
    if (testMode)
        query='&test='+testNum; 
    httpGetAsync('/app/nextBatch?num='+numBatch+'&width='+imgWidth+query,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                if (testMode&&jres.resultsId=='X') {
                    window.location = "app-test?test="+(testNum+1);
                } else {
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
            }
            if (toload!==undefined && --toload>0)
                getNextBatch(window,toload);
        }
        
        
    });
}
//////////////////////
////slide
                var batchHeader = document.createElement("div");
                //batchHeader.classList.toggle('spotting');
                batchHeader.classList.toggle('batchHeader');
                batchHeader.id='b'+jres.batchId
                batchHeader.innerHTML='<div>'+jres.ngram+'</div>';
	            if (batchQueue.length>0 && jres.ngram == batchQueue[batchQueue.length-1].ngram) {
                    batchHeader.hidden=true
                    
	            }
	            
	            batchHeader.style.background=headerColors[colorIndex];
	            
	            
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

/////tap
            function(batchId,resultsId,ngram,spottings) {
                    var batchContainer = document.createElement("div");
                    batchContainer.id='b'+batchId;
                    
                    var batchHeader = document.createElement("div");
                    batchHeader.classList.toggle('spotting');
                    batchHeader.classList.toggle('batchHeader');
                    batchHeader.innerHTML=ngram;
                    if (lastNgram!=ngram) {
		                colorIndex = (++colorIndex)%headerColors.length;
		                lastNgram=ngram;
	                }
		            batchHeader.style.background=headerColors[colorIndex];
		            
		            batchContainer.appendChild(batchHeader);
                    
                    for (var index=0; index<spottings.length; index++) {
                        var i=spottings[index];
                        var im = new Image();
                        im.src='data:image/png;base64,'+i.data;
                        var widget = createTapped(im,i.id,batchId);
                        batchContainer.appendChild(widget);
                        batches[batchId].spottings[i.id]=1;//assume good
                    }
                    
                    windowEle.insertBefore(batchContainer,windowEle.childNodes[0]);
                    if (batchQueue.length>0) {
                                 batchContainer.hidden=true
		            }
                    spinner.hidden=true;
                
            }
                */   


/*function getNextBatchBase(elementSetup,toload,callback) {
    

    var query='';
    if (testMode)
        query='&test='+testNum;
    httpGetAsync('/app/nextBatch?width='+imgWidth+query,function (res){
        var jres=JSON.parse(res);
        if (jres.err==null) {
            if (jres.batchType=='spottings') {
                batches[jres.batchId]={sent:false, ngram:jres.ngram, spottings:{}};
                
                
                if (batchQueue.length>0 && jres.ngram != batchQueue[batchQueue.length-1].ngram) {
	                colorIndex = (++colorIndex)%headerColors.length;
                }
                
                elementSetup(jres.batchId,jres.resultsId,jres.ngram,jres.spottings);
                batchQueue.push({ngram:jres.ngram, id:jres.batchId, rid:jres.resultsId});
                
                spinner.hidden=true;
            }
            
            if (toload!==undefined && --toload>0)
                getNextBatchBase(elementSetup,toload,callback);
            else if (callback!==undefined)
                callback();
            
            
        }
        
        
    });
}*/
/////////////////////////


//pink,green, orange, pink, blue
var headerColors = ['#E0B0FF','#7fb13d','#CD7F32','#C5908E','#95B9C7'];
var spottingColors = ['#f3e5fb','#cad6bb','#eedece','#e7d6d5','#d7e6ec'];
var ondeckColors = ['#eed3ff','#b3d389','#ddb185','#dfb8b7','#bbcbd1'];
var colorIndex=0;

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

var lastRemovedBatchInfo=[];

function batchShiftAndSend(batchId,callback) {
    
    var info = batchQueue[0];
    if (info.type+info.id != batchId) {// && info.id != batchId.substr(1)) {
        console.log("ERROR, not matching ids: "+info.type+info.id+" "+batchId);
        console.log(batchQueue);
    }
    batchQueue=batchQueue.slice(1)
    if (batchQueue.length>0) {
        
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
        if (testMode)
            query+='&test='+testNum;
        var labels = [];
        var ids = [];
        for (var id in batches[batchId].spottings){
            if (batches[batchId].spottings.hasOwnProperty(id)) {
                var label=batches[batchId].spottings[id];
                ids.push(id);
                labels.push(label);
            }
        }
        httpPostAsync('/app/submitBatch'+query,{batchId:info.id,resultsId:resultsId,labels:labels,ids:ids,undos:countUndos,time:new Date().getTime()-startTime},function (res){
            
            var jres=JSON.parse(res);
            console.log(jres);
            if (testMode && jres.done) {
                //console.log(batchQueue.length);
                if (batchQueue.length==0)
                    window.location.href = "/app-test-"+(testNum+1);
            } else (!testMode || !allReceived)
                callback();
        });
        batches[batchId].sent=true;
    } else if (info.type=='t' || info.type=='m') {
        var query='?type=transcription';
        if (info.type=='m') {
            query+='Manual';
        }
        httpPostAsync('/app/submitBatch'+query,{batchId:info.id,label:info.transcription,time:new Date().getTime()-startTime},function (res){
            
            var jres=JSON.parse(res);
            console.log(jres);
            if (testMode && jres.done) {
                //console.log(batchQueue.length);
                if (batchQueue.length==0)
                    window.location.href = "/app-test-"+(testNum+1);
            } else (!testMode || !allReceived)
                callback();
        });

    } else if (info.type=='e') {
        
        var query='?type=newExemplars&resend='+batches[batchId].sent;
        if (testMode)
            query+='&test='+testNum;
        var labels = [];
        for (var id in batches[batchId].spottings){
            if (batches[batchId].spottings.hasOwnProperty(id)) {
                var label=batches[batchId].spottings[id];
                labels.push(label);
            }
        }
        httpPostAsync('/app/submitBatch'+query,{batchId:info.id,labels:labels,undos:countUndos,time:new Date().getTime()-startTime},function (res){
            
            var jres=JSON.parse(res);
            console.log(jres);
            if (testMode && jres.done) {
                //console.log(batchQueue.length);
                if (batchQueue.length==0)
                    window.location.href = "/app-test-"+(testNum+1);
            } else (!testMode || !allReceived)
                callback();
        });
        batches[batchId].sent=true;
    }
}
