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
    
    //this.getElementsByClassName('num')[0].innerHTML='( '+xUp+' )'
    if (this.xDiff>BAD_THRESH) {
        this.parentNode.style.background='hsl(350,100%,40%)';
    } else if (this.xDiff>0) {
        this.parentNode.style.background='hsl(350,'+(60*(this.xDiff/(0.0+BAD_THRESH)))+'%,40%)';
        //console.log((75*(this.xDiff/(0.0+OK_THRESH)))+'%');
    } else if (this.xDiff<OK_THRESH) {
        this.parentNode.style.background='hsl(130,100%,30%)';
    } else if (this.xDiff<0) {
        this.parentNode.style.background='hsl(130,'+(60*(this.xDiff/(0.0+OK_THRESH)))+'%,30%)';
    }                           
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
    
    if (OK)
        isGood(this);
    else
        isBad(this);
}

function undo() {
    if (lastRemoved.length>0) {
        //var _lastRemovedParent=lastRemovedParent.pop();
        //_lastRemovedParent.insertBefore(lastRemoved.pop(),_lastRemovedParent.getElementsByClassName("spottings")[0]);
        lastRemoved.pop().hidden=false;
        //TODO do something with 
        lastRemovedOK.pop();
    }
    //console.log('UNDO');
}

function handleTouchEnd(evt) {
    //var xUp = evt.touches[0].clientX;    
    //this.getElementsByClassName('num')[0].innerHTML=this.getElementsByClassName('num')[0].innerHTML+' dif='+this.xDiff;
    
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
    var windows = document.getElementsByClassName('window');
    for (var i = 0; i < windows.length; i++) {
        //initSlider(windows[i]);
        windows[i].addEventListener('touchstart', function(e){ e.preventDefault(); });
        windows[i].addEventListener('mousedown', function(e){ e.preventDefault(); });
    }
    begin();
}


function initSlider(ele) {
    ele.addEventListener('touchstart', handleTouchStart, false);        
    ele.addEventListener('touchmove', handleTouchMove, false);
    ele.addEventListener('touchend', handleTouchEnd, false);
    
    ele.addEventListener('mousedown', handleTouchStart, false);        
    ele.addEventListener('mousemove', handleTouchMove, false);
    ele.addEventListener('mouseup', handleTouchEnd, false);
    ele.xDown=null;
}
