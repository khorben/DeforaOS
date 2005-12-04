//tree
function folder_expand(e)
{
	var tag;

	if(!e)
		e = window.event;
	if(e.target)
		tag = e.target;
	else
		tag = e.srcElement;
	tag.src = 'icons/tree/mlastnode.gif';
	tag.onclick = folder_collapse;
	dir = tag.parentNode.childNodes;
	for(var i = 0; i < dir.length; i++)
	{
		if(dir[i].tagName != 'DIV')
			continue;
		dir[i].style.display = 'block';
	}
}

function folder_collapse(e)
{
	var tag;

	if(!e)
		e = window.event;
	if(e.target)
		tag = e.target;
	else
		tag = e.srcElement;
	tag.src = 'icons/tree/plastnode.gif';
	tag.onclick = folder_expand;
	dir = tag.parentNode.getElementsByTagName('DIV');
	for(var i = 0; i < dir.length; i++)
	{
		dir[i].style.display = 'none';
		if(dir[i].getElementsByTagName('DIV').length == 0)
			continue;
		img = dir[i].childNodes;
		for(var j = 0; j < img.length; j++)
		{
			if(img[j].tagName != 'IMG' || img[j].className != 'node')
				continue;
			img[j].src = 'icons/tree/plastnode.gif';
			img[j].onclick = folder_expand;
		}
	}
}
