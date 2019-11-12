function ServerGet(filename, okCallback, errorCallback) {
    let sendRequest = new XMLHttpRequest();
    sendRequest.open("GET", filename, true);
    sendRequest.onreadystatechange = function() {
	if(sendRequest.readyState == 4) {
	    if(sendRequest.status == 200) {
		okCallback(sendRequest.responseText);
	    } else {
		errorCallback(sendRequest.responseText);
	    }
	}
    }
    sendRequest.send(null);

}

let gOCRLoaded = false;
let gHitsLoaded = false;
let gOcrSplittedFile = null;
let gPageLayoutFile = null;

function CorrectedScrollIntoView(elem)
{
    let rect = document.getElementsByClassName("topnav")[0].getBoundingClientRect();
    window.scrollTo(0,elem.offsetTop-(rect.bottom+50));
}

function HighlightHitsAndConstructLinkList()
{
    let hitlist = document.getElementById("fullsearchhitlist").hitsloaded;

    let ulhitlst = document.getElementById("fullsearchhitlist");
    ulhitlst.innerHTML = "";	//Delete hits if there are any

    if(hitlist && hitlist.books)
    {
	let HighlightList = [];
	if(hitlist.is_fuzzy==false)
	    HighlightList = getParameterByName('query').split('+');

	for(let i = 0; i < hitlist.books.length; i++)
	{
	    let link = document.createElement("a");

	    let inner = hitlist.books[i].page;
	    if(hitlist.is_fuzzy==true)
	    {
		inner+="("+hitlist.books[i].words+"); ";
	    }
	    else
		inner+=" ";
	    link.innerHTML = inner;
	    link.page = hitlist.books[i].page;
	    link.classList.add('hitlinkstyle');
	    link.onclick = function() {
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+hitlist.books[i].page));
		link.style.color = "purple";
	    }

	    ulhitlst.appendChild(link);

	    if(hitlist.is_fuzzy)
	    {
		HighlightList = hitlist.books[i].words.split(",");
	    }

	    for(let x = 0; x < HighlightList.length; x++)
	    {
		let thpage = document.getElementById("uniqueocrpage"+hitlist.books[i].page);
		if(thpage!=null)
		    thpage.innerHTML=thpage.innerHTML.replace(new RegExp('('+HighlightList[x].replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&')+')','gi'),'<mark>$1</mark>');
	    }
	}
    }
    if(location.hash=="")
    {
	CorrectedScrollIntoView(document.getElementById("uniquepageid"+hitlist.books[0].page));
    }
    else
    {
	CorrectedScrollIntoView(document.getElementById("uniquepageid"+location.hash.substr(5)));
    }
}

function UpdateLinkPrev()
{
    let val = document.getElementsByClassName("hitlinkstyle");
    let page = window.location.hash.substr(5);
    if(page=="")
	page=0;
    else
	page=parseInt(page);
    for(let i = 0; i < val.length; i++)
    {
	if(page<=val[i].page)
	{
	    val[i].scrollIntoView();
	    return;
	}
    }
}

function loadOCRFile(ocrtxt)
{
    ocrtxt = ocrtxt.replace(new RegExp("<",'g'),"&lt;");
    ocrtxt = ocrtxt.replace(new RegExp(">",'g'),"&gt;");

    let RegSrch = /-----\s\d+\s.\s\d+\s-----/g;
    gOcrSplittedFile = ocrtxt.split(RegSrch);
    gOcrSplittedFile.shift();
    console.log("SPLIT length: "+gOcrSplittedFile.length);
    //Pagelayout file loaded yet? Then construct the pages
    if(gPageLayoutFile!=null)
	CreatePageLayout();
}

