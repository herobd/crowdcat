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
   
    var c=meanTime;
    var t=stdTime*1.5;
    var s=stdTime*1.0;
    var p=stdTime*0.75;
    var a=stdTime*0.5;

    var trainingIndex=1;
    for (var i=0; i<48; i++)
    {
        var lr=[0.01, 0.01, 0.01, 0.01, 0.01];
        lr[trainingIndex]=(0.5)/(i+1);
        for (var ii=0; ii<100; ii++) {
            spottingTimingInstances = shuffle(spottingTimingInstances);
            var avgDif=0;
            for (inst of spottingTimingInstances)
            {
                var dif = inst.time - (c + inst.numT*t + inst.numSkip*s + (inst.prevSame?1:0)*p + inst.accuracy*a);
                c += dif*lr[0];
                t += dif*(inst.numT*lr[1]);
                s += dif*(inst.numSkips*lr[2]);
                p += dif*((inst.prevSame?1:0)*lr[3]);
                a += dif*(inst.accuracy*lr[4]);
                avgDif+=dif;
            }
            console.log('i:'+i+' ii:'+ii+'     c:'+c+' t:'+t+' s:'+s+' p:'+p+' a:'+a);
            console.log(lr);
            console.log('dif: '+(avgDif/spottingTimingInstances.length)+'\n');
            for (var j=0; i<lr.length; i++)
                lr[j]*=0.9;
        }
        trainingIndex = (trainingIndex+1)%lr.length;
    }

}


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

