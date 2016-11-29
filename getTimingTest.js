const assert = require('assert');
var fs = require('fs');

var dataname1='BENTHAM';
var dataname2='NAMES';
var dataname=dataname2;

function shuffle(array) { //from: http://stackoverflow.com/a/2450976/1018830
  var currentIndex = array.length, temporaryValue, randomIndex;

  // While there remain elements to shuffle...
  while (0 !== currentIndex) {

    // Pick a remaining element...
    randomIndex = Math.floor(Math.random() * currentIndex);
    currentIndex -= 1;

    // And swap it with the current element.
    temporaryValue = array[currentIndex];
    array[currentIndex] = array[randomIndex];
    array[randomIndex] = temporaryValue;
  }

  return array;
}

//time = c + numChars*n + numPoss*p + position*i + prevTrans*t + 
function findParamsMan(err,manTimingInstances) {
    console.log('----MAN----');
    console.log(manTimingInstances.length+' instances');
    assert(manTimingInstances.length>0,"manTimingInstances is empty");
    var meanTime=0.0;

    var minTime=999999;
    var maxTime=-1;

    var meanAcc=0.0;


    var allCSV='acc,time,numChar,prev,user\n';
    for (var inst of manTimingInstances)
    {
        meanTime+= inst.time;
        meanAcc+= inst.accuracy;

        allCSV+=inst.accuracy+','+inst.time+','+inst.numChar+','+(inst.prev?1:0)+','+inst.user+'\n';
    }
    meanTime /= manTimingInstances.length;
    meanAcc /= manTimingInstances.length;
    
    console.log('TIME');
    console.log('all  : '+meanTime);
    var stdTime=0.0;
    for (var inst of manTimingInstances)
    {
        stdTime += (inst.time-meanTime)*(inst.time-meanTime);
    }
    stdTime = Math.sqrt(stdTime/manTimingInstances.length);
    console.log('std time: '+stdTime);

    console.log('\nACC');
    console.log('all  : '+meanAcc);
    var stream = fs.createWriteStream("man_"+dataname+".csv");
    stream.once('open', function(fd) {
        stream.write(allCSV);
        stream.close();

    });
    return;
}

