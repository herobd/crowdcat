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
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAVCAIAAACRwV+KAAAM/klEQVR4Ae3BbWxUdaLH8e//":29,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAANCAIAAACcmMQFAAALwklEQVRoBc3BcWyc9WHH4c/v":8,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAARCAIAAAAKUB2cAAAL1UlEQVRoBe3BfWzU92HH8ff3":22,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAUCAIAAAC4QZdWAAAMS0lEQVR4Ae3BcUxcdYIH8O/v":4.5,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAVCAIAAACRwV+KAAANlUlEQVR4Ae3Bf2yUdYLH8ff3":6,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAANCAIAAACcmMQFAAAIvElEQVRoBe3Bf0ikd2LH8ff3":15,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAaCAIAAACCS/YmAAAPY0lEQVR4Ae3BD2zWd4HH8ff3":3,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAWCAIAAAD1iTZdAAAKk0lEQVR4Ae3BcUxch2HH8e97":13
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
//[23a]
var approvalMap6 = {2:0,
                    4.5:0,
                    6:"error",
                    9:0,
                    11:0,
                    15:1,
                    16:0,
                    18:0,
                    22:0};
//[23b]
var approvalMap7 = {2:0,
                    4.5:1,
                    6:"error",
                    9:0,
                    11:0,
                    15:1,
                    16:0,
                    18:0,
                    22:0};
//[30]
var approvalMap8 = {2:0,
                    3:1,
                    6:0,
                    9:0,
                    11:0,
                    13:1, //"error",
                    16:0,
                    18:0,
                    22:0};
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
                if (getSpot(s.data)) {
                    var label = approvalMap[getSpot(s.data)];
                    if (label!==undefined) {
                        if (label!="error")
                            labels.push(label);
                        else {
                            labels.push(-1);
                            console.log("ERROR: Error given as label for ["+getSpot(s.data)+"]");
                            exit();
                        }
                    } else {
                        labels.push(0);
                        console.log("ERROR: Undefined given as label for ["+getSpot(s.data)+"]");
                        exit();
                    }

                    console.log('    approve '+getSpot(s.data));
                    console.log('    '+s.data.substr(0,72));
                } else
                    console.log('ERROR, no label for '+s.data);
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
var rrIndex=-1;
var savedFarrB;

function finish() {
    spottingaddon.showCorpus(function(){spottingaddon.stopSpotting(function(){console.log('stopped');});});
}

function do32() {
    console.log('[do32]');

    if (toDo.length==1){
        if (toDo[0][3][0].ngram=='at')
            finish();
        else {
            console.log('32 ERROR: '+toDo[0][3][0].ngram);
        }
    } else {

        console.log('32 ERROR: toDo:');
        console.log(toDo);
        console.log(toDo[0][3][0]);
    }
}

function do30() {
    console.log('[do30]');
    toDo=[];
    setTimeout(function() {
        approveAllSpottingBatches(approvalMap8,toDo,do32)
    }, 4000);
}


function do29() {
    console.log('[do29]');
    approveAllSpottingBatches(approvalMap8,toDo,function(){ 
        if (toDo.length==1 && toDo[0][3][0].ngram=='fa'){
            spottingaddon.newExemplarsBatchDone(toDo[0][2],[1],0,function() {setTimeout(do30,4000);});
        }else {
            console.log('29 ERROR: toDo:');
            console.log(toDo);
            console.log(toDo[0][3][0]);
        }
    });
}
        
function do27() {
    console.log('[do27 and 28]');
    toDo=[];
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType==null) {
            spottingaddon.transcriptionBatchDone(savedFarrB[2],'farr',function(){setTimeout(do29,2000);});
        } else {
            console.log('27 ERROR: Should be empty batch, got: '+batchType);
        }
    });
}


function do26() {
    console.log('[do26]');
    if (toDo.length==1  && toDo[0][1]=='transcription') {
        if (toDo[0][4].length==1 && toDo[0][4][0].ngram=='rr') {
            var ngramId = toDo[0][4][0].id; 
            spottingaddon.transcriptionBatchDone(toDo[0][2],'$REMOVE:'+ngramId+'$',function(){setTimeout(do27,2000);}); 
        } else {
            console.log('26 ERROR: ngrams for trams');
            console.log(toDo[0][4]);
        }
    }
    else {
        console.log('26 ERROR: toDo: should have trans for fat(farr)');
        console.log(toDo);
    }
}


