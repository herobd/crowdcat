module.exports =  function() {
   //security experts, please feel free to cringe
   var fjfjfj=[32,523,435,21,2436,36,234,5243,346,34,542,5,24,5243,6256,456,24,65,464,
   65,465,56,56,546,542,6,565,46,654,6,5645,654,64,87,8790,3,571,7548,760,36,87356,76,
   57235,9838,625,645,76587,65338,56387,65,47,658,65,53,5,3,52,5,35,436,76,98,5,975,6,
   34,435,1,0,7,65,4,80,644,5,7,9,6,44,6,8,7,544,5,7,88,456,6,54,5,77,45624,456,6,56,
   34,435,1,0,7,65,4,830,644,5,7,9,6,44,6,8,5,544,5,7,88,456,6,54,5,77,45624,456,6,5];
   var fs = require('fs');
    function Database(address,gdlDir) {
        
        var self=this;
        
        self.mongo = require('mongodb').MongoClient;

        // Connect to the db (localhost:27017/exampleDb)
        self.mongo.connect("mongodb://"+address, function(err, db) {
          if(!err) {
            console.log("We are connected to the database.");
            db.collection('THESIS_USERS', function(err, collection) {
                if(!err) {
                    self.userCollection=collection;
                } else {
                    console.log('ERROR: conencting to MongoDB colection THESIS_USERS: '+err);
                }
            });
            
            db.collection('THESIS_ALPHA', function(err, collection) {
                if(!err) {
                    self.alphaCollection=collection;
                } else {
                    console.log('ERROR: conencting to MongoDB colection THESIS_ALPHA: '+err);
                }
            });
            
          } else {
            console.log('ERROR: conencting to MongoDB: '+err);
          }
        });
        
        //test
        var teststr = 'dsafj435jkhrsa@dssfa-d_asf.asdf';
        var enc = self.hideEmail(teststr);
        var back = self.revealEmail(enc);
        if (teststr!=back)
            console.log('Email scramble failed');
    }
    
    Database.prototype.addUser = function (email,survey,callback)  {
        
        var self=this;
        
        self.findUser(email, function(err,item) {
            if (err) {
                callback(err);
            } else if (item!=null){
                callback("User already exists");
            } else {
                self.userCollection.insert({id:email, presurvey:survey}, {w:1}, function(err, result) {
                    callback(err);
                });
            }
        });
        
        
        
    };
    
    
    Database.prototype.findUser = function (email,callback) {
        var self=this;
        self.userCollection.findOne({id:email}, function(err, item) {
            callback(err,item);
        });
    }
    
    Database.prototype.hideEmail = function(email) {
        var ret='';
        for (var i in email) {
            ret+=String.fromCharCode((email.charCodeAt(i)+fjfjfj[i])%256);
        }
        return ret;
    }
    
        
    Database.prototype.revealEmail = function(enc) {
        var ret=''; 
        for (var i in enc) {
            var v = enc.charCodeAt(i)-fjfjfj[i];
            while (v<0)
                v+=256;
            ret+=String.fromCharCode(v%256);
        }
        return ret;
    }
    
    Database.prototype.saveAlphaTest = function(userNum,info,callback) {
        var self=this;
        self.alphaCollection.findOne({_id:(userNum+'')}, function(err, item) {
            if (err) {
                callback(err);
            } else if (item==null) {
                //console.log('adding new user '+userNum);
                self.alphaCollection.insert({_id:(userNum+''), tests:[info]}, {w:1}, callback);
            } else {
                //console.log('updating user '+userNum);
                self.alphaCollection.update({_id:(userNum+'')},{ $push: { tests: info } },{w:1}, callback);
            }
        });
        //self.alphaCollection.insert({_id:(userNum+''), tests:[]}, {w:1}, function(err) {
        //    self.alphaCollection.update({_id:(userNum+'')},{ $push: { tests: info } },{w:1}, callback)
        //});
    }
    
    Database.prototype.saveAlphaSurvey = function(userNum,results,callback) {
        results.email = this.hideEmail(results.email);
        this.alphaCollection.update({_id:(userNum+'')},{$set: { survey:results } } ,{w:1}, callback)
    }
    
    
    return Database
};