function CreatePageLayout()
{
    console.log("OCR size: "+gOcrSplittedFile.length);
    console.log("Pages size: "+gPageLayoutFile.pages.length);
    for(let i = 0; i < gOcrSplittedFile.length; i++)
    {
	let x = document.createElement("p");
	let cont = document.createElement("div");
	let ocrcont = document.createElement("div");
	let anchor = document.createElement("a");
	anchor.id = "page"+(i+1);

	ocrcont.classList.add("ocrcontainer");

	cont.classList.add('pagecontainer');
	cont.id = "uniquepageid"+(i+1);
	x.innerHTML = gOcrSplittedFile[i]+"<br/><b style='float: right;'>"+(i+1)+"/"+gOcrSplittedFile.length+"</b>";
	x.id = "uniqueocrpage"+(i+1);
	x.classList.add('ocrpage');
	ocrcont.appendChild(anchor);
	ocrcont.appendChild(x);
	cont.appendChild(ocrcont);
	cont.pageNumber = (i+1);

	document.body.appendChild(cont);
    }

    for(let i = 0; i < gPageLayoutFile.pages.length; i++)
    {
	let doc = document.getElementById("uniquepageid"+gPageLayoutFile.pages[i].pageNumber);
	if(doc==null)
	{
	    console.log("Found page without ocr! At : "+gPageLayoutFile.pages[i].pageNumber);
	    continue;
	}

	let svgcode = "<svg class='svgpage' data-width='"+gPageLayoutFile.pages[i].width+"' data-height='"+gPageLayoutFile.pages[i].height+"' data-path='"+"/books/"+getParameterByName('scanId')+ "/" + gPageLayoutFile.pages[i].file.substr(gPageLayoutFile.pages[i].file.search("page"))+"' viewBox='0 0 "+gPageLayoutFile.pages[i].width+" "+gPageLayoutFile.pages[i].height+"'><rect x='0' y='0' width='100%' height='100%'/></svg>";

	doc.innerHTML +=svgcode;
    }

    let timer = null;
    let currentSize = document.getElementsByClassName("topnav")[0].getBoundingClientRect().bottom;
    document.body.onscroll = function() {
	let gNewSize = document.getElementsByClassName("topnav")[0].getBoundingClientRect().bottom;
	if(currentSize!=gNewSize)
	{
	    console.log("Resizing ocrs top value!");
	    let lst = document.getElementsByClassName("ocrpage");
	    for(let i = 0; i < lst.length; i++)
		lst[i].style.top = (gNewSize+17)+"px";
	    currentSize = gNewSize;
	}

	if(timer)
	{
	    window.clearTimeout(timer);
	}

	timer = window.setTimeout(function(){
	    let arr = isPageVisible('svgpage');

	    if(window.history.replaceState)
	    {
		let kk = isPageVisible('pagecontainer');
		if(kk.length>0)
		{
		    let newurl = window.location.search;
		    newurl+="#page"+kk[0].pageNumber;
		    window.history.replaceState({},null,newurl);
		}
	    }

	    UpdateLinkPrev();
	    for(let i = 0; i < arr.length; i++)
	    {
		let img = document.createElement("img");
		img.style.width = arr[i].dataset.width;
		img.style.height = arr[i].dataset.height;
		img.classList.add('imgpage');
		img.src=arr[i].dataset.path;
		img.onload = function(){
		    let x = document.body.scrollTop;
		    arr[i].parentElement.replaceChild(img,arr[i]);
		    document.body.scrollTop = x;
		}
	    }
	},400);
    }

    if(gHitsLoaded)
	HighlightHitsAndConstructLinkList();
    gOCRLoaded = true;
}

function isPageVisible(whatisvis)
{
    let allPages = document.getElementsByClassName(whatisvis);

    let visiblePages = new Array();
    for(let i = 0; i < allPages.length; i++) {
	if(isElementInViewport(allPages[i])) {
	    visiblePages.push(allPages[i]);
	}
    }
    return(visiblePages);
}

function isElementInViewport (el) {
    var rect = el.getBoundingClientRect();
    var optsrect = document.getElementsByClassName("topnav")[0].getBoundingClientRect();

    // it's definitely outside if bottom < 0
    if(rect.bottom <= 0)
	return(false);
    if(rect.top > window.innerHeight)
	return(false);
    if(rect.bottom<(optsrect.bottom+90))
	return(false);
    return(true);
}


function loadOCRFileError(errortext)
{
    document.getElementById("uniqueocrpage0").innerHTML = "Could not load ocr file sorry for this :(";
}

function loadMetadataFile(metadatatxt)
{
    let json = JSON.parse(metadatatxt);
    document.getElementById("bibliography").innerHTML = json.bib;
    if(json.data.creators.length>0)
	document.title = json.data.creators[0].lastName+" ("+json.data.date+"). "+json.data.title;
}

function loadMetadataFileError(errortxt)
{
    document.getElementById("bibliography").innerHTML = "Could not load metadata sorry for that :(";
}

function loadPageLayoutFile(layout)
{   
    gPageLayoutFile = JSON.parse(layout);
    //OCR loaded and splitted yet? Then start building the page!
    if(gOcrSplittedFile!=null)
	CreatePageLayout();
}

function loadPageLayoutFileError(errortxt)
{
    document.getElementById("bookpageimagcontainer").innerHTML = "Could not load image layout sorry for that :(";
}

function highlightHitsAndLoadHitlistError(text)
{
    document.getElementById("fullsearchhitlist").innerHTML = "<li>Could not load hit list sorry for that</li>";
    document.getElementById("fullsearchhitlist").hitsloaded = null;
    if(gOCRLoaded)
	HighlightHitsAndConstructLinkList();
    gHitsLoaded = true;
}

function highlightHitsAndLoadHitlist(hits)
{
    let searchhits = JSON.parse(hits);
    document.getElementById("fullsearchhitlist").hitsloaded = searchhits;
    if(gOCRLoaded)
	HighlightHitsAndConstructLinkList();
    gHitsLoaded = true;
}

