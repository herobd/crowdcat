#!/bin/env node

/*Brian Davis
 *Controller for Thesis project
 */


var express = require('express');
var fs      = require('fs');
var request = require('request');

//var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var session = require('express-session');
var multer = require('multer'); // v1.0.5
var upload = multer();
var flash = require('connect-flash');

var Database = require('./database')();

var passport = require('passport')
  , LocalStrategy = require('passport-local').Strategy;

var transcriberaddon = require("./cpp/build/Debug/transcriberaddon");
//var transcriberaddon = require("./cpp/build/Release/transcriberaddon")

numberOfTests=2;

function printErr(err){if (err) console.log(err);}

//SYSTEM PARAMS
var lexiconFiles=[  "/home/brian/intel_index/data/wordsEnWithNames.txt",
                    "/home/brian/intel_index/data/wordsEnWithNames.txt",
                    "/home/brian/intel_index/data/names_only_f300.txt"
                 ];
var pageImageDirs=[ "/home/brian/intel_index/data/gw_20p_wannot",
                    "/home/brian/intel_index/data/bentham/BenthamDatasetR0-Images/Images/Pages",
                    "/home/brian/intel_index/data/us1930_census/names_only"
                  ];
var segmentationFiles=[ "/home/brian/intel_index/EmbAttSpotter/test/queries_test.gtp",
                        "/home/brian/intel_index/data/bentham/ben_cattss_c_corpus.gtp",
                        "/home/brian/intel_index/data/us1930_census/names_only/seg_names_corpus.gtp"
                      ];
var datasetNames=[  'GW',       //0
                    'BENTHAM',  //1
                    'NAMES'     //2
                 ];
var contextPads=[ 0,
                  0,
                  15
                ];
var avgCharWidths=[ 38,
                    37,
                    20
                  ];
var transcriberModelPrefixes=[ "/home/brian/intel_index/data/gw_20p_wannot/network/phocnet_msf",
                               "?",
                               "?"
                             ];


var datasetNum=0;
var lexiconFile=lexiconFiles[datasetNum];
var pageImageDir=pageImageDirs[datasetNum];
var segmentationFile=segmentationFiles[datasetNum];
var datasetName=datasetNames[datasetNum];
var contextPad=contextPads[datasetNum];
var avgCharWidth=avgCharWidths[datasetNum];
var transcriberModelPrefix=transcriberModelPrefixes[datasetNum];
var savePrefix="save/2_";
var transSaveFile="save/trans_"+datasetName+".dat";
var numThreadsSpotting=5;
var numThreadsUpdating=3;
var showWidth=2500;
var showHeight=1000;
var showMilli=4000;

var saveMode=false;
var timingTestMode=false;
var trainUsers=false;
var debug=true;

//if (saveMode)
savePrefix+=datasetName
if (timingTestMode)
{
    numThreadsSpotting=0;
    savePrefix+='_tt';
}

if (timingTestMode)
{
    numThreadsSpotting=0;
    numThreadsUpdating=0;
}
/**
 *  Define the application.
 */
