//Copyright 2005 Pierre Pronchery
//Some parts Copyright 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//PHP Explorer is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
//PHP Explorer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PHP Explorer; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//explorer
function change_class(id, newClass)
{
	var tag;

	tag = document.getElementById(id);
	tag.className = newClass;
}


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
