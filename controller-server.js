#!/bin/env node

/*Brian Davis
 *Controller for Thesis project
 */


var express = require('express');
var fs      = require('fs');
var request = require('request');

//var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser
var Database = require('./database')();





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
            self.zcache = { 'xx.html': '' };
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
     *  Create the routing table entries + handlers for the application.
     */
    self.createRoutes = function() {
        self.routes = { };


        self.routes['/'] = function(req, res, next) {
             passport.authenticate('local', { successRedirect: '/index.html',
                                   failureRedirect: '/login',
                                   failureFlash: true });
            
        };
        
        self.routes['/login'] = function(req, res, next) {
             res.redirect('/login.html');
            
        };
        self.routes['/info'] = function(req, res, next) {
             res.redirect('/');
            
        };
        self.routes['/a-page'] = function(req, res) {
            res.setHeader('Content-Type', 'text/html');
            res.send(self.cache_get('a-page.html') );
        };
        self.routes['/redir'] = function(req, res) {
            res.redirect('https://somewhere');
        };
        
        
        self.routes['/lastParams'] = function(req, res) {
            //console.log(req.query.id + ' is connecting.');
            
            res.setHeader('Content-Type', 'application/json');
            if (req.query.id === 'evaluator') {
                database.getLastParams(req.query.id,function(err,params) {
                    res.json({id:"controller",status:err, params:params});
                });
            } else {
                res.json({id:"controller",status:"id not recognized"});
            }
            
        };

        
    };


    /**
     *  Initialize the server (express) and create the routes and register
     *  the handlers.
     */
    self.initializeServer = function() {
        self.createRoutes();
        self.app = express();//.createServer();
        self.app.use(express.static('public')); //static file: images, css
        self.app.use(bodyParser.json()); // for parsing application/json
        self.app.use(bodyParser.urlencoded({ extended: true })); // for parsing application/x-www-form-urlencoded
        //self.app.use(cookieParser('this is bad security333221'));
        

        //  Add handlers for the app (from the routes).
        for (var r in self.routes) {
            self.app.get(r, self.routes[r]);
        }
        
        self.app.post('/submit_game', upload.array(), function (req, res, next) {
            //console.log(req.body);
            //meta = JSON.parse(req.body.meta);//meta has id and score atleast
            res.setHeader('Content-Type', 'application/json');
            if (req.body.meta.id && req.body.meta.intrinsicScore && req.body.gdl && req.body.hlgdl)
            {
                self.database.storeGame(req.body.meta,req.body.gdl,req.body.hlgdl, function(err) {
                    var err=self.gameCord.enqueue(req.body.meta);
                
                    res.json({id:req.body.meta.id, status:err});
                });
                
            }
            else
            {
                res.json({status:'malformed'});
            }
        });
        
        self.app.post('/spotting-batch-finished', upload.array(), function (req, res, next) {
            /*Things  I need
            -user_id
            -time_started
            -time_recieved
            -time_finished
            -count_undos
            -batch_id
            -batch {spotting_id: true/false,...}
            */
            console.log(req);
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
        
        
        // Create the express server and routes.
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
    };
    
    
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
    
    
};   /*  END Controller Application.  */



/**
 *  main():  Main code.
 */
var zapp = new ControllerApp(+process.argv[2]);
zapp.initialize();
zapp.start();

