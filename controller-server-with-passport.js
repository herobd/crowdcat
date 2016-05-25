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

var spottingaddon = require("./cpp/build/Debug/spottingaddon")

var debug=true;
numberOfTests=2;
/**
 *  Define the sample application.
 */
var ControllerApp = function(port) {

    //  Scope.
    var self = this;
    if (isNaN(port))
        self.port=13723;
    else
        self.port=port;

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
        //  Process on exit and signals.
        process.on('exit', function() { self.terminator(); });

        // Removed 'SIGPIPE' from the list - bugz 852598.
        ['SIGHUP', 'SIGINT', 'SIGQUIT', 'SIGILL', 'SIGTRAP', 'SIGABRT',
         'SIGBUS', 'SIGFPE', 'SIGUSR1', 'SIGSEGV', 'SIGUSR2', 'SIGTERM'
        ].forEach(function(element, index, array) {
            process.on(element, function() { self.terminator(element); });
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
            res.setHeader('Content-Type', 'text/html');
            res.send(self.cache_get('index.html') );
        });
        
        
        
        
        self.app.get('/app', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                var appName = 'app';
                res.render(appName, {app_version:'app', testMode:false, message: req.flash('error') });
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/app-tap', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                var appName = 'app_tap';
                res.render(appName, {app_version:'app_tap', testMode:false, message: req.flash('error') });
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/app-hardcore', function(req, res) {
            if (req.user || debug) {
                //console.log('[app] user:'+req.user.id+' hit app');
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                var appName = 'app_hardcore';
                res.render(appName, {app_version:'app_hardcore', testMode:false, message: req.flash('error') });
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/home', function(req, res) {
            if (req.user) {
                //res.setHeader('Content-Type', 'text/html');
                //res.send(self.cache_get('app.html') );
                res.render('home', { message: self.getTestInstructions(req.user) });
            } else {
                res.redirect('/login');
            }
        });
        
        self.userCount=new Date().getTime();
        self.app.get('/app-test-([123])', function(req, res) {
            if (req.sessionID) {
                
                if (!self.userSessionMap[req.sessionID])
                    res.redirect('/user-study');
                else {
                    var num = +req.params[0];
                
                
                    if (num==undefined || num!=num)
                        num=1;
                    if (num>numberOfTests) {
                        res.redirect('/feedback');
                    } else {
                        var appName = self.getTestApp(self.userSessionMap[req.sessionID],num);
                        res.render(appName, {app_version:appName, testMode:true, testNum:num, message: req.flash('error') });
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
            console.log('hit suer-study, creating user '+(1+self.userCount));
            self.userSessionMap[req.sessionID]=++self.userCount;
            res.render('user-study-alpha', {});
        });
        
        self.app.get('/thankyou', function(req, res) {
            if (self.userSessionMap[req.sessionID])
                self.userSessionMap[req.sessionID]=undefined;
            res.render('thankyou', {});
        });
        
        self.app.get('/feedback', function(req, res) {
            if (self.userSessionMap[req.sessionID])
            {
                res.render('feedback', {userid:self.userSessionMap[req.sessionID]});
            } else {
                res.redirect('/user-study');
            }
        });
        
        self.app.get('/app/test_image', function(req, res) {
            if (req.user || debug) {
                spottingaddon.getTestImage(+req.query.quality,function (err,data64) {
                    res.send('data:image/png;base64,'+data64);
                });
            } else {
                res.redirect('/login');
            }
        });
        
        self.app.get('/app/nextBatch', function(req, res) {
            res.setHeader("Cache-Control", "no-cache, no-store, must-revalidate"); // HTTP 1.1.
            res.setHeader("Pragma", "no-cache"); // HTTP 1.0.
            res.setHeader("Expires", "0"); // Proxies.
            if (req.user || debug) {
                var num=-1;
                if (req.query.num!==undefined && req.query.num!==null)
                    num=+req.query.num;
                
                if (req.query.test) {
                    //console.log('user '+self.userSessionMap[req.sessionID]+' is getting a batch (color '+req.query.color+')');
                    spottingaddon.getNextTestBatch(+req.query.width,+req.query.color,num,self.userSessionMap[req.sessionID],function (err,batchType,batchId,resultsId,ngram,spottings) {
                        //setTimeout(function(){
                        res.send({batchType:batchType,batchId:batchId,resultsId:resultsId,ngram:ngram,spottings:spottings});
                        //},2000);
                    });
                } else {
                    spottingaddon.getNextBatch(+req.query.width,+req.query.color,req.query.prevNgram,num,function (err,batchType,batchId,resultsId,ngram,spottings) {
                        //setTimeout(function(){
                        res.send({batchType:batchType,batchId:batchId,resultsId:resultsId,ngram:ngram,spottings:spottings});
                        //},2000);
                    });
                }
                
            } else {
                res.redirect('/login');
            }
        });
        
        
        
        
        self.app.post('/app/submitBatch', function (req, res) {
            /*Things  I need
            -user_id = req.user.id
            -time_started
            -time_recieved
            -time_finished
            -count_undos
            -batch_id
            -batch {spotting_id: true/false,...}
            */
            if (req.user || debug) {
                //console.log(req.body.labels)
                var resend=0;
                if(req.query.resend == 'true')
                    resend=1;
                if (req.query.test) {
                    //console.log('user '+self.userSessionMap[req.sessionID]+' is submitting a batch');
                    
                    spottingaddon.spottingTestBatchDone(req.body.resultsId,req.body.ids,req.body.labels,resend,self.userSessionMap[req.sessionID],function (err,done,fPos,fNeg) {
                        //console.log('submission complete');
                        if (fPos>=0 && fNeg>=0) {
                            res.send({done:true});
                            //res.redirect('/app-test?test='+(req.query.test+1));
                            //TODO something with the tracking info.
                            var userNum = self.userSessionMap[req.sessionID];
                            console.log('user: '+userNum)
                            console.log(fPos+' false positives, '+fNeg+' false negatives');
                            console.log('num of undos: '+req.body.undos)
                            console.log('time elapsed: '+req.body.time)
                            var info = {version:self.getTestApp(userNum,+req.query.test), fp:fPos, fn:fNeg, undos:req.body.undos, time:req.body.time};
                            self.database.saveAlphaTest(userNum,info,function(err){if (err) console.log(err);});
                        } else {
                            res.send({done:false});
                        }
                    });
                } else {
                    spottingaddon.spottingBatchDone(req.body.resultsId,req.body.ids,req.body.labels,resend,function (err) {
                        
                    });
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
            self.database.addUser(req.body.email,{age:req.body.age,experiece:req.body.experience}, function (err) {
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
            self.database.saveAlphaSurvey(req.body.userid,req.body,function(err){if (err) console.log(err);});
            res.redirect('/thankyou');
        });
        
    };


    /**
     *  Initializes the sample application.
     */
    self.initialize = function() {
        //self.setupVariables();
        self.populateCache();
        self.setupTerminationHandlers();
        
        
        self.database=new Database('localhost:27017');
        passport.use(new LocalStrategy({
            usernameField: 'email',
            passwordField: 'password'
          },
          function(username, password, done) {
            console.log('LocalStrat: '+username);
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
          console.log('serializeUser: '+user.id);
          done(null, user.id);
        });

        passport.deserializeUser(function(id, done) {
          console.log('deserializeUser: '+id);
          self.database.findUser(id, function(err, user) {
            if (user==null)
                console.log('Failed to find user: '+id);
            done(err, user);
          });
        });
        
        // Create the express server and routes.
        self.userSessionMap={};
        self.initializeServer();
    };


    /**
     *  Start the server (starts up the sample application).
     */
    self.start = function() {
        //  Start the app on the specific interface (and port).
        self.app.listen(self.port, /*self.ipaddress,*/ function() {
            console.log('%s: Node server started on :%d ...',
                        Date(Date.now() ), self.port);
            
            
            
        });
        self.resetTestUsers();
    };
    
    
    self.resetTestUsers = function() {
        
        //This could cuase bad things if people are using it right now. But hopefully nobody's up this late...
        spottingaddon.clearTestUsers(function(){
                self.userSessionMap={};
                //self.userCount=0;
            });
        console.log('Cleared users');
        var now = new Date();
        var millisTillTime = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 2, 0, 0, 0) - now;
        if (millisTillTime < 0) {
             millisTillTime += 86400000; // it's after 2am, try 2am tomorrow.
        }
        setTimeout(self.resetTestUsers, millisTillTime);
    }
    
    
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
            return 'app_hardcore';
        else
            return 'app_tap';
    }
    
    self.getTestInstructions = function(user) {
        //TODO
        return 'This is filler for instructions currently.';
    }
    
};   /*  END Controller Application.  */



/**
 *  main():  Main code.
 */
var zapp = new ControllerApp(+process.argv[2]);
zapp.initialize();
zapp.start();

