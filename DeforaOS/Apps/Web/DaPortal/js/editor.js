/* js/editor.js */



//editor
function editorExec(cmd)
{
	var editor = document.getElementById('editor');

	editor.contentWindow.document.execCommand(cmd, false, null);
	editor.contentWindow.focus();
}


function editorImage()
{
	var editor = document.getElementById('editor');
	var url;

	url = prompt('Enter URL:', 'http://');
	if(url != null && url != '')
		editor.contentWindow.document.execCommand('insertimage', false,
				url);
}


function editorInsert(str)
{
	var editor = document.getElementById('editor');

	editor.contentWindow.document.execCommand('inserthtml', false, str);
}


function editorLink()
{
	var editor = document.getElementById('editor');
	var url;

	url = prompt('Enter URL:', 'http://');
	if(url != null && url != '')
		editor.contentWindow.document.execCommand('createlink', false,
				url);
}


function editorPreview(id)
{
	var editor = document.getElementById('editor');
	var preview = document.getElementById(id);

	if(editor == null || preview == null)
		return true;
	preview.innerHTML = editor.contentWindow.document.body.innerHTML;
	return false;
}


function editorSelect(id)
{
	var editor = document.getElementById('editor');
	var e = document.getElementById(id);
	var sel = e.selectedIndex;

	if(sel != 0)
	{
		editor.contentWindow.document.execCommand(id, false,
				e.options[sel].value);
		e.selectedIndex = 0;
	}
	editor.contentWindow.focus();
}


function editorStart()
{
	var editor = document.getElementById('editor');
	var editortext = document.getElementById('editortext');

	editortext.style.visibility = 'hidden';
	editortext.className = 'hidden';
	editor.className = '';
	editor.contentWindow.document.designMode = "on";
	try {
		editor.contentWindow.document.execCommand("undo", false, null);
	}
	catch (e) {
		alert("This editor is not supported in your browser");
		return;
	}
	editor.contentWindow.document.body.innerHTML = editortext.value;
	editor.contentWindow.focus();
}


function editorSubmit()
{
	var editor = document.getElementById('editor');
	var editortext = document.getElementById('editortext');

	editortext.value = editor.contentWindow.document.body.innerHTML;
}
