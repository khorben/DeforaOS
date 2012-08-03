/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Web DaPortal */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



//editor
function editorStart(editor, id)
{
	var editortext = document.getElementById(id);

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
