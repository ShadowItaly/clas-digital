class Server 
{
    static Get(filename, okCallback, errorCallback) 
    {
        let sendRequest = new XMLHttpRequest();
        sendRequest.open("GET", filename, true);
        sendRequest.onreadystatechange = () => 
        {
            if(sendRequest.readyState == 4) 
            {
                if(sendRequest.status == 200) {
                    okCallback(sendRequest.responseText);
                } else {
                    errorCallback(sendRequest.responseText);
                }
            }
        }

        sendRequest.send(null);
	}
}
function LoadMetadata()
{
    let Index = window.location.href.indexOf("scanId=");
    let value = window.location.href.substr(Index+7);
    Server.Get("/getmetadata?scanId="+value,function(content) {
    let json = JSON.parse(content);
    document.getElementById("details").innerHTML += json.bib;
    document.getElementById("bookcontentlinklist").innerHTML += "<li><a href='/GetBooks.html?scanId="+value+"'>View Book</a></li>";

    if(json.data.ISBN != undefined && json.data.ISBN != "")
        document.getElementById("isbn").innerHTML += json.data.ISBN;
    else{
        document.getElementById("isbn").style.display="none";
        document.getElementById("isbn2").style.display="none";
    }

    if(json.data.date != undefined && json.data.date != "")
        document.getElementById("date").innerHTML+=json.data.date;

    if(json.data.title != undefined && json.data.title != "")
        document.getElementById("title").innerHTML += json.data.title;

    if(json.data.itemType != undefined || json.data.itemType != "")
        document.getElementById("itemType").innerHTML += json.data.itemType;

    if(json.data.place!= undefined && json.data.place != "")
        document.getElementById("place").innerHTML += json.data.place;

    if(json.data.creators != undefined)
    {
        let creator_str = "";
        for(let i = 0; i < json.data.creators.length; i++)
        {
        if(json.data.creators[i].firstName != undefined)
            creator_str += json.data.creators[i].firstName+' ';
        if(json.data.creators[i].lastName != undefined)
            creator_str += json.data.creators[i].lastName;
        if((i+1)<json.data.creators.length)
            creator_str+=";";
        }
        document.getElementById("author").innerHTML += creator_str;
    }
    else
        document.getElementById("author").style.display = "none";

    },function(){});
}
