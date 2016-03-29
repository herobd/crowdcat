function begin() {
    
}
function httpGetAsync(theUrl, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
        else if (xmlHttp.readyState == 4)
            callback(xmlHttp.status);
            
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}
function httpPostAsync(theUrl, theData, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
        else if (xmlHttp.readyState == 4)
            callback(xmlHttp.status);
            
    }
    xmlHttp.open("POST", theUrl, true); // true for asynchronous 
    xmlHttp.setRequestHeader("Content-type", 'application/json');
    xmlHttp.send(JSON.stringify(theData));
}

function isBad(id) {
    //save this spotting is bad. Assum all others good
}

function test() {
    httpGetAsync('/app/test_image?quality=9',function (res){
        var myImage = new Image();
        myImage.src=res;
        document.getElementById('testImageHere').appendChild(myImage);
    });
}

/*window.onbeforeunload = confirmExit;
function confirmExit(){
    return "You have attempted to leave this page.  If you have made any changes to the fields without clicking the Save button, your changes will be lost.  Are you sure you want to exit this page?";
}*/
