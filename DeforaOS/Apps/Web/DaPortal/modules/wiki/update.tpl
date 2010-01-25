<div class="wiki">
<?php if(isset($title)) { ?>
	<h1 class="title wiki"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<script type="text/javascript" src="js/editor.js"></script>
	<form class="explorer">
	<div class="toolbar">
		<span class="icon cut" title="<?php echo _html_safe(CUT); ?>" onclick="editorExec('cut')"></span>
		<span class="icon copy" title="<?php echo _html_safe(COPY); ?>" onclick="editorExec('copy')"></span>
		<span class="icon paste" title="<?php echo _html_safe(PASTE); ?>" onclick="editorExec('paste')"></span>
		<span class="separator"></span>
		<span class="icon undo" title="<?php echo _html_safe(UNDO); ?>" onclick="editorExec('undo')"></span>
		<span class="icon redo" title="<?php echo _html_safe(REDO); ?>" onclick="editorExec('redo')"></span>
		<span class="separator"></span>
		<span class="icon hrule" title="<?php echo _html_safe(INSERT_HORIZONTAL_RULE); ?>" onclick="editorInsert('<hr>')"></span>
		<span class="icon link" title="<?php echo _html_safe(INSERT_LINK); ?>" onclick="editorLink()"></span>
		<span class="icon insert-image" title="<?php echo _html_safe(INSERT_IMAGE); ?>" onclick="editorImage()"></span>
	</div>
	<div class="toolbar">
		<select id="formatblock" onchange="editorSelect(this.id)">
			<option value="Style"><?php echo _html_safe(STYLE); ?></option>
			<option value="<h1>">Heading 1</option>
			<option value="<h2>">Heading 2</option>
			<option value="<h3>">Heading 3</option>
			<option value="<h4>">Heading 4</option>
			<option value="<h5>">Heading 5</option>
			<option value="<h6>">Heading 6</option>
			<option value="<p>">Normal</option>
			<option value="<pre>">Preformatted</option>
		</select>
		<select id="fontname" unselectable="on" onchange="editorSelect(this.id)">
			<option value="Font"><?php echo _html_safe(FONT); ?></option>
			<option value="cursive">Cursive</option>
			<option value="fantasy">Fantasy</option>
			<option value="monospace">Monospace</option>
			<option value="sans-serif">Sans serif</option>
			<option value="serif">Serif</option>
		</select>
		<select id="fontsize" unselectable="on" onchange="editorSelect(this.id)">
			<option value="Size"><?php echo _html_safe(SIZE); ?></option>
			<option value="1">1</option>
			<option value="2">2</option>
			<option value="3">3</option>
			<option value="4">4</option>
			<option value="5">5</option>
			<option value="6">6</option>
		</select>
		<span class="separator"></span>
		<span class="icon bold" title="<?php echo _html_safe(BOLD); ?>" onclick="editorExec('bold')"></span>
		<span class="icon italic" title="<?php echo _html_safe(ITALIC); ?>" onclick="editorExec('italic')"></span>
		<span class="icon underline" title="<?php echo _html_safe(UNDERLINE); ?>" onclick="editorExec('underline')"></span>
		<span class="icon strikethrough" title="<?php echo _html_safe(STRIKE); ?>" onclick="editorExec('strikethrough')"></span>
		<span class="separator"></span>
		<span class="icon subscript" title="<?php echo _html_safe(SUBSCRIPT); ?>" onclick="editorExec('subscript')"></span>
		<span class="icon superscript" title="<?php echo _html_safe(SUPERSCRIPT); ?>" onclick="editorExec('superscript')"></span>
		<span class="separator"></span>
		<span class="icon align_left" title="<?php echo _html_safe(ALIGN_LEFT); ?>" onclick="editorExec('justifyleft')"></span>
		<span class="icon align_center" title="<?php echo _html_safe(ALIGN_CENTER); ?>" onclick="editorExec('justifycenter')"></span>
		<span class="icon align_right" title="<?php echo _html_safe(ALIGN_RIGHT); ?>" onclick="editorExec('justifyright')"></span>
		<span class="icon align_justify" title="<?php echo _html_safe(ALIGN_JUSTIFY); ?>" onclick="editorExec('justifyfull')"></span>
		<span class="separator"></span>
		<span class="icon bullet" title="Bullets" onclick="editorExec('insertunorderedlist')"></span>
		<span class="icon enum" title="Enumerated" onclick="editorExec('insertorderedlist')"></span>
		<span class="icon unindent" title="Unindent" onclick="editorExec('outdent')"></span>
		<span class="icon indent" title="Indent" onclick="editorExec('indent')"></span>
	</div>
	</form>
	<form action="<?php echo _html_link(); ?>" method="post" onsubmit="editorSubmit()">
		<input type="hidden" name="module" value="wiki"/>
<?php if(!isset($wiki['id'])) { ?>
		<input type="hidden" name="action" value="insert"/>
<?php } else { ?>
		<input type="hidden" name="action" value="update"/>
		<input type="hidden" name="id" value="<?php echo _html_safe($wiki['id']); ?>"/>
<?php } ?>
		<table width="100%">
<?php if(!isset($wiki['id'])) { ?>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<?php if(isset($wiki['title'])) echo _html_safe($wiki['title']); ?>"/></td></tr>
<?php } ?>
		<tr><td colspan="2"><textarea id="editortext" name="content" cols="80" rows="20"><?php if(isset($wiki['content'])) echo _html_safe($wiki['content']); ?></textarea></td></tr>
		<tr><td colspan="2"><iframe id="editor" class="hidden" width="100%" height="400px" onload="editorStart()"></iframe></td></tr>
		<tr><td colspan="2"><input type="submit" name="preview" value="<?php echo _html_safe(PREVIEW); ?>" onclick="return editorPreview('wikicontent')"/> <input type="submit" name="send" value="<?php echo _html_safe(SUBMIT); ?>"/></td></tr>
		</table>
	</form>
</div>
