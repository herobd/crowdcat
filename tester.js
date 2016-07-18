#!/bin/env node

/*Brian Davis
 *Tester for Thesis project
 */

var numInBat=2;

var spottingaddon = require("./cpp/build/Debug/spottingaddon");
spottingaddon.startSpotting(3,function(){console.log("done spotting");});
var spottingMap = { "iVBORw0KGgoAAAANSUhEUgAAASwAAAAWCAIAAAD1iTZdAAAKOElEQVR4Ae3Bf0zX953A8efr":2,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAZCAIAAADmA5/xAAAPNUlEQVR4Ae3Bf2yU92HH8ff3":4,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAYCAIAAADPg1ctAAAPhElEQVR4Ae3Be3CV9YHH4c/v":5,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAXCAIAAADcCf6BAAAPFElEQVR4Ae3Bf2yU92HH8ff3":9,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAWCAIAAAAXVS0kAAAOsElEQVR4Ae3BD2zW9YHH8ff3":11,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAASCAIAAABuGHRLAAAJj0lEQVRoBe3Bf0xchQEH8O97":14,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAUCAIAAABanYwvAAAOYElEQVR4Ae3BD2zWd4HH8ff3":17,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAATCAIAAAClRKfuAAAMvUlEQVR4Ae3Bf2yU92HH8ff3":19,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAXCAIAAAA+1eX4AAAOqUlEQVR4Ae3BcWyU52HH8e/z":23,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAZCAIAAAAE34SIAAANw0lEQVR4Ae3BD2yV9aHH4c/v":16,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAYCAIAAAAtX0xUAAAPQ0lEQVR4Ae3Bf3CU9YHH8ff3":18,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAXCAIAAAA+1eX4AAAOjklEQVR4Ae3BfXBV9YHH4c/v":21,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAcCAIAAAC2zg5CAAANm0lEQVR4Ae3Bf2zfdYHH8ef7":24,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAaCAIAAABgl+1fAAAQkklEQVR4Ae3Be3CV5YHH8e/z":25,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAcCAIAAAC2zg5CAAAPq0lEQVR4Ae3BfXDU9YHH8ff3":26,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAWCAIAAAD1iTZdAAAOAUlEQVR4Ae3Bf2yT94HH8ff3":27,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAYCAIAAADPg1ctAAAPu0lEQVR4Ae3Bf3CU9YHH8ff3":28,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAVCAIAAACRwV+KAAAM/klEQVR4Ae3BbWxUdaLH8e//":29
                    };

function getSpot(image) {
    return spottingMap[image.substr(0,72)];
}


//[2]
var approvalMap1 = {2:"error",
                    4:0,
                    5:1,
                    9:1,
                    11:1,
                    14:0,
                    17:"error",
                    19:0,
                    23:1,
                    24:0,
                    25:0,
                    26:0,
                    27:0,
                    28:0,
                    29:0};
//[4]
var approvalMap2 = {2:1,
                    4:0,
                    5:1,
                    9:1,
                    11:1,
                    14:0,
                    17:1,
                    19:0,
                    23:1,
                    24:0,
                    25:0,
                    26:0,
                    27:0,
                    28:0,
                    29:0};
//[8]
var approvalMap3 = {2:0,
                    4:0,
                    5:0,
                    9:0,
                    11:0,
                    14:0,
                    16:1,
                    18:0,
                    21:1};
//[13]
var approvalMap4 = {2:"error",
                    4:"error",
                    5:"error",
                    9:"error",
                    11:"error",
                    14:"error",
                    16:"error",
                    18:"error",
                    21:"error"};
//[17]
var approvalMap5 = {2:0,
                    4:0,
                    8:1,
                    9:0,
                    11:0,
                    14:0,
                    16:0,
                    18:0,
                    22:"error"};
/*/
function approveAllSpottingBatchesX(approvalMap,toDo,callback) {
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=='spottings')
        {
            //res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5});
            var ids=[];
            var labels=[];
            console.log(arg5[0].data);
        }
        else
            toDo.push([err,batchType,batchId,arg3,arg4,arg5]);
    });
}

approveAllSpottingBatchesX(approvalMap1,[],0);
return
/**/


function approveAllSpottingBatches(approvalMap,toDo,callback) {
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=='spottings') {
            //res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5});
            console.log('  approving batch of '+arg4);
            var ids=[];
            var labels=[];
            for (var s of arg5) {
                ids.push(s.id);
                var label = approvalMap[getSpot(s.data)];
                if (label!="error")
                    labels.push(label);
                else {
                    labels.push(-1);
                    console.log("Error given as label for ["+getSpot(s.data)+"]");
                }

                console.log('    approve '+getSpot(s.data));
                console.log('    '+s.data.substr(0,72));
            }

            spottingaddon.spottingBatchDone(arg3,ids,labels,0,function() {approveAllSpottingBatches(approvalMap,toDo,callback);});
        }
        else if (batchType!=null) {
            toDo.push([err,batchType,batchId,arg3,arg4,arg5]);
            approveAllSpottingBatches(approvalMap,toDo,callback);
        }
        else
        {
            console.log("finished approving spottings");
            callback();
        }
    });
}

