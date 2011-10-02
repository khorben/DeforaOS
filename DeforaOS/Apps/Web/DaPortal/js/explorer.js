/* js/explorer.js */



//explorer
function change_class(id, newClass)
{
	var tag;

	tag = document.getElementById(id);
	tag.className = newClass;
	return false;
}


var _entry_click_last = new Array;

function _entry_set(explorer, nb, checked)
{
	var ckbox;
	
	ckbox = document.getElementsByName('entry_'+explorer+'_'+nb);
	if(ckbox.length == 0)
		return 1;
	ckbox = ckbox.item(0);
	ckbox.checked = checked;
	ckbox.parentNode.className = 'entry'+(checked ? ' selected' : '');
	return 0;
}

function entry_click(explorer, nb, event)
{
	var ckbox;

	if(!event)
		event = window.event;
	if(event.ctrlKey)
	{
		ckbox = document.getElementsByName('entry_'+explorer+'_'+nb);
		ckbox = ckbox.item(0);
		_entry_set(explorer, nb, !ckbox.checked);
		_entry_click_last[explorer] = 0;
	}
	else if(event.shiftKey)
	{
		for(var i = _entry_click_last[explorer]; i < nb; i++)
			if(_entry_set(explorer, i, 1))
				break;
		_entry_set(explorer, nb, 1);
		for(var i = nb + 1; i < _entry_click_last[explorer]; i++)
			if(_entry_set(explorer, i, 1))
				break;
	}
	else
	{
		unselect_all(explorer);
		_entry_set(explorer, nb, 1);
		_entry_click_last[explorer] = nb;
	}
}


function select_all(explorer)
{
	_entry_click_last[explorer] = 0;
	for(var i = 1; _entry_set(explorer, i, 1) == 0; i++);
	return false;
}


function unselect_all(explorer)
{
	_entry_click_last[explorer] = 0;
	for(var i = 1; _entry_set(explorer, i, 0) == 0; i++);
	return false;
}


function selection_apply(explorer, action, confirm)
{
	var count = 0;
	var ckbox;
	var explorer;

	for(var i = 1; ckbox = document.getElementsByName('entry_'+explorer+'_'+i); i++)
	{
		if(ckbox.length == 0)
			break;
		if(ckbox.item(0).checked)
			count++;
	}
	if(count == 0)
		return false;
	if(confirm && !window.confirm('Confirm you want to "'+confirm+'" '
			+count+' element(s):'))
		return false;
	explorer = document.getElementsByName('explorer_'+explorer).item(0);
	explorer.apply.value = action;
	explorer.submit();
	return false;
}