function do23b() {
    console.log('[do23b]');
    approveAllSpottingBatches(approvalMap7,toDo,function(){ 
        //This hopefully will only be the correct farr trans batch
        if (toDo.length==1 && toDo[0][1]=='transcription'){
            savedFarrB=toDo[0];
            toDo=[];
            setTimeout(function(){approveAllSpottingBatches(approvalMap7,toDo,do26);},2000);
        
        } else if (toDo.length==2) {
            if (toDo[0][1]=='transcription' && toDo[1][1]=='transcription') {
                if (toDo[0][4].length==1 && toDo[1][4].length==1) {
                    if (toDo[0][4][0].x>toDo[1][4][0].x) {
                        savedFarrB=toDo[0];
                        toDo=[toDo[1]];
                        do26();
                    }else {
                        savedFarrB=toDo[1];
                        toDo=[toDo[0]];
                        console.log('23b order assumption wrong');
                        do26();
                    }

                } else {
                    console.log('23b ERROR: ngrams length off');
                    console.log(toDo);
                }
            } else {
                console.log('23b ERROR: on todo not transcription');
                console.log(toDo);
            }
        } else {
            console.log('23b ERROR: toDo: should have trans for farr');
            console.log(toDo);
        }
    });
}

function do23() {
    console.log('[do23a]');
    toDo=[];
    approveAllSpottingBatches(approvalMap6,toDo,function(){ 
        if (toDo.length==0)
            setTimeout(do23b,4000);
        
        else {
            console.log('23a ERROR: toDo:');
            console.log(toDo);
            console.log(toDo[0][3][0]);
        }
    });
}


function do21() {
    console.log('[do21]');
    toDo=[];
    var approval=[0,0];
    approval[rrIndex]=1;
    //correcting the incorrect spotting
    spottingaddon.newExemplarsBatchDone(testId,approval,1,function() {setTimeout(do23,1500);});
}


function do20 (err,batchType,batchId,arg3,arg4,arg5) {
    console.log('[do20]');
    toDo=[];
    if (batchType=="newExemplars") {
        if (arg3.length==2) {
            if (arg3[0].ngram=='rr') {
                rrIndex=0;
                if (arg3[1].ngram!='re') {
                    console.log("ERROR 20: odd ngram (should be re): "+arg3[1].ngram);
                    return;
                }
            } else if (arg3[1].ngram=='rr') {
                rrIndex=1;
                if (arg3[0].ngram!='re') {
                    console.log("ERROR 20: odd ngram (should be re): "+arg3[0].ngram);
                    return;
                }
            } else {
                console.log("ERROR 20: odd ngram (should be rr or re): "+arg3[1].ngram);
                return;
            }
            var approval=[-1,-1];
            approval[rrIndex]=0;
            testId=batchId
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
        if (toDo.length==0) {
            console.log('  getBatch in do17 again');
            setTimeout(function() {spottingaddon.getNextBatch(300,0,"",numInBat,do20);},4000);
        }
        else if (toDo.length==1)
            do20(toDo[0][0],toDo[0][1],toDo[0][2],toDo[0][3],toDo[0][4]);
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
                spottingaddon.newExemplarsBatchDone(toDo[0][2],[1],0,function(){setTimeout(do17,4000)});

            }
            else {
                console.log('13 ERROR: should be new exemplar "ee", but instead is '+toDo[0][1]+'.   '+toDo[0][3]);
                if (toDo[0][1]=='newExemplars') {
                    for (var ob of toDo[0][3])
                        console.log(ob);
                }
            }
        }
        else {
            console.log('13 ERROR: toDo: ');
            console.log(toDo);
        }

            
    } );

}

function do12() {
    console.log('[do12]');
    spottingaddon.transcriptionBatchDone(testId,'teer',function(){setTimeout(do13,4000);}); //This will extract one exemplar, 'ee', and stop/remove the other two
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
            spottingaddon.transcriptionBatchDone(testId,'texer',function(){setTimeout(do11,2000);}); //This will extract two exemplars, 'ex' 'xe'
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

    //approve 'te'
    approveAllSpottingBatches(approvalMap3,toDo,function(){setTimeout(function() {
        if (toDo.length==0) {
            console.log('   run do8 again');
            approveAllSpottingBatches(approvalMap3,toDo,function() {setTimeout(do9,5000);});
        }
        else
            do9();
    }, 2000);} );
}

function do6() {
    console.log('[do6]');
    toDo=[];
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=="newExemplars") {
            if (arg3.length==1 && arg3[0].ngram=='te') {
                spottingaddon.newExemplarsBatchDone(batchId,[1],0,function(){setTimeout(do8,4000);});
                
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
/**/
