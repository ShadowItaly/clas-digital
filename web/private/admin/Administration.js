
//Create an enum to identify the valid types of State Banner changes
var StateBannerType = {"success":1,"danger":2,"warning":3,"info":4,"release":5};
Object.freeze(StateBannerType);

//Shows a banner at the top of the screen, used to inform the user of errors and success
class StateBanner {
  static ChangeBanner(msg, type)
  {
    //Show the new banner message
    let alt = document.getElementById("StateBanner");
    let newHTML = "<strong>";
    newHTML+= msg;
    newHTML+="</strong>";
    alt.innerHTML = newHTML;

    //Decide what kind of layout the banner should show
    if(type===StateBannerType.success)
    {
        alt.setAttribute("class","alert alert-success");
    }
    else if(type===StateBannerType.danger)
    {
        alt.setAttribute("class","alert alert-danger"); 
    }
    else if(type===StateBannerType.warning)
    {
        alt.setAttribute("class","alert alert-warning"); 
    }
    else if(type===StateBannerType.info)
    {
        alt.setAttribute("class","alert alert-info"); 
    }
    else if(type===StateBannerType.release)
    {
        alt.setAttribute("class","None");
        alt.innerHTML = "";
    }
  }
}

let UserList = [];
function OnUserList()
{
    let xhr = new XMLHttpRequest();
    let requ = "/api/v2/server/userlist";
    xhr.open("GET",requ,true);
    xhr.onload = function() {

    let elem = document.getElementById("saveClassGroup");
    let list = document.getElementById("UserList");
    list.innerHTML = '';
    list.appendChild(elem);

    let jsonObj = JSON.parse(xhr.response);

    for(var x in jsonObj)
    {
        if(jsonObj[x].email == undefined)
            continue;
        else if(jsonObj[x].access == undefined)
            continue;

            let y = document.createElement("li");
            y.classList.add("list-group-item");
            let divRo = document.createElement("div");
            divRo.classList.add("row");

            let nameDiv = document.createElement("div");
            nameDiv.classList.add("col-sm-4");
            let textName = document.createTextNode(jsonObj[x].email);
            nameDiv.appendChild(textName);
            divRo.appendChild(nameDiv);

            let currType = jsonObj[x].access;
            UserList[jsonObj[x].email] = currType;

            for( var single = 0; single < 3; single++)
            {
                        let tdiv = document.createElement("div");
                        tdiv.classList.add("col-sm-2");
                        let inp = document.createElement("input");
                        inp.setAttribute("type","checkbox");
                        inp.setAttribute("onclick","UpdateValues(this);SaveUserList();");
                        inp.from_who = jsonObj[x].email;
                        inp.which_attr = single;
			if(single===0)
			{
			    inp.disabled = true;
			}
                        if((currType&(1<<single))!=0)
                        {
                            inp.setAttribute("checked","");
                        }
                    
                    tdiv.appendChild(inp);
                    divRo.appendChild(tdiv);
            }
                nameDiv = document.createElement("div");
                nameDiv.classList.add("col-sm-2");
                textName = document.createElement("button");
                textName.setAttribute("class","btn btn-danger smallerButton");
                textName.innerHTML = "DELETE";
                textName.setAttribute("onclick","DeleteUser(this)");
                textName.from_who = jsonObj[x].email;
                nameDiv.appendChild(textName);
                divRo.appendChild(nameDiv);

                y.appendChild(divRo);
                document.getElementById("UserList").appendChild(y);
        }
    }
    xhr.send(null);
}

