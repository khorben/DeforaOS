<div class="wiki">
<?php require_once('./system/icon.php'); ?>
<?php if(isset($title)) { ?>
	<h1 class="title wiki"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<script type="text/javascript" src="js/editor.js"></script>
	<form class="explorer">
	<div class="toolbar">
		<img src="<?php echo _html_safe(_icon('cut')); ?>" alt="<?php echo _html_safe(CUT); ?>" title="<?php echo _html_safe(CUT); ?>" onclick="editorExec('cut')"/>
		<img src="<?php echo _html_safe(_icon('copy')); ?>" alt="<?php echo _html_safe(COPY); ?>" title="<?php echo _html_safe(COPY); ?>" onclick="editorExec('copy')"/>
		<img src="<?php echo _html_safe(_icon('paste')); ?>" alt="<?php echo _html_safe(PASTE); ?>" title="<?php echo _html_safe(PASTE); ?>" onclick="editorExec('paste')"/>
		<span class="separator"></span>
		<img src="<?php echo _html_safe(_icon('undo')); ?>" alt="<?php echo _html_safe(UNDO); ?>" title="<?php echo _html_safe(UNDO); ?>" onclick="editorExec('undo')"/>
		<img src="<?php echo _html_safe(_icon('redo')); ?>" alt="<?php echo _html_safe(REDO); ?>" title="<?php echo _html_safe(REDO); ?>" onclick="editorExec('redo')"/>
		<span class="separator"></span>
		<img src="<?php echo _html_safe(_icon('hrule')); ?>" alt="<?php echo _html_safe(INSERT_HORIZONTAL_RULE); ?>" title="<?php echo _html_safe(INSERT_HORIZONTAL_RULE); ?>" onclick="editorInsert('<hr>')"/>
		<img src="<?php echo _html_safe(_icon('insert-link')); ?>" alt="<?php echo _html_safe(INSERT_LINK); ?>" title="<?php echo _html_safe(INSERT_LINK); ?>" onclick="editorLink()"/>
		<img src="<?php echo _html_safe(_icon('insert-image')); ?>" alt="<?php echo _html_safe(INSERT_IMAGE); ?>" title="<?php echo _html_safe(INSERT_IMAGE); ?>" onclick="editorImage()"/>
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
		<img src="<?php echo _html_safe(_icon('bold')); ?>" alt="<?php echo _html_safe(BOLD); ?>" title="<?php echo _html_safe(BOLD); ?>" onclick="editorExec('bold')"/>
		<img src="<?php echo _html_safe(_icon('italic')); ?>" alt="<?php echo _html_safe(ITALIC); ?>" title="<?php echo _html_safe(ITALIC); ?>" onclick="editorExec('italic')"/>
		<img src="<?php echo _html_safe(_icon('underline')); ?>" alt="<?php echo _html_safe(UNDERLINE); ?>" title="<?php echo _html_safe(UNDERLINE); ?>" onclick="editorExec('underline')"/>
		<img src="<?php echo _html_safe(_icon('strikethrough')); ?>" alt="<?php echo _html_safe(STRIKE); ?>" title="<?php echo _html_safe(STRIKE); ?>" onclick="editorExec('strikethrough')"/>
		<span class="separator"></span>
		<img src="<?php echo _html_safe(_icon('subscript')); ?>" alt="<?php echo _html_safe(SUBSCRIPT); ?>" title="<?php echo _html_safe(SUBSCRIPT); ?>" onclick="editorExec('subscript')"/>
		<img src="<?php echo _html_safe(_icon('superscript')); ?>" alt="<?php echo _html_safe(SUPERSCRIPT); ?>" title="<?php echo _html_safe(SUPERSCRIPT); ?>" onclick="editorExec('superscript')"/>
		<span class="separator"></span>
		<img src="<?php echo _icon('justify-left'); ?>" alt="<?php echo _html_safe(ALIGN_LEFT); ?>" title="<?php echo _html_safe(ALIGN_LEFT); ?>" onclick="editorExec('justifyleft')"/>
		<img src="<?php echo _icon('justify-center'); ?>" alt="<?php echo _html_safe(ALIGN_CENTER); ?>" title="<?php echo _html_safe(ALIGN_CENTER); ?>" onclick="editorExec('justifycenter')"/>
		<img src="<?php echo _icon('justify-right'); ?>" alt="<?php echo _html_safe(ALIGN_RIGHT); ?>" title="<?php echo _html_safe(ALIGN_RIGHT); ?>" onclick="editorExec('justifyright')"/>
		<img src="<?php echo _icon('justify-fill'); ?>" alt="<?php echo _html_safe(ALIGN_JUSTIFY); ?>" title="<?php echo _html_safe(ALIGN_JUSTIFY); ?>" onclick="editorExec('justifyfull')"/>
		<span class="separator"></span>
		<img src="<?php echo _html_safe(_icon('bullet')); ?>" alt="<?php echo _html_safe(BULLETS); ?>" title="<?php echo _html_safe(BULLETS); ?>" onclick="editorExec('insertunorderedlist')"/>
		<img src="<?php echo _html_safe(_icon('enum')); ?>" alt="<?php echo _html_safe(ENUMERATED); ?>" title="<?php echo _html_safe(ENUMERATED); ?>" onclick="editorExec('insertorderedlist')"/>
		<img src="<?php echo _html_safe(_icon('unindent')); ?>" alt="<?php echo _html_safe(UNINDENT); ?>" title="<?php echo _html_safe(UNINDENT); ?>" onclick="editorExec('outdent')"/>
		<img src="<?php echo _html_safe(_icon('indent')); ?>" alt="<?php echo _html_safe(INDENT); ?>" title="<?php echo _html_safe(INDENT); ?>" onclick="editorExec('indent')"/>
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
		<tr><td class="field"><?php echo _html_safe(MESSAGE); ?>:</td><td><input type="text" name="message" value="<?php if(isset($message)) echo _html_safe($message); ?>" size="30"/></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('wiki', FALSE, isset($wiki['id']) ? $wiki['id'] : FALSE, isset($wiki['id']) ? $wiki['title'] : FALSE); ?>"><button type="button" class="icon cancel"/><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" name="preview" value="<?php echo _html_safe(PREVIEW); ?>" class="icon preview" onclick="return editorPreview('wikicontent')"/> <input type="submit" name="send" value="<?php echo _html_safe(SUBMIT); ?>" class="icon submit"/></td></tr>
		</table>
	</form>
</div>
