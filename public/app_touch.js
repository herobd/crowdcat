var OK_THRESH=-100;
var BAD_THRESH=100;

//var lastRemovedParent=[];
var lastRemoved=[];
var lastRemovedOK=[];

function handleTouchStart(evt) {
    if (evt.touches)                                     
        this.xDown = evt.touches[0].clientX;  
    else
           this.xDown = evt.clientX;                                 
    //yDown = evt.touches[0].clientY;                                      
};                                                

function handleTouchMove(evt) {
    if ( ! this.xDown) {
        return;
    }
    
    var xUp;
    if (evt.touches)
        xUp = evt.touches[0].clientX; 
    else {
        if (evt.buttons%2!=1) {
            handleTouchEnd.call(this,evt);
            return;
        }
        xUp = evt.clientX; 
    }                              
    //var yUp = evt.touches[0].clientY;

    this.xDiff = xUp-this.xDown;
    this.style.left = this.xDiff+'px';
    //var yDiff = yDown - yUp;
    this.getElementsByClassName('num')[0].innerHTML='( '+xUp+' )'
    /*if ( Math.abs( xDiff ) > Math.abs( yDiff ) ) {
        if ( xDiff > 0 ) {
            
            console.log('xDiff: '+xDiff);
            this.getElementsByClassName('num')[0].innerHTML='( '+evt.touches[0].clientX+' )'
        } else {
            
            console.log('xDiff: '+xDiff);
            this.getElementsByClassName('num')[0].innerHTML='( '+evt.touches[0].clientX+' )'
        }
    }*/
    /* reset values */                                           
};

function removeSpotting(OK) {
    lastRemoved.push(this);
    //lastRemovedParent.push(this.parentNode);
    
    lastRemovedOK.push(OK);
    if (lastRemoved.length>10) {
        lastRemoved.shift();
        //lastRemovedParent.shift();
        lastRemovedOK.shift();
    }
    //this.parentNode.removeChild(this);
    this.hidden=true;
    
    if (!OK)
        isBad(this.id);
}

function undo() {
    if (lastRemoved.length>0) {
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        lastRemoved.pop().hidden=false;
        //TODO do something with 
        lastRemovedOK.pop();
    }
}

function handleTouchEnd(evt) {
    //var xUp = evt.touches[0].clientX;    
    this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
    
    this.xDown=null;
    this.style.left = '0px';
    //var xDiff = this.xDown - xUp;
    if (this.xDiff<OK_THRESH) {
        removeSpotting.call(this,true);
        
    } else if (this.xDiff>BAD_THRESH) {
        removeSpotting.call(this,false);
    } else {
    
        
    }
    this.xDiff=0;
}

function setup() {
    var windows = document.getElementsByClassName('spotting');
    for (var i = 0; i < windows.length; i++) {
        windows[i].addEventListener('touchstart', handleTouchStart, false);        
        windows[i].addEventListener('touchmove', handleTouchMove, false);
        windows[i].addEventListener('touchend', handleTouchEnd, false);
        
        windows[i].addEventListener('mousedown', handleTouchStart, false);        
        windows[i].addEventListener('mousemove', handleTouchMove, false);
        windows[i].addEventListener('mouseup', handleTouchEnd, false);
        
        //windows[i].addEventListener('click', function(){console.log('click')}, false);
        windows[i].xDown=null
    }
    document.getElementById('title').addEventListener('click', function(){console.log('click title')}, false);
    
    begin();
}
