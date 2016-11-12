const assert = require('assert');

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
var dataname='BENTHAM';
var database=new Database('localhost:27017/cattss', [dataname], function(db){

    //store as such{ngram, numSkip, numT, numF, prevSame, numObv, accuracy, time}
    //
    db.getSpottingTimings(dataname,findParams);
        
        
        
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
            /*stream.write("#Ngram spotting timing set "+dataname+"\n");
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