var ControllerApp = function(port) {

    //  Scope.
    var self = this;
    if (isNaN(port))
        self.port=13723;
    else
        self.port=port;
    
    self.showing=true;
    if (saveMode) {
        self.saveSpottingsQueue={};
        self.saveTransQueue={};
    }    
    if (timingTestMode) {
        self.testSpottingsLabels={};
        self.testTransLabels={};
    }

    self.datasetAssignCounter=1;
    //self.sourceCounter={};
    /*  ================================================================  */
    /*  Helper functions.                                                 */
    /*  ================================================================  */

    /**
     *  Populate the cache.
     */
    self.populateCache = function() {
        if (typeof self.zcache === "undefined") {
            self.zcache = { 'index.html': fs.readFileSync('public/index.html')};
                            //'app.html': fs.readFileSync('public/app.html'),
                            //'login.html': fs.readFileSync('public/login.html') };
        }

        //  Local cache for static content.
        //self.zcache['index.html'] = fs.readFileSync('./index.html');
        //self.zcache['projects.html'] = fs.readFileSync('./projects.html');
        
        //self.zcache['game/assests/Monster Growl-SoundBible.com-344645592.mp3'] = fs.readFileSync('./public/game/assests/Monster Growl-SoundBible.com-344645592.mp3');
        //self.zcache['game/assests/Zombie Moan-SoundBible.com-565291980.wav#t=0.1']=fs.readFileSync('./public/game/assests/Zombie Moan-SoundBible.com-565291980.wav#t=0.1');
        //self.zcache['game/assests/Japanese Temple Bell Small-SoundBible.com-113624364.mp3']=fs.readFileSync('./public/game/assests/Japanese Temple Bell Small-SoundBible.com-113624364.mp3');
    };


    /**
     *  Retrieve entry (content) from cache.
     *  @param {string} key  Key identifying content to retrieve from cache.
     */
    self.cache_get = function(key) { return self.zcache[key]; };


    /**
     *  terminator === the termination handler
     *  Terminate server on receipt of the specified signal.
     *  @param {string} sig  Signal to terminate on.
     */
    self.terminator = function(sig){
        //console.log(self.sourceCounter);
        //transcriberaddon.stopSpotting(function(){console.log("told spotter to stop");}); 
        
        if (typeof sig === "string") {
           console.log('%s: Received %s - terminating control app ...',
                       Date(Date.now()), sig);
           
           process.exit(1);
        }
        console.log('%s: Node server stopped.', Date(Date.now()) );
    };


    /**
     *  Setup termination handlers (for exit and a list of signals).
     */
    self.setupTerminationHandlers = function(){
        //console.log(self.sourceCounter);
        //  Process on exit and signals.
        process.on('exit', function() { self.terminator(); });

        // Removed 'SIGPIPE' from the list - bugz 852598.
        ['SIGHUP', 'SIGINT', 'SIGQUIT', 'SIGILL', 'SIGTRAP', 'SIGABRT',
         'SIGBUS', 'SIGFPE', 'SIGUSR1', 'SIGSEGV', 'SIGUSR2', 'SIGTERM'
        ].forEach(function(element, index, array) {
            process.on(element, function() { self.terminator(element); });
        });
        process.on('SIGPIPE', function () {
              console.log('SIGPIPE fired');
        });
    };


    /*  ================================================================  */
    /*  App server functions (main app logic here).                       */
    /*  ================================================================  */

    
    /**
     *  Initialize the server (express) and create the routes and register
     *  the handlers.
     */
    self.initializeServer = function() {
        self.app = express();//.createServer();
        self.app.use(express.static('public')); //static file: images, css
        self.app.set('views', __dirname + '/views');
        self.app.set('view engine', 'ejs');
        self.app.use(bodyParser.json()); // for parsing application/json
        self.app.use(bodyParser.urlencoded({ extended: true })); // for parsing application/x-www-form-urlencoded
        //self.app.use(cookieParser('this is bad security333221'));
        self.app.use(session({ secret: 'this is bad security333221',
                               saveUninitialized: true, // (default: true)
                               resave: true, // (default: true) 
                              }));
        self.app.use(passport.initialize());
        self.app.use(passport.session());
        self.app.use(flash());
        
        
        //  Add handlers for the app
        
        
        
        self.app.get('/', function(req, res) {
            //console.log('hit');
            //console.log(self.sourceCounter);
            res.setHeader('Content-Type', 'text/html');
            res.send(self.cache_get('index.html') );
            /*if (req.query.source) {
                if (self.sourceCounter.hasOwnProperty(req.query.source)) {
                    if (self.sourceCounter[req.query.source]++ % 10==0) {
                        console.log('source counter');
                        console.log(self.sourceCounter);
                    }
                } else
                    self.sourceCounter[req.query.source]=1;
            }*/
        });
        
        
        self.app.get('/progress_image', function(req, res) {
            res.writeHead(200, "image/jpeg");
            var fileStream = fs.createReadStream('progress/show.jpg');
            fileStream.pipe(res);
        });
        
        
        self.app.get('/app', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                var appName = 'app_full';
                if (!saveMode)
                    res.render(appName, {app_version:'app_full', testMode:false, trainMode:false, save:saveMode, message: req.flash('error') });
                else
                    res.redirect('/');
            } else {
                res.redirect('/login');
            }
        });
        self.app.get('/app-demo', function(req, res) {
            //if (debug) {
                var appName = 'app_full';
                if (!saveMode)
                    res.render(appName, {app_version:'app_full', testMode:false, trainMode:true, save:false, message: req.flash('error') });
                else
                    res.redirect('/');
            //} else {
            //    res.redirect('/login');
            //}
        });
        self.app.get('/app-prep', function(req, res) {
            if (req.user || debug) {

                var appName = 'app_prep';
                if (!saveMode)
                    res.render(appName, {app_version:'app_prep', testMode:false, trainMode:false, save:saveMode, message: req.flash('error') });
                else
                    res.redirect('/');
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/app-tap', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                res.redirect('/');
                //var appName = 'app_tap';
                //res.render(appName, {app_version:'app_tap', testMode:false, trainingMode:false, message: req.flash('error') });
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/app-hardcore', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                var appName = 'app_full';
                if (!saveMode)
                    res.render(appName, {app_version:'app_full', testMode:false, trainMode:false, save:false, message: req.flash('error') });
                else
                    res.redirect('/');
            } else {
                res.redirect('/login');
            }
        });
        self.app.get('/app-test', function(req, res) {
            if (req.user || debug) {
                if (req.user.datasetTiming && !saveMode) {
                    //console.log('[app] user:'+req.user.id+' hit app');
                    var appName = 'app_full';
                    res.render(appName, {app_version:'app_full', testMode:timingTestMode, trainMode:trainUsers, save:false, message: req.flash('error') });
                } else {
                    res.redirect('/home');
                }
            } else {
                res.redirect('/login');
            }
        });
        self.app.get('/app-label', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                var appName = 'app_full';
                if (saveMode)
                    res.render(appName, {app_version:'app_full', testMode:false, trainMode:false, save:true, message: req.flash('error') });
                else
                    res.redirect('/home');
            } else {
                res.redirect('/login');
            }
        });
        self.app.get('/app-false-label', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                var appName = 'app_full';
                if (saveMode)
                    res.render(appName, {app_version:'app_full', testMode:false, trainMode:false, save:false, message: req.flash('error') });
                else
                    res.redirect('/home');
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/home', function(req, res) {
            if (req.user) {
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                if (timingTestMode && !req.user.timingTest) {
                    req.user.datasetTiming=datasetNames[1+(self.datasetAssignCounter++)%(datasetNames.length-1)];
                    req.user.timingTest=true;
                    self.database.updateUser(req.user,printErr);
                }
                //console.log(req.user);
                res.render('home', { message: self.getTestInstructions(req.user), id:req.user.id});
            } else {
                res.redirect('/login');
            }
        });
        
        self.userCount=new Date().getTime();
        self.app.get('/app-test-([123])', function(req, res) {
            if (req.sessionID) {
                var userId = self.userSessionMap[req.sessionID];
                if (!userId)
                    res.redirect('/user-study');
                else {
                    var num = +req.params[0];
                    if (self.userStateMap[userId]!=num) {
                        res.redirect('/app-test-'+self.userStateMap[userId]);
                    } else {
                
                        if (num==undefined || num!=num || num<1)
                            num=1;
                        if (num>numberOfTests) {
                            res.redirect('/feedback');
                        } else {
                            var appName = self.getTestApp(userId,num);
                            res.render(appName, {app_version:appName, testMode:true, testNum:num, message: req.flash('error') });
                        }
                    }
                }
            } else {
                res.redirect('/error');
            }
        });
        
        self.app.get('/login', function(req, res) {
            
            /*(var userMapped = self.userSessionMap[req.session.id]
            self.database.findUser(req.session.id, function(err, user) {
                if (err)
                    console.log(err)
                else if (user!==null)
                    console.log('found mathcing user '+user.id)
                else
                    console.log('no mathcing user');
            });*/
            if (req.user) {
                res.redirect('/home');
            } else {
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('login.html') );
                res.render('login', { message: req.flash('error'), was:req.flash('was') });
            }
        });
        
        self.app.get('/user-study', function(req, res) {
            self.userSessionMap[req.sessionID]=++self.userCount;
            self.userStateMap[self.userSessionMap[req.sessionID]]=1;
            res.render('user-study-alpha', {});
        });
        
        self.app.get('/thankyou', function(req, res) {
            if (self.userSessionMap[req.sessionID]) {
                 self.userStateMap[self.userSessionMap[req.sessionID]]=undefined;
                self.userSessionMap[req.sessionID]=undefined;
            }
            res.render('thankyou', {});
            if (req.user) {
                console.log(req.user.id+' finished');
            }
        });
        
        self.app.get('/feedback', function(req, res) {
            if (self.userSessionMap[req.sessionID])
            {
                var state = self.userStateMap[self.userSessionMap[req.sessionID]];
                if (state > numberOfTests+1) {
                    res.redirect('/thankyou');
                } else if (state > numberOfTests) {
                    res.render('feedback', {userid:self.userSessionMap[req.sessionID]});
                } else {
                    res.redirect('/app-test-'+state);
                }
            } else {
                res.redirect('/user-study');
            }
        });
        
        self.app.get('/xxx/manual*inish', function(req, res) {
            if ((req.user && req.user.id=='herobd@gmail.com') || debug) {
                
                transcriberaddon.manualFinish(function (err) {
                    if (err) console.log(err);
                    res.send('not permitted by CrowdCAT');
                });
            } else {
                res.redirect('/login');
            }
        });
        /*self.app.get('/xxx/resetAllWords_', function(req, res) {
            if ((req.user && req.user.id=='herobd@gmail.com' && saveMode) || debug) {
                transcriberaddon.resetAllWords_();
                res.send('ok, reset');
            } else {
                res.redirect('/login');
            }
        });*/
        self.app.get('/xxx/show_results', function(req, res) {
            if ((req.user && req.user.id=='herobd@gmail.com') || debug) {
                transcriberaddon.showCorpus(function (err) {
                    if (err) console.log(err);
                    res.send('ok');
                });
            } else {
                res.redirect('/login');
            }
        });
        self.app.get('/xxx/show', function(req, res) {
            if ((req.user && req.user.id=='herobd@gmail.com') || debug) {
                var page = 1;
                if (req.query.page) {
                    page = +req.query.page;
                }
                transcriberaddon.showInteractive(page,function (err) {
                    if (err) console.log(err);
                    res.send('ok');
                });
            } else {
                res.redirect('/login');
            }
        });
        /*self.app.get('/xxx/force', function(req, res) {
            if ((req.user && req.user.id=='herobd@gmail.com') || debug) {
                var ngram = '';
                if (req.query.ngram) {
                    ngram = req.query.ngram;
                }
                transcriberaddon.forceNgram(ngram,function (err) {
                    if (err) console.log(err);
                    res.send('ok');
                });
            } else {
                res.redirect('/login');
            }
        });*/
        self.app.get('/xxx/show_toggle', function(req, res) {
            if (req.user || debug) {
                if (self.showing)
                    self.showing=false;
                else {
                    self.showing=true;
                    self.showProgress();
                }
                res.send('ok');
            } else {
                res.redirect('/login');
            }
        });
        self.nextBatch =  function(req, res) {
            res.setHeader("Cache-Control", "no-cache, no-store, must-revalidate"); // HTTP 1.1.
            res.setHeader("Pragma", "no-cache"); // HTTP 1.0.
            res.setHeader("Expires", "0"); // Proxies.
            if (req.user || debug) {
                /*var batchHandler=function (err,batchType,batchId,arg3,arg4,arg5) {
                        //setTimeout(function(){
                        if (batchType==='spottings')
                            res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5});
                        else if (batchType==='transcription' || batchType==='manual')
                            res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,possibilities:arg5});
                        else if (batchType==='newExemplars')
                            res.send({batchType:batchType,batchId:batchId,exemplars:arg3});
                        //else if (batchType==='manual')
                       //     res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,estNumChars:arg5});
                        else
                            res.send({batchType:'ERROR',batchId:-1});
                        //},2000);
                    };*/
                var num=-1;
                if (req.query.num!==undefined && req.query.num!==null)
                    num=+req.query.num;
                
                /*if (req.query.test) {
                    //console.log('user '+self.userSessionMap[req.sessionID]+' is getting a batch (color '+req.query.color+')');
                    if (+req.query.test == self.userStateMap[self.userSessionMap[req.sessionID]]) {
                        var reset=0;
                        if (req.query.reset)
                            reset=1;
                        transcriberaddon.getNextTestBatch(+req.query.width,+req.query.color,num,self.userSessionMap[req.sessionID],reset,function (err,batchType,batchId,resultsId,ngram,spottings) {
                            //setTimeout(function(){
                            res.send({batchType:batchType,batchId:batchId,resultsId:resultsId,ngram:ngram,spottings:spottings});
                            //},2000);
                        });
                    } else {
                        res.send({batchType:"spottings",batchId:"R",resultsId:"X",ngram:"Error, refresh",spottings:[]});
                    }*/
                    
                if (req.query.trainingNum) {
                    transcriberaddon.getNextTrainingBatch(+req.query.width,+req.query.color,req.query.prevNgram,num,+req.query.trainingNum,
                            function (err,batchType,batchId,arg3,arg4,arg5,loc,correct,instructions,lastTraining) {
                                if (batchType==='spottings') {
                                    res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5,instructions:instructions,lastTraining:lastTraining,correct:correct});
                                }
                                else if (batchType==='transcription' || batchType==='manual') {
                                    res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,possibilities:arg5,instructions:instructions,lastTraining:lastTraining,correct:correct});
                                }
                                else if (batchType==='newExemplars') {
                                    res.send({batchType:batchType,batchId:batchId,exemplars:arg3,instructions:instructions,lastTraining:lastTraining,correct:correct});
                                }
                                //else if (batchType==='manual')
                               //     res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,estNumChars:arg5});
                                else if (err==null)
                                    res.send({batchType:'EMPTY',batchId:-1});
                                else
                                    res.send({batchType:'ERROR',batchId:-1,err:err});
                                
                            });
                } else if (req.query.testingNum && timingTestMode && req.user.datasetTiming) {
                    transcriberaddon.getNextTestingBatch(+req.query.width,+req.query.color,req.query.prevNgram,num,
                            +req.query.testingNum,
                            req.user.datasetTiming,
                            function (err,batchType,batchId,arg3,arg4,arg5,loc,correct) {
                                if (batchType==='spottings') {
                                    res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5,correct:correct});
                                    for (var index=0; index<arg5.length; index++) {
                                        var spotting = arg5[index];
                                        self.testSpottingsLabels[spotting.id] = correct[index];
                                    }
                                }
                                else if (batchType==='transcription' || batchType==='manual') {
                                    //console.log('trans correct: '+correct);
                                    res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,possibilities:arg5,correct:correct});
                                    self.testTransLabels[batchId] = correct;
                                }
                                else if (batchType==='newExemplars') {
                                    res.send({batchType:batchType,batchId:batchId,exemplars:arg3,correct:correct});
                                    for (var index=0; index<arg3.length; index++) {
                                        var spotting = arg3[index];
                                        self.testSpottingsLabels[batchId+':'+index] = correct[index];
                                    }
                                }
                                else if (err==null)
                                    res.send({batchType:'EMPTY',batchId:-1});
                                else
                                    res.send({batchType:'ERROR',batchId:-1,err:err});
                                
                            });
                } else {
                    transcriberaddon.getNextBatch(req.user,+req.query.width,
                            function (err,wordId,batchType,img,possibilities,gt) {
                                if (batchType==='spottings') {
                                    console.log('ERROR: spottings batch issued by CrowdCAT, but not implemented on server.');
                                    res.send({batchType:'ERROR',batchId:-1,err:'ERROR: spottings batch issued by CrowdCAT, but not implemented on server.'});
                                    /*
                                    if (saveMode && req.query.save) {
                                        var tmpSpottings=arg5;
                                        var arg5=[];
                                        var doneIds=[];
                                        var doneLabels=[];
                                        for (var index=0; index<tmpSpottings.length; index++) {
                                            var spotting = tmpSpottings[index];
                                            if (correct[index]=='UNKNOWN')
                                            {
                                                self.saveSpottingsQueue[spotting.id] = {ngram:arg4, loc:loc[index], label:null, gt:correct[index]};
                                                arg5.push(spotting);
                                            } else {
                                                doneIds.push(spotting.id);
                                                doneLabels.push(+correct[index]);
                                                var toSave = {ngram:arg4, loc:loc[index], label:+correct[index], gt:correct[index]};
                                                self.database.saveSpotting(datasetName,spotting.id,toSave);
                                            }
                                        }
                                        if (doneIds.length>0) {
                                            console.log('auto completed '+doneIds.length+' '+arg4+' spottings');
                                            transcriberaddon.spottingBatchDone(arg3,doneIds,doneLabels,0,printErr);
                                        }
                                        if (arg5.length>0) {
                                            res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5});
                                        } else {
                                            self.nextBatch(req,res);
                                        }
                                    } else {
                                        res.send({batchType:batchType,batchId:batchId,resultsId:arg3,ngram:arg4,spottings:arg5, debug:err});
                                    }
                                    */
                                }
                                else if (batchType==='transcription' || batchType==='manual') {
                                    res.send({batchType:batchType,batchId:wordId,wordImg:img,ngrams:[],possibilities:possibilities});
                                    if (saveMode && req.query.save) {
                                        //if (batchType==='transcription') {
                                        self.saveTransQueue[batchId] = {wordId:wordId,
                                                                        poss:possibilities,  
                                                                        label:gt, 
                                                                        manual:(batchType==='manual')};
                                    }
                                }
                                else if (batchType==='newExemplars') {
                                    console.log('ERROR: newExemplars batch issued by CrowdCAT, but not implemented on server.');
                                    res.send({batchType:'ERROR',batchId:-1,err:'ERROR: newExemplars batch issued by CrowdCAT, but not implemented on server.'});
                                    /*
                                    res.send({batchType:batchType,batchId:batchId,exemplars:arg3});
                                    if (saveMode && req.query.save) {
                                        for (var index=0; index<arg3.length; index++) {
                                            var spotting = arg3[index];
                                            self.saveSpottingsQueue[batchId+':'+index] = {ngram:spotting.ngram, loc:loc[index], label:null};
                                        }
                                    }
                                    */
                                }
                                //else if (batchType==='manual')
                               //     res.send({batchType:batchType,batchId:batchId,wordImg:arg3,ngrams:arg4,estNumChars:arg5});
                                else if (err==null)
                                    res.send({batchType:'EMPTY',batchId:-1});
                                else
                                    res.send({batchType:'ERROR',batchId:-1,err:err});
                            });
                }
                
            } else {
                res.redirect('/login');
            }
        };
        self.app.get('/app/nextBatch',self.nextBatch);
        
        
        
        
        self.app.post('/app/submitBatch', function (req, res) {
            if (req.user || debug) {
                var resend=0;
                if(req.query.resend == 'true')
                    resend=1;

                if (req.query.trainingNum) {
                    //nothing
                    res.send({done:false});
                } else if (req.query.testingNum) {
                    //TIMING TEST/////////////////////////////////////////////////////////////
                    if (timingTestMode && req.user.datasetTiming) {
                        if (req.query.type=='spottings') {
                            var accuracy=0;
                            var trueRatioDid=0;
                            var trueRatioFull=0;
                            var did=0;
                            for (var index=0; index<req.body.ids.length; index++) {
                                if (req.body.labels[index]!==-1) {
                                    if (+self.testSpottingsLabels[req.body.ids[index]] == req.body.labels[index])
                                        accuracy+=1;
                                    if (+self.testSpottingsLabels[req.body.ids[index]])
                                        trueRatioDid+=1;

                                    did+=1;
                                }
                                if (+self.testSpottingsLabels[req.body.ids[index]])
                                    trueRatioFull+=1;
                                
                            }
                            accuracy /= (0.0+did);
                            trueRatioDid /= (0.0+did);
                            trueRatioFull /= (0.0+req.body.ids.length);
                            var didRatio = did/(0.0+req.body.ids.length);
                            var undoRatio = req.body.undos/(0.0+req.body.ids.length);

                            var info = {userId:req.user.id,
                                        batchNum:+req.query.testingNum,
                                        //dataset:req.user.datasetTiming, 
                                        ngram:req.body.ngram, 
                                        prevNgramSame:req.body.prevNgramSame,
                                        accuracy:accuracy, 
                                        undoRatio:undoRatio,
                                        didRatio:didRatio,
                                        trueRatioDid:trueRatioDid, 
                                        trueRatioFull:trueRatioFull, 
                                        resend:resend,
                                        batchTime:req.body.batchTime};
                            self.database.saveTimingTestSpotting(req.user.datasetTiming,info,printErr);
                        }
                        else if (req.query.type=='transcription') {
                            var accuracy = ( self.testTransLabels[req.body.batchId].toLowerCase()==req.body.label.toLowerCase() )?1:0;
                            var wasBadNgram=false;
                            var wasError=false;
                            var skipped=false;
                            if (self.testTransLabels[req.body.batchId].substr(0,7)=='$REMOVE')
                                wasBadNgram=true;
                            else if (self.testTransLabels[req.body.batchId]=='$ERROR$')
                                wasError=true;
                            else if (req.body.label=='$PASS$') {
                                skipped=true;
                                accuracy=null;
                            }

                            var info = {userId:req.user.id,
                                        batchNum:+req.query.testingNum,
                                        batchId:req.body.batchId,
                                        //dataset:req.user.datasetTiming, 
                                        prevWasTrans:req.body.prevWasTrans,
                                        numPossible:req.body.numPossible, 
                                        positionCorrect:req.body.positionCorrect,
                                        accuracy:accuracy, 
                                        wasBadNgram:wasBadNgram,
                                        wasError:wasError,
                                        skipped:skipped,
                                        undos:req.body.undos,
                                        batchTime:req.body.batchTime};
                            self.database.saveTimingTestTrans(req.user.datasetTiming,info,printErr);
                        }
                        else if (req.query.type=='newExemplars') {
                            console.log('WARNING: newExemplars batch recived during TimingTest');
                        }
                        else if (req.query.type=='transcriptionManual') {
                            var skipped = req.body.label=='$PASS$';
                            var accuracy = ( self.testTransLabels[req.body.batchId].toLowerCase()==req.body.label.toLowerCase() )?1:0;
                            if (skipped)
                                accuracy=null;
                            var info = {userId:req.user.id,
                                        batchNum:+req.query.testingNum,
                                        batchId:req.body.batchId,
                                        //dataset:req.user.datasetTiming, 
                                        prevWasManual:req.body.prevWasManual,
                                        numChar:req.body.label.length, 
                                        accuracy:accuracy, 
                                        skipped:skipped,
                                        undos:req.body.undos,
                                        batchTime:req.body.batchTime};
                            self.database.saveTimingTestManual(req.user.datasetTiming,info,printErr);
                        }
                        res.send({done:false});
                    }
                    else
                        res.send({done:false, err:'timingTest not running'});
                    //END TEST//////////////////////////////////////////////////////////////
                } else { 
                    //Normal behavior
                    if (req.query.type=='spottings') {
                        /*
                        transcriberaddon.spottingBatchDone(req.body.resultsId,req.body.ids,req.body.labels,resend,printErr);
                        if (saveMode && req.query.save) {
                            for (var index=0; index<req.body.ids.length; index++) {
                                if (req.body.labels[index]!==-1 ) {
                                    if (!self.saveSpottingsQueue[req.body.ids[index]])
                                        self.saveSpottingsQueue[req.body.ids[index]]={};
                                    else if (self.saveSpottingsQueue[req.body.ids[index]].gt!='UNKNOWN' &&
                                             (+self.saveSpottingsQueue[req.body.ids[index]].gt)!=req.body.labels[index]) {
                                         console.log("wrong spotting label, correcting");
                                         req.body.labels[index] = (+self.saveSpottingsQueue[req.body.ids[index]].gt);
                                    } 
                                    self.saveSpottingsQueue[req.body.ids[index]].label = req.body.labels[index];
                                    self.database.saveSpotting(datasetName,req.body.ids[index],self.saveSpottingsQueue[req.body.ids[index]]);
                                    self.saveSpottingsQueue[req.body.ids[index]]=null;
                                }
                            }
                        }
                        */
                    }
                    else if (req.query.type=='transcription') {
                        transcriberaddon.transcriptionBatchDone(req.user,req.body.batchId,req.body.label,req.body.wasManual,printErr);
                        if (saveMode && req.query.save && req.body.label!=='$PASS$') {
                            /*if (self.saveTransQueue[req.body.batchId].label!=req.body.label) {
                                console.log('WARNING label dif on trans['+req.body.batchId']: '+self.saveTransQueue[req.body.batchId].label+' != '+req.body.label);
                            }*/
                            if (!self.saveTransQueue[req.body.batchId])
                                self.saveTransQueue[req.body.batchId]={};
                            self.saveTransQueue[req.body.batchId].label=req.body.label;
                            self.database.saveTrans(datasetName,req.body.batchId,self.saveTransQueue[req.body.batchId]);
                            self.saveTransQueue[req.body.batchId]=null; 
                        }
                    }
                    else if (req.query.type=='newExemplars') {
                        /*
                        transcriberaddon.newExemplarsBatchDone(req.body.batchId,req.body.labels,resend,printErr);
                        if (saveMode && req.query.save) {
                            for (var index=0; index<req.body.labels.length; index++) {
                                if (req.body.labels[index]!==-1) {
                                    if (!self.saveSpottingsQueue[req.body.batchId+':'+index])
                                        self.saveSpottingsQueue[req.body.batchId+':'+index]={};
                                    self.saveSpottingsQueue[req.body.batchId+':'+index].label = req.body.labels[index];
                                    self.database.saveSpotting(datasetName,req.body.batchId+':'+index,self.saveSpottingsQueue[req.body.batchId+':'+index]);
                                    self.saveSpottingsQueue[req.body.batchId+':'+index]=null;
                                }
                            }
                        }
                        */
                    }
                    /*
                    else if (req.query.type=='transcriptionManual') {
                        transcriberaddon.manualBatchDone(req.body.batchId,req.body.label,printErr);
                        if (saveMode && req.query.save && req.body.label!=='$PASS$') {
                            if (!self.saveTransQueue[req.body.batchId])
                                self.saveTransQueue[req.body.batchId]={};
                            else if (self.saveTransQueue[req.body.batchId].label!=req.body.label) {
                                console.log('WARNING label dif on trans['+req.body.batchId+']: '+self.saveTransQueue[req.body.batchId].label+' != '+req.body.label);
                            }
                            self.saveTransQueue[req.body.batchId].label=req.body.label;
                            self.database.saveTrans(datasetName,req.body.batchId,self.saveTransQueue[req.body.batchId]);
                            self.saveTransQueue[req.body.batchId]=null; 
                        }
                    }*/

                    res.send({done:false});
                }
                
            } else
                res.redirect('/login');
        });
        

        
        self.app.post('/login',
          
          passport.authenticate('local', { successRedirect: '/home',
                                           failureRedirect: '/login',
                                           failureFlash: true })
        );
        
        self.app.post('/logout',function (req, res) {
            req.logout();
            req.flash('error', 'Thank you!');
            res.redirect('/login');
        });
        
        self.app.post('/create_account', function (req, res) {
            self.database.addUser(req.body.email,{age:req.body.age,experiece:req.body.experience,contactInFuture:req.body.contactInFuture}, function (err) {
                if (err) {
                    req.flash('error', err);
                    req.flash('was', 'create_account');
                    //console.log('Flsh error: '+err);
                    res.redirect('/login');
                } else {
                    //req.user={id:req.body.email}
                    req.login({id:req.body.email}, function (err) {
                        if (err) {
                            req.flash('error', err);
                            req.flash('was', 'create_account');
                            res.redirect('/login');
                        }
                        else
                            res.redirect('/home');
                    });
                }
            });
            
        });
        
        self.app.post('/feedback',function (req, res) {
            //console.log(req.body);
            self.userStateMap[req.body.userid]++;
            self.database.saveAlphaSurvey(req.body.userid,req.body,printErr);
            res.redirect('/thankyou');
        });
        
    };

    //recursively load saved spottings/transcriptions for testing.
    //Recursive to prevent variable overwriting with async.
    self.loadLabeled = function(database,i) {
        if (i>=datasetNames.length)
            return;
        var dataN=datasetNames[i];
        database.getLabeledSpottings(dataN,function(err,labeledSpottings){
            if (err)
                console.log(err);
            else
            {
                labeledSpottings.forEach(function(s) {
                    transcriberaddon.loadLabeledSpotting(dataN,s.ngram,s.label,s.loc.page,s.loc.x1,s.loc.y1,s.loc.x2,s.loc.y2);
                });
                database.getLabeledTrans(dataN,function(err,labeledTrans){
                    if (err)
                        console.log(err);
                    else
                    {
                        labeledTrans.forEach(function(t) {
                            var ngrams=[];
                            var ngramLocs=[];
                            t.ngrams.forEach(function (n) {
                                ngrams.push(n.ngram);
                                ngramLocs.push([n.loc.x1,n.loc.y1,n.loc.x2,n.loc.y2, n.id]);
                            });
                            transcriberaddon.loadLabeledTrans(dataN,t.label,t.poss,ngrams,ngramLocs,+t.loc.wordIndex,t.manual);
                        });
                        transcriberaddon.testingLabelsAllLoaded(dataN);
                        console.log('Loaded all labels for testing: '+dataN);
                        self.loadLabeled(database,i+1);
                    }
                });
            }
        });
    }

    /**
     *  Initializes the sample application.
     */
    self.initialize = function() {
        //self.setupVariables();
        self.populateCache();
        self.setupTerminationHandlers();
        if (timingTestMode) {
            //transcriberaddons={};
            for (var i=1; i<datasetNames.length; i++)
            {
                //transcriberaddons[datasetNames[i]]=require("./cpp/build/Debug/transcriberaddon");
                transcriberaddon.loadTestingCorpus(
                                    datasetNames[i],
                                    pageImageDirs[i],
                                    segmentationFiles[i],
                                    contextPads[i]);
            }

        } else { 
            transcriberaddon.start(lexiconFile,
                                pageImageDir,
                                segmentationFile,
                                transcriberModelPrefix,
                                savePrefix,
                                transSaveFile,
                                //avgCharWidth,
                                //numThreadsSpotting,
                                numThreadsUpdating,
                                showHeight,
                                showWidth,
                                showMilli,
                                contextPad);
        }

        self.database=new Database('localhost:27017/crowdcat', datasetNames, function(database) {
            if (timingTestMode) {
                //for (var i=1; i<datasetNames.length; i++) {
                self.loadLabeled(database,1);
                //}
            }
        });
        passport.use(new LocalStrategy({
            usernameField: 'email',
            passwordField: 'password'
          },
          function(username, password, done) {
            //console.log('LocalStrat: '+username);
            self.database.findUser(username, function (err, user) {
              if (err) { return done(err); }
              if (!user) {
                return done(null, false, { message: 'This email is not in the system.' });
              }
              //if (true||!user.validPassword(password)) {
               // return done(null, false, { message: 'Incorrect password.' });
              //}
               
              return done(null, user);
            });
          }
        ));
        passport.serializeUser(function(user, done) {
            var minutes = 1000 * 60;
            var hours = minutes * 60;
            var days = hours * 24;
            var years = days * 365.25;
            var d = new Date();
            var t= d.getTime();

            var y = Math.floor(t / years);
            var day = Math.floor((t-years*y) / days);
            var hour = Math.floor((t-(years*y+days*day)) / hours);
            var minute = Math.floor((t-(years*y+days*day+hour*hours)) / minutes);
            console.log('Login: '+user.id+', '+day+' '+(6+hour)+':'+minute);
            done(null, user.id);
        });

        passport.deserializeUser(function(id, done) {
          self.database.findUser(id, function(err, user) {
            if (user==null)
                console.log('Failed to find user: '+id);
            done(err, user);
          });
        });
        
        // Create the express server and routes.
        self.userSessionMap={};
        self.userStateMap={};
        self.initializeServer();

    };


    /**
     *  Start the server (starts up the sample application).
     */
    self.start = function() {
        //  Start the app on the specific interface (and port).
        self.app.listen(self.port, /*self.ipaddress,*/ function() {
            console.log('%s: CATTSS-Node server started on :%d ...',
                        Date(Date.now() ), self.port);
            
            /*for (var ii=0; ii<4; ii++)
            {
                transcriberaddon.startSpotting(1,function(){console.log("done spotting, thread "+ii);});
            }*/
            
        });
        //self.resetTestUsers();
        //self.showProgress();
        //transcriberaddon.showProgress(self.showH,self.showW,4000,function(){});
    };
    
    
    /*self.resetTestUsers = function() {
        
        //This could cuase bad things if people are using it right now. But hopefully nobody's up this late...
        transcriberaddon.clearTestUsers(function(){
                self.userSessionMap={};
                self.userStateMap={};
                //self.userCount=0;
            });
        console.log('Cleared users');
        var now = new Date();
        var millisTillTime = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 2, 0, 0, 0) - now;
        if (millisTillTime < 0) {
             millisTillTime += 86400000; // it's after 2am, try 2am tomorrow.
        }
        setTimeout(self.resetTestUsers, millisTillTime);
    }*/
    /*self.showProgress = function() {
        if (self.showing) { 
            console.log('Server::showProgress()');
            transcriberaddon.showProgress(self.showH,self.showW,function(){});
            setTimeout(self.showProgress, 2500);
        }
    }*/
    
    
    //////////////////////////////additional functions
     

    
    self.sendGameDone = function(results) {
        
        request.post(
            'http://'+self.evaluatorAddress+'/gameDone',
            { json: results },
            function (error, response, body) {
                if (!error && response.statusCode == 200) {
                    
                    if (body.status!=='recieved'&&body.status!=='ok') {
                        console.log('ERROR: match '+results.id+' didnt stick in evaluator');
                    }
                    
                }
            }
        );
    };
    
    //localhost:8081/connect?id=controller&address=localhost:8080
    //http://localhost:8081/connect?id=controller&address=localhost:8080
    
    
    self.getTestApp = function(userNum,testNum) {
        if ((userNum+testNum)%2==0)
            return 'app_full';
        else
            return 'app_tap';
    }
    
    self.getTestInstructions = function(user) {
        
        return 'You have been assigned data from the '+user.datasetTiming+' dataset.';
    }
    
};   /*  END Controller Application.  */



/**
 *  main():  Main code.
 */
var zapp = new ControllerApp(+process.argv[2]);
zapp.initialize();
zapp.start();

