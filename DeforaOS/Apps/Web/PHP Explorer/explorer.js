//$Id$
//Copyright (c) 2005-2012 Pierre Pronchery <khorben@defora.org>
//Some parts Copyright (c) 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



//explorer
function change_class(id, newClass)
{
	var tag;

	tag = document.getElementById(id);
	tag.className = newClass;
}


var _entry_click_last = 0;

function _entry_set(nb, checked)
{
	var ckbox = document.getElementsByName('entry_'+nb);
	if(ckbox.length == 0)
		return 1;
	ckbox = ckbox.item(0);
	ckbox.checked = checked;
	ckbox.parentNode.className = 'entry'+(checked ? ' selected' : '');
	return 0;
}

function entry_click(nb, event)
{
	var ckbox;

	if(!event)
		event = window.event;
	if(event.ctrlKey)
	{
		ckbox = document.getElementsByName('entry_'+nb).item(0);
		_entry_set(nb, !ckbox.checked);
		_entry_click_last = 0;
	}
	else if(event.shiftKey)
	{
		for(var i = _entry_click_last; i < nb; i++)
			if(_entry_set(i, 1))
				break;
		_entry_set(nb, 1);
		for(var i = nb + 1; i < _entry_click_last; i++)
			if(_entry_set(i, 1))
				break;
	}
	else
	{
		unselect_all();
		_entry_set(nb, 1);
		_entry_click_last = nb;
	}
}


function popup(url)
{
	var p = window.open(url, 'popup', 'height=200, width=300');
	if(p.focus)
		p.focus();
	return false;
}


function select_all()
{
	_entry_click_last = 0;
	for(var i = 0; _entry_set(i, 1) == 0; i++);
}


function unselect_all()
{
	_entry_click_last = 0;
	for(var i = 0; _entry_set(i, 0) == 0; i++);
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


function selection_delete()
{
	var count = 0;
	var ckbox;

	for(var i = 0; ckbox = document.getElementsByName('entry_'+i); i++)
	{
		if(ckbox.length == 0)
			break;
		ckbox = ckbox.item(0);
		if(ckbox.checked)
			count++;
	}
	if(count == 0)
		return;
	if(!window.confirm('Confirm you want to delete '+count+' file(s):'))
		return;
	document.explorer.action.value = 'delete';
	document.explorer.submit();
}