function initialise()
{
    let scanId = getParameterByName('scanId');
    let query = getParameterByName('query');
    let fuzzyness = getParameterByName('fuzzyness');

    ServerGet("/books/"+scanId+"/ocr.txt", loadOCRFile,loadOCRFileError);
    ServerGet("/books/"+scanId+"/info.json",loadMetadataFile,loadMetadataFileError);
    ServerGet("/books/"+scanId+"/readerInfo.json",loadPageLayoutFile,loadMetadataFileError);
    ServerGet("/searchinbook?scanId="+scanId+'&query='+encodeURIComponent(query)+'&fuzzyness='+fuzzyness,highlightHitsAndLoadHitlist,highlightHitsAndLoadHitlistError);

    document.getElementById("srchbox").oninput = function() {
	DoFuzzyMatching(this.value);
    }
    document.getElementById('srchbox').addEventListener("focusout", function(event){
	if(event.relatedTarget)
	    event.relatedTarget.click();
	document.getElementById("fuzzysuggestions").style.visibility="hidden";
	document.getElementById("fuzzysuggestions").selec = undefined;});
	document.getElementById("srchbox").addEventListener("keydown",function(event){
			let x = document.getElementById("srchbox").value;
			let k = document.getElementById("fuzzysuggestions").selec;
			if(k==undefined)
			    k = -1;

			if(event.key == "Enter")
			{
			    if(k!=-1)
			    {
				let lst = document.getElementsByClassName("fuzzysugslink");
				lst[k].click();
				return;
			    }
			    doCompleteNewSearch();
			}
			if(event.key=="ArrowUp" || event.key=="ArrowDown")
			{
			    let lst = document.getElementsByClassName("fuzzysugslink");
			    if(k!=-1)
				lst[k].style.background = "";
			    if(event.key=="ArrowUp")
			    {
				k-=1;
				if(k < 0)
				    k = lst.length-1;
			    }
			    else
				k+=1;
			    k = k%lst.length;
			    lst[k].style.background = "#ddd";
			    event.preventDefault();
			    document.getElementById("fuzzysuggestions").selec = k;
			}
		});


    let kk;
    function resize(e) {
	const d_y = e.y-kk;
	kk = e.y;
	console.log(d_y);
	let topnav = document.getElementsByClassName("topnav")[0];
	let resizer = document.getElementsByClassName("resizer")[0];
	topnav.scrollTop = 0;
	console.log((parseInt(getComputedStyle(topnav, '').height) + d_y) + "px");


	topnav.style.height = (parseInt(getComputedStyle(topnav, '').height) + d_y) + "px";
	resizer.style.top = topnav.getBoundingClientRect().bottom + "px";
    }


    document.getElementsByClassName("resizer")[0].addEventListener("mousedown",function(e) {
	document.addEventListener("mousemove",resize,false);
	kk = e.y;
    }, false);
    document.addEventListener("mouseup", function(){
    document.removeEventListener("mousemove", resize, false);
}, false);
}



function SelectLastHit()
{
    SelectHit(false);
}

function SelectHit(direction)
{
    let curr = window.location.hash;
    let integer = parseInt(curr.substr(5));
    let lnks = document.getElementsByClassName("hitlinkstyle");

    if(direction==false)
    {
	for(let i = lnks.length-1; i > -1; i--)
	{
	    if(parseInt(lnks[i].page) < integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at or before the first hit, so jump to the last hit
	lnks[lnks.length-1].click();
	return;
    }
    else
    {
	for(let i = 0; i < lnks.length; i++)
	{
	    if(parseInt(lnks[i].page) > integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at the last hit so jump to the first one
	lnks[0].click();
    }
}


function SelectNextHit()
{
    SelectHit(true);
}

function DoFuzzyMatching(x)
{
    if(x=="")
	return;

    let value = document.getElementsByClassName("ocrpage");
    let results = 0;
    let suggestions = document.getElementById("fuzzysuggestions");
    suggestions.style.visibility = 'visible';
    suggestions.innerHTML = '';

    for(let i = 0; i < value.length; i++)
    {
	let strval = value[i].innerHTML;
	let res = strval.match(new RegExp('.{0,30}('+x+').{0,30}','gi'));
	if(res==null)
	    continue;
	for(let i2 = 0; i2 < res.length; i2++)
	{
	    results+=1;
	    res[i2] = res[i2].replace(new RegExp('('+x+')','gi'),'<mark>$1</mark>');

	    let a = document.createElement("a");
	    a.innerHTML = res[i2]+"<span style='font-weight: bold;float:right;margin-left: 0.8rem;'> S."+(i+1)+"</span>";
	    a.page = i+1;
	    a.word = x;
	    a.result = results-1;
	    a.tabIndex = 0;
	    a.classList.add("fuzzysugslink");
	    a.onclick = function(){
		let page = document.getElementById("uniqueocrpage"+this.page);
		page.innerHTML = page.innerHTML.replace(new RegExp('('+this.word+')','gi'),'<mark>$1</mark>');
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+this.page));
		document.getElementById("fuzzysuggestions").style.visibility = 'hidden';
	    }

	    suggestions.appendChild(a);
	    if(results>9)
	    {
		console.log(results);
		return;
	    }
	}
    }
    if(results==0)
	suggestions.style.visibility = 'hidden';

}

function doCompleteNewSearch()
{
    let newurl = "/GetBooks.html?query="+document.getElementById("srchbox").value+"&scanId="+getParameterByName("scanId")+"&fuzzyness="+getParameterByName('fuzzyness');
    window.location = newurl;
}

window.addEventListener("load",initialise,false);