var toDo=[]
var testId;


function do20 (err,batchType,batchId,arg3,arg4,arg5) {
    console.log('[do20]');
    toDo=[];
    if (batchType=="newExemplars") {
        if (arg3.length==2 && arg3.indexOf('rr')!=-1 && arg3.indexOf('re')!=-1) {
            var approval=[-1,-1];
            approval[arg3.indexOf('rr')]=1;
            //pass on 're'
            spottingaddon.newExemplarsBatchDone(batchId,approval,0,do21);
            
        }
        else {
            console.log('20 ERROR: wrong new ex: ');
            console.log(arg3);
        }
    }
    else
        console.log('20 ERROR: batch it not newEx, but '+batchType+'.   '+arg3);
}

function do17() {
    console.log('[do17]');
    toDo=[];
    //approve 'ee' spotting
    approveAllSpottingBatches(approvalMap5,toDo,function(){
        if (toDo.length==0)
            setTimeout(function() {spottingaddon.getNextBatch(300,0,"",numInBat,do20);},1500);
        else if (toDo.length==1)
            do20(toDo[0],toDo[1],toDo[2],toDo[3],toDo[4]);
        else {
            console.log('17 ERROR: toDo: ');
            console.log(toDo);
        }
    });
}



function do13() {
    console.log('[do13]');
    
    approveAllSpottingBatches(approvalMap4,toDo,function() {
        if (toDo.length==1) {//This should only have new ex 'ee'
            if (toDo[0][1]=='newExemplars' && toDo[0][3].length==1 && toDo[0][3][0].ngram=='ee'){
                spottingaddon.newExemplarsBatchDone(toDo[0][2],[1],0,function(){setTimeout(do17,1500)});

            }
            else
                console.log('13 ERROR: should be new exemplar "ee", but instead is '+toDo[0][1]+'.   '+toDo[0][3]);
        }
        else {
            console.log('13 ERROR: toDo: ');
            console.log(toDo);
        }

            
    } );

}

function do12() {
    console.log('[do12]');
    spottingaddon.transcriptionBatchDone(testId,'teer',function(){setTimeOut(do13,2000);}); //This will extract one exemplar, 'ee', and stop/remove the other two
}
function do11() {
    console.log('[do11]');
    toDo=[];
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=="newExemplars") {
            if (arg3.length==2) {
                spottingaddon.newExemplarsBatchDone(batchId,[1,-1],0,do12);
                
            }
            else {
                console.log('11 ERROR: wrong new ex: ');
                console.log(arg3);
            }
        }
        else
            console.log('11 ERROR: batch it not newEx, but '+batchType+'.   '+arg3);
    });
}

function do9() {
    console.log('[do9]');
    if (toDo.length==1) {
       if (toDo[0][1]=='transcription') {
          if (toDo[0][5].indexOf('teer')!=-1) {
            testId=toDo[0][2];
            spottingaddon.transcriptionBatchDone(testId,'texer',function(){setTimeOut(do11,2000);}); //This will extract two exemplars, 'ex' 'xe'
          } else
             console.log('9 ERROR: no "teer" in : '+toDo[0][5]);

       }
       else
          console.log('9 ERROR: should be transcription task, is '+toDo[0][1]); 
    }
    else {
        console.log('9 ERROR: toDo: ');
        console.log(toDo);
    }
}

function do8() {
    console.log('[do8]');
    approveAllSpottingBatches(approvalMap3,toDo,function(){setTimeout(function() {
        if (toDo.length==0) {
            approveAllSpottingBatches(approvalMap3,toDo,do9);
        }
        else
            do9();
    }, 500);} );
}

function do6() {
    console.log('[do6]');
    toDo=[];
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=="newExemplars") {
            if (arg3.length==1 && arg3[0].ngram=='te') {
                spottingaddon.newExemplarsBatchDone(batchId,[1],0,function(){setTimeOut(do8,4000);});
                
            }
            else {
                console.log('6 ERROR: wrong new ex: ');
                console.log(arg3);
            }
        }
        else
            console.log('6 ERROR: batch it not newEx, but '+batchType+'.   '+arg3);
    });
}

function do5() {
    console.log('[do5]');
    if (toDo.length==1) {
       if (toDo[0][1]=='transcription') {
          if (toDo[0][5].indexOf('ter')!=-1)
            spottingaddon.transcriptionBatchDone(toDo[0][2],'ter',function(){setTimeout(do6,2000);});
          else
             console.log('5 ERROR: no "ter" in : '+toDo[0][5]);

       }
       else
          console.log('5 ERROR: should be transcription task, is '+toDo[0][1]); 
    } else {
        console.log("5 ERROR: toDo:");
        console.log(toDo);
    }
}

function do3() {
    console.log('[do3]');
    if (toDo.length!=0) {
        console.log("3 ERROR: toDo:");
        console.log(toDo);
    } else {
        setTimeout( function(){ approveAllSpottingBatches(approvalMap2,toDo,do5);}, 2000 );   
    }
}
setTimeout( function() {
    console.log('[do1]');
    approveAllSpottingBatches(approvalMap1,toDo,do3);
}, 500);
