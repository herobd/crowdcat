#!/bin/env node

/*Brian Davis
 *Tester for Thesis project
 */

var numInBat=2;

var spottingaddon = require("./cpp/build/Debug/spottingaddon");

var spottingMap = { "iVBORw0KGgoAAAANSUhEUgAAASwAAAAWCAIAAAD1iTZdAAAKOElEQVR4Ae3Bf0zX953A8efr":2,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAZCAIAAADmA5/xAAAPNUlEQVR4Ae3Bf2yU92HH8ff3":4,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAYCAIAAADPg1ctAAAPhElEQVR4Ae3Be3CV9YHH4c/v":5,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAXCAIAAADcCf6BAAAPFElEQVR4Ae3Bf2yU92HH8ff3":9,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAWCAIAAAAXVS0kAAAOsElEQVR4Ae3BD2zW9YHH8ff3":11,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAASCAIAAABuGHRLAAAJj0lEQVRoBe3Bf0xchQEH8O97":14,
                    "iVBORw0KGgoAAAANSUhEUgAAASsAAAAUCAIAAABanYwvAAAOYElEQVR4Ae3BD2zWd4HH8ff3":17,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAATCAIAAAClRKfuAAAMvUlEQVR4Ae3Bf2yU92HH8ff3":19,
                    "iVBORw0KGgoAAAANSUhEUgAAASwAAAAXCAIAAAA+1eX4AAAOqUlEQVR4Ae3BcWyU52HH8e/z":23};

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
                    17:1,
                    19:0,
                    23:"error"};
//[4]
var approvalMap2 = {2:1,
                    4:0,
                    5:1,
                    9:1,
                    11:1,
                    14:0,
                    17:1,
                    19:0,
                    23:1};
/*
function approveAllSpottingBatches(approvalMap,toDo,callback) {
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

approveAllSpottingBatches(approvalMap1,[],0);
*/


function approveAllSpottingBatches(approvalMap,toDo,callback) {
    spottingaddon.getNextBatch(300,0,"",numInBat,function (err,batchType,batchId,arg3,arg4,arg5) {
        if (batchType=='spottings') {
            //res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5});
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
            }
        }
        else if (batchType!=null)
            toDo.push([err,batchType,batchId,arg3,arg4,arg5]);
        else
        {
            console.log("finished approving spottings");
            callback();
        }
    });
}

var toDo=[]

function do5() {
    console.log(" finished 4");
    if (toDo.length==1) {
        
    } else {
        console.log("5 ERROR: toDo:");
        console.log(toDo);
    }
}

function do3() {
    if (toDo.length!=0) {
        console.log("3 ERROR: toDo:");
        console.log(toDo);
    } else
        setTimeout( function(){ approveAllSpottingBatches(approvalMap2,toDo,do5);}, 4000 );   
    }
approveAllSpottingBatches(approvalMap1,toDo,do3);
/*        
        
        self.app.post('/app/submitBatch', function (req, res) {
            if (req.user || debug) {
                //console.log(req.body.labels)
                var resend=0;
                if(req.query.resend == 'true')
                    resend=1;
                if (req.query.test) {
                    //console.log('user '+self.userSessionMap[req.sessionID]+' is submitting a batch');
                    var userNum = self.userSessionMap[req.sessionID];
                    
                    spottingaddon.spottingTestBatchDone(req.body.resultsId,req.body.ids,req.body.labels,resend,userNum,function (err,done,fPos,fNeg) {
                        //console.log('submission complete');
                        if (fPos>=0 && fNeg>=0) {
                            
                            self.userStateMap[userNum]++;
                            res.send({done:true});
                            //res.redirect('/app-test?test='+(req.query.test+1));
                            //TODO something with the tracking info.
                            
                            //console.log('user: '+userNum)
                            //console.log(fPos+' false positives, '+fNeg+' false negatives');
                            //console.log('num of undos: '+req.body.undos)
                            //console.log('time elapsed: '+req.body.time)
                            var info = {version:self.getTestApp(userNum,+req.query.test), fp:fPos, fn:fNeg, undos:req.body.undos, time:req.body.time};
                            self.database.saveAlphaTest(userNum,info,function(err){if (err) console.log(err);});
                        } else {
                            res.send({done:false});
                        }
                    });
                } else {
                    if (req.query.type=='spottings')
                        spottingaddon.spottingBatchDone(req.body.resultsId,req.body.ids,req.body.labels,resend,function (err) {
                           if (err) console.log(err); 
                        });
                    else if (req.query.type=='transcription')
                        spottingaddon.transcriptionBatchDone(req.body.id,req.body.label,function (err) {
                            if (err) console.log(err);
                        });

                    res.send({done:false});
                }
                
            } else
                res.redirect('/login');
        });





spottingaddon.showCorpus(function (err) {
    if (err) console.log(err);
});
*/