//time = c + numChars*n + numPoss*p + position*i + prevTrans*t + 
function findParamsTrans(err,transTimingInstances) {
    console.log('---TRANS----');
    console.log(transTimingInstances.length+' instances');
    assert(transTimingInstances.length>0,"transTimingInstances is empty");
    var meanTime=0.0;
    var meanTimeAvail=0.0;
    var meanTimeBad=0.0;
    var meanTimeError=0.0;
    var meanTimeNotAvail=0.0;

    var minTime=999999;
    var maxTime=-1;

    var meanAcc=0.0;
    var meanAccAvail=0.0;
    var meanAccBad=0.0;
    var meanAccError=0.0;
    var meanAccNotAvail=0.0;

    var countAvail=0;
    var countBad=0;
    var countError=0;

    var allCSV='acc,time,numPoss,position,prev,avail,bad,error,user\n';
    var availCSV='';
    var badCSV='';
    var errorCSV='';
    var avgN={};
    var countN={};
    for (var inst of transTimingInstances)
    {
        position = inst.position==-1?10:inst.position;
        meanTime += inst.time;
        if (inst.time<minTime)
            minTime=inst.time;
        if (inst.time<maxTime)
            maxTime=inst.time;

        meanAcc+= inst.accuracy;

        allCSV+=inst.accuracy+','+inst.time+','+inst.numPoss+','+position+','+(inst.prev?1:0)+','+(inst.position>-1?1:0)+','+(inst.bad?1:0)+','+(inst.error?1:0)+','+inst.user+'\n';
        if (inst.position>-1) {
            meanAccAvail+= inst.accuracy;
            meanTimeAvail += inst.time;
            countAvail+=1;
            availCSV+=inst.accuracy+','+inst.time+','+inst.numPoss+','+position+','+(inst.prev?1:0)+','+inst.user+'\n';
        }
        else {
            meanTimeNotAvail+=inst.time;
            meanAccNotAvail += inst.accuracy;
        }
        if (inst.bad) {
            meanAccBad+= inst.accuracy;
            meanTimeBad += inst.time;
            //console.log('Bad time: '+inst.time);
            countBad+=1;
            badCSV+=inst.accuracy+','+inst.time+','+inst.numPoss+','+position+','+(inst.prev?1:0)+','+inst.user+'\n';
        }
        if (inst.error) {
            meanAccError+= inst.accuracy;
            meanTimeError += inst.time;
            //console.log('Error time: '+inst.time);
            countError+=1;
            errorCSV+=inst.accuracy+','+inst.time+','+inst.numPoss+','+position+','+(inst.prev?1:0)+','+inst.user+'\n';
        }
        if (!avgN.hasOwnProperty(inst.n)) {
            avgN[inst.n]=0;
            countN[inst.n]=0.0;
        }
        avgN[inst.n]+=inst.time;
        countN[inst.n]+=1.0;
    }
    meanTime /= transTimingInstances.length;
    meanAcc /= transTimingInstances.length;
    
    meanTimeAvail /= countAvail;
    meanAccAvail /= countAvail;
    meanTimeNotAvail /= transTimingInstances.length-countAvail;
    meanAccNotAvail /= transTimingInstances.length-countAvail;

    meanTimeBad /= countBad;
    meanAccBad /= countBad;

    meanTimeError /= countError;
    meanAccError /= countError;
    console.log('tot:'+transTimingInstances.length+' avail:'+countAvail+' bad:'+countBad+' error:'+countError);
    console.log('TIME');
    console.log('all  : '+meanTime);
    console.log('avail: '+meanTimeAvail);
    console.log('not avail: '+meanTimeNotAvail);
    console.log('bad  : '+meanTimeBad);
    console.log('error: '+meanTimeError);
    for (var n in avgN) {
        console.log(n+': '+(avgN[n]/countN[n]));
    }
    var stdTime=0.0;
    for (var inst of transTimingInstances)
    {
        stdTime += (inst.time-meanTime)*(inst.time-meanTime);
    }
    stdTime = Math.sqrt(stdTime/transTimingInstances.length);
    console.log('std time: '+stdTime);

    console.log('\nACC');
    console.log('all  : '+meanAcc);
    console.log('avail: '+meanAccAvail);
    console.log('not avail: '+meanAccNotAvail);
    console.log('bad  : '+meanAccBad);
    console.log('error: '+meanAccError);
    var stream = fs.createWriteStream("timing_all_"+dataname+".csv");
    stream.once('open', function(fd) {
        stream.write(allCSV);
        stream.close();

        stream = fs.createWriteStream("timing_avail_"+dataname+".csv");
        stream.once('open', function(fd) {
            stream.write(availCSV);
            stream.close();

            stream = fs.createWriteStream("timing_bad_"+dataname+".csv");
            stream.once('open', function(fd) {
                stream.write(badCSV);
                stream.close();

                stream = fs.createWriteStream("timing_error_"+dataname+".csv");
                stream.once('open', function(fd) {
                    stream.write(errorCSV);
                    stream.close();
                });
            });
        });
    });
    return;
    var c=meanTime+1.5*stdTime;
    var t=-1*stdTime*1.5;
    var s=stdTime*1.0;
    var p=-1*stdTime*0.75;
    var a=-1*stdTime*0.5;
    console.log('c:'+c+' t:'+t+' s:'+s+' p:'+p+' a:'+a);
    //assert(false);

    var trainingIndex=1;
    var slr=[0.00005, 0.00005, 0.00005, 0.00005, 0.00005];
    for (var i=0; i<3200; i++)
    {
        var lr=slr.slice(0);
        lr[trainingIndex]=(lr[trainingIndex]*2); //(0.0002)/(Math.min(220,i)/8.0+1);
        for (var ii=0; ii<60; ii++) {
            transTimingInstances = shuffle(transTimingInstances);
            var avgDif=0;
            for (inst of transTimingInstances)
            {
                //console.log(inst);
                //console.log(inst.time+' - ('+c+' + '+inst.numT+'*'+t+' + '+inst.numSkip+'*'+s+' + ('+inst.prevSame+'?1:0)*'+p+' + '+inst.accuracy+'*'+a+')');
                var dif = inst.time - (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
                c += dif*lr[0];
                t += dif*(inst.numT*lr[1]);
                //s += dif*(inst.numSkip*lr[2]);
                p += dif*((inst.prevSame?1:0)*lr[3]);
                a += dif*(inst.accuracy*lr[4]);
                avgDif+=dif*dif;
                //console.log('dif: '+dif);
                //assert(!isNaN(c));
            }
            //assert(false);
            //console.log(lr);
            roundDif = (avgDif/transTimingInstances.length);
            console.log('dif: '+roundDif+'\n');
            if (Math.abs(roundDif) < 10)
            {
                var avgT=0;
                for (inst of transTimingInstances)
                {
                    avgT += (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
                }
                console.log('Real mean: '+meanTime);
                console.log('Est mean:  '+avgT/transTimingInstances.length);
                if (Math.abs(avgT/transTimingInstances.length - meanTime) <50) {
                    i=1000000;
                    break;
                }
            }
            for (var j=0; j<lr.length; j++)
                lr[j]*=0.9;
        }
        //console.log('i:'+i+'      c:'+c+' t:'+t+' s:'+s+' p:'+p+' a:'+a);
        console.log('i:'+i+'      c:'+c+' t:'+t+' p:'+p+' a:'+a);
        
        trainingIndex = (trainingIndex+1)%lr.length;
        for (var j=0; j<slr.length; j++)
            slr[j]-=0.00000001;
    }
    var avgT=0;
    for (inst of transTimingInstances)
    {
        avgT += (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
    }
    console.log('Real std:  '+stdTime);
    console.log('Real mean: '+meanTime);
    console.log('Est mean:  '+avgT/transTimingInstances.length);

}

function findParams2(err,spottingTimingInstances) {
    console.log('---SPOTTINGS---');
    console.log(spottingTimingInstances.length+' instances');
    assert(spottingTimingInstances.length>0,"spottingTimingInstances is empty");
    var meanT=0.0;
    var meanS=0.0;
    var meanE=0.0;
    var allCSV='numTrue,skip,error,acc,time,n,prev,user\n';
    var avgN={};
    var countN={};
    var avgTime=0;
    var countTime=0;
    var avgSkipT = [0,0,0,0,0,0];
    var avgErrorT = [0,0,0,0,0,0];
    var counterT = [0,0,0,0,0,0];
    for (var inst of spottingTimingInstances)
    {
        meanT += inst.numT;
        meanS += inst.numSkip;
        meanE += 1-inst.accuracy;
        allCSV += inst.numT+','+inst.numSkip+','+( 1-inst.accuracy)+','+inst.accuracy+','+inst.time+','+inst.n+','+(inst.prevSame?1:0)+','+inst.user+'\n';
        if (!avgN.hasOwnProperty(inst.n)) {
            avgN[inst.n]=0;
            countN[inst.n]=0.0;
        }
        avgN[inst.n]+=inst.time;
        countN[inst.n]+=1.0;
        avgTime+=inst.time;
        countTime+=1.0;

        avgSkipT[inst.numT] += inst.numSkip;
        avgErrorT[inst.numT] += ( 1-inst.accuracy);
        counterT[inst.numT] += 1;
    }
    meanT /= spottingTimingInstances.length;
    meanS /= spottingTimingInstances.length;
    meanE /= spottingTimingInstances.length;
    console.log('T:'+meanT+' S:'+meanS+' E:'+meanE);
    console.log('numT,skip,error');
    for (var t=0; t<=5; t++) {
        console.log(t+','+(avgSkipT[t]/counterT[t])+','+(avgErrorT[t]/counterT[t]));
    }

    console.log('times');
    console.log('ALL: '+(avgTime/countTime));
    for (var n in avgN) {
        console.log(n+': '+(avgN[n]/countN[n]));
    }

    var stream = fs.createWriteStream("spottings_all_"+dataname+".csv");
    stream.once('open', function(fd) {
        stream.write(allCSV);
        stream.close();
    });
}//1.9586776859504131 0.15289256198347104 0.0689393939393939
//NAMES 1.8539823008849559 0.05309734513274336 0.10685840707964596

//time = c + numT*t + numSkip*s + same*p + acc*a
function findParams(err,spottingTimingInstances) {
    assert(spottingTimingInstances.length>0,"spottingTimingInstances is empty");
    var meanTime=0.0;
    var minTime=999999;
    var maxTime=-1;
    for (var inst of spottingTimingInstances)
    {
        meanTime += inst.time;
        if (inst.time<minTime)
            minTime=inst.time;
        if (inst.time<maxTime)
            maxTime=inst.time;
    }
    meanTime /= spottingTimingInstances.length;
    var stdTime=0.0;
    for (var inst of spottingTimingInstances)
    {
        stdTime += (inst.time-meanTime)*(inst.time-meanTime);
    }
    stdTime = Math.sqrt(stdTime/spottingTimingInstances.length);
   
    var c=meanTime+1.5*stdTime;
    var t=-1*stdTime*1.5;
    var s=stdTime*1.0;
    var p=-1*stdTime*0.75;
    var a=-1*stdTime*0.5;
    console.log('c:'+c+' t:'+t+' s:'+s+' p:'+p+' a:'+a);
    //assert(false);

    var trainingIndex=1;
    var slr=[0.00005, 0.00005, 0.00005, 0.00005, 0.00005];
    for (var i=0; i<3200; i++)
    {
        var lr=slr.slice(0);
        lr[trainingIndex]=(lr[trainingIndex]*2); //(0.0002)/(Math.min(220,i)/8.0+1);
        for (var ii=0; ii<60; ii++) {
            spottingTimingInstances = shuffle(spottingTimingInstances);
            var avgDif=0;
            for (inst of spottingTimingInstances)
            {
                //console.log(inst);
                //console.log(inst.time+' - ('+c+' + '+inst.numT+'*'+t+' + '+inst.numSkip+'*'+s+' + ('+inst.prevSame+'?1:0)*'+p+' + '+inst.accuracy+'*'+a+')');
                var dif = inst.time - (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
                c += dif*lr[0];
                t += dif*(inst.numT*lr[1]);
                //s += dif*(inst.numSkip*lr[2]);
                p += dif*((inst.prevSame?1:0)*lr[3]);
                a += dif*(inst.accuracy*lr[4]);
                avgDif+=dif*dif;
                //console.log('dif: '+dif);
                //assert(!isNaN(c));
            }
            //assert(false);
            //console.log(lr);
            roundDif = (avgDif/spottingTimingInstances.length);
            console.log('dif: '+roundDif+'\n');
            if (Math.abs(roundDif) < 10)
            {
                var avgT=0;
                for (inst of spottingTimingInstances)
                {
                    avgT += (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
                }
                console.log('Real mean: '+meanTime);
                console.log('Est mean:  '+avgT/spottingTimingInstances.length);
                if (Math.abs(avgT/spottingTimingInstances.length - meanTime) <50) {
                    i=1000000;
                    break;
                }
            }
            for (var j=0; j<lr.length; j++)
                lr[j]*=0.9;
        }
        //console.log('i:'+i+'      c:'+c+' t:'+t+' s:'+s+' p:'+p+' a:'+a);
        console.log('i:'+i+'      c:'+c+' t:'+t+' p:'+p+' a:'+a);
        
        trainingIndex = (trainingIndex+1)%lr.length;
        for (var j=0; j<slr.length; j++)
            slr[j]-=0.00000001;
    }
    var avgT=0;
    for (inst of spottingTimingInstances)
    {
        avgT += (c + inst.numT*t + 0/*inst.numSkip*s*/ + (inst.prevSame?1:0)*p + inst.accuracy*a);
    }
    console.log('Real std:  '+stdTime);
    console.log('Real mean: '+meanTime);
    console.log('Est mean:  '+avgT/spottingTimingInstances.length);

}
//BENTHAM
//c:40405.123776839086 t:-1944.281912751877 s:5259.0488539179505 p:-3617.1328997568985 a:-20106.61738677886
//c:31383.19685283945 t:-1927.9653804122374 s:13242.712293769018 p:-10357.975453175748 a:-7542.029116605035

//i:3199      c:24248.92906163023 t:-2028.5833450262646 p:-3352.148054049143 a:-2186.604047802453
//Real std:  23061.302757294943
//Real mean: 16115.995867768595
//Est mean:  16120.393965834126

//NAMES
//i:3199      c:27739.37916363281 t:-604.340381032048 p:-5066.242238638676 a:-7473.881787734643
//Real std:  13819.777775636374
//Real mean: 16783.181415929204
//Est mean:  16782.910959408386

var Database = require('./database')();
var database=new Database('localhost:27017/cattss', [dataname1, dataname2], function(db){

    //store as such{ngram, numSkip, numT, numF, prevSame, numObv, accuracy, time}
    //
    db.getSpottingTimings(dataname,findParams2);
    db.getTransTimings(dataname,findParamsTrans);
    db.getManTimings(dataname,findParamsMan);
/*        function(err,spottingTimingInstances1) {

        var meanTime=0.0;
        for (var inst of spottingTimingInstances1)
        {
            meanTime += inst.time;
        }
        db.getSpottingTimings(dataname2,function(err,spottingTimingInstances2) {

            for (var inst of spottingTimingInstances2)
            {
                meanTime += inst.time;
            }
            meanTime /= spottingTimingInstances2.length;
        
        
        
        /*function(err,spottingTimingInstances) {
        if (err) {
            console.log('err '+err);
            return;
        }
        var meanTime=0.0;
        var minTime=999999;
        var maxTime=-1;
        for (var inst of spottingTimingInstances)
        {
            meanTime += inst.time;
            if (inst.time<minTime)
                minTime=inst.time;
            if (inst.time<maxTime)
                maxTime=inst.time;
        }
        meanTime /= spottingTimingInstances.length;
        var stdTime=0.0;
        for (var inst of spottingTimingInstances)
        {
            stdTime += (inst.time-meanTime)*(inst.time-meanTime);
        }
        stdTime = Math.sqrt(stdTime/spottingTimingInstances.length);
       
        var aCode = 'a'.charCodeAt(0);
        var totalLetters = 'z'.charCodeAt(0) - aCode +1;

        /*var fs = require('fs');
        var stream = fs.createWriteStream("timingTrainData_"+dataname+".spec");
        stream.once('open', function(fd) {
            /*stream.write("#ngram spotting timing set "+dataname+"\n");
            stream.write("#milli mean: "+meanTime+"\n");
            stream.write("#milli std: "+stdTime+"\n");

            stream.write("#num inputs\n"+(totalLetters*2 + 5)+"\n");
            stream.write("#num outputs\n1\n");
            for (var inst of spottingTimingInstances) {
                var line = '';
                //one-hot encoding of each letter of ngram
                for (var i=0; i<totalLetters; i++) {
                    if (i==inst.ngram.charCodeAt(0)-aCode)
                        line+='1.0 ';
                    else
                        line+='0.0 ';
                }
                for (var i=0; i<totalLetters; i++) {
                    if (i==inst.ngram.charCodeAt(1)-aCode)
                        line+='1.0 ';
                    else
                        line+='0.0 ';
                }
                line+=((inst.numSkip/5.0)*2.0-1.0)+' ';
                line+=((inst.numT/5.0)*2.0-1.0)+' ';
                line+=((inst.numF/5.0)*2.0-1.0)+' ';
                line+=inst.prevSame?'1.0 ':'0.0 ';
                //line+=((inst.numObv/5.0)*2.0-1.0)+' ';
                line+=(2.0*inst.accuracy-1.0)+'\n';
                //line+=(inst.time-minTime)/(maxTime-minTime)
                line+=((inst.time-meanTime)/(2*stdTime) ) +'\n';
                stream.write(line);
            }*/
            /*stream.write("%Ngram spotting timing set "+dataname+"\n");
            stream.write("%milli mean: "+meanTime+"\n");
            stream.write("%milli std: "+stdTime+"\n");
            stream.write("@RELATION BENTHAM_TIME");
            for (var i=0; i<totalLetters; i++)
                stream.write("@ATTRIBUTE "+String.fromCharCode(aCode+i)+" NUMERIC);
            for (var inst of spottingTimingInstances) {
                var line = '';
                //one-hot encoding of each letter of ngram
                for (var i=0; i<totalLetters; i++) {
                    if (i==inst.ngram.charCodeAt(0)-aCode)
                        line+='1.0 ';
                    else
                        line+='0.0 ';
                }
                for (var i=0; i<totalLetters; i++) {
                    if (i==inst.ngram.charCodeAt(1)-aCode)
                        line+='1.0 ';
                    else
                        line+='0.0 ';
                }
                line+=((inst.numSkip/5.0)*2.0-1.0)+' ';
                line+=((inst.numT/5.0)*2.0-1.0)+' ';
                line+=((inst.numF/5.0)*2.0-1.0)+' ';
                line+=inst.prevSame?'1.0 ':'0.0 ';
                //line+=((inst.numObv/5.0)*2.0-1.0)+' ';
                line+=(2.0*inst.accuracy-1.0)+'\n';
                //line+=(inst.time-minTime)/(maxTime-minTime)
                line+=((inst.time-meanTime)/(2*stdTime) ) +'\n';
                stream.write(line);
            }
            stream.end();
            console.log('COMPLETE');
        });*/
    //});
});