function TriggerAlert(msg) {
    StateBanner.ChangeBanner(msg,StateBannerType.danger);
}
function CreateNewUser()
{
    let name = document.getElementById("NUsrName");
    let pw = document.getElementById("NUsrPW");
    let pw2 = document.getElementById("NUsrPW2");
    if(pw.value!=pw2.value)
    {
        TriggerAlert("Passwords do not match!",1);
        return -1;
    }
    else if(pw.value.length < 6)
    {
        TriggerAlert("The password must be at least 6 characters long!",1);
        return -1;
    }
    
    if(name.value in UserList)
    {
        TriggerAlert("The user name is already in use!",1);
        return -1;
    }

    let xhr = new XMLHttpRequest();
	let requ = "/api/v2/server/userlist";
  xhr.open("POST",requ,true);
  xhr.onload = function() {
  if(xhr.status==200)
  {
	  window.location = window.location;
  }
  else
  {
    StateBanner.ChangeBanner(xhr.responseText,StateBannerType.danger);
  }
  }
	xhr.send(JSON.stringify([{action:"CREATE",email:name.value,password:pw.value,access:1}]));
    name.value ="";
    pw2.value="";
    pw.value="";
}

function DeleteUser(elem)
{
    if(confirm("Do you really want to delete user "+elem.from_who))
    {
        let xhr = new XMLHttpRequest();
        let requ = "/api/v2/server/userlist";
        xhr.open("POST",requ,true);
        xhr.onload = function(){
            if(xhr.status==200)
	      StateBanner.ChangeBanner("The user was deleted successfully",StateBannerType.success);
            else
	      StateBanner.ChangeBanner("Something went wrong",StateBannerType.danger);
                window.location = window.location;};
		xhr.send(JSON.stringify([{action:"DELETE",email:elem.from_who}]));
    }
}

function ShutdownServer()
{
    let xhr = new XMLHttpRequest();
    xhr.open("GET","/api/v1/shutdown",true);
    xhr.withCredentials = true;
    xhr.onload = function() {
	if(xhr.status==200)
	{
	    StateBanner.ChangeBanner("The server will restart in a few seconds!",StateBannerType.success);
	    window.scrollTo(0,0);
	    window.setTimeout(function(){DoLogout()},1000);
	    return;
	}
	StateBanner.ChangeBanner("Could not restart server, error code: "+xhr.responseText,StateBannerType.danger);
    }
    xhr.send();
}

function UpdateZotero()
{
    let xhr = new XMLHttpRequest();
    xhr.open("GET","/api/v1/update_zotero",true);
    xhr.withCredentials = true;
    xhr.onload = function() {
	if(xhr.status==200)
	{
	    StateBanner.ChangeBanner("Zotero has been updated successfully",StateBannerType.success);
	    window.scrollTo(0,0);
	    return;
	}
	StateBanner.ChangeBanner("Could not update zotero, error code: "+xhr.responseText,StateBannerType.danger);
    }
    xhr.send();
    window.scrollTo(0,0);
    StateBanner.ChangeBanner("Zotero started to update, this may take half an hour you can close this window safely",StateBannerType.success);

}

function SaveUserList()
{
  let xhr = new XMLHttpRequest();
  let requ = "/api/v2/server/userlist";
  xhr.open("POST",requ,true);
  
  let sendReq = "";
	var sendObj = [];
  for(var i in UserList)
  {
	  sendObj.push({action: "CHANGEACCESS",email: i, access: UserList[i]});
  }
  xhr.onload = function() {
    if(xhr.status == 200) {
    StateBanner.ChangeBanner("Changed user rights successfully!",StateBannerType.success);
    }
    else {
      StateBanner.ChangeBanner("You are not authorized to change the user table!",StateBannerType.danger);
    }
    }
    xhr.onerror = function() {
    StateBanner.ChangeBanner("You are not authorized to change the user table!",StateBannerType.danger);
    }
  xhr.send(JSON.stringify(sendObj));
}

function UpdateValues(elem)
{
    UserList[elem.from_who] = (UserList[elem.from_who]&(~(1<<elem.which_attr)))|(elem.checked<<elem.which_attr);
}

window.addEventListener("load",function(){initialise("administrationlink");OnUserList();},false);
