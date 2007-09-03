<div class="wiki">
	<h1 class="title wiki"><?php echo _html_safe($title); ?></h1>
	<script type="text/javascript"><!-- //FIXME put in a separate js file
function wikiStart()
{
	var wiki = document.getElementById('wiki');
	var wikitext = document.getElementById('wikitext');

	wiki.contentWindow.document.designMode = "on";
	try {
		wiki.contentWindow.document.execCommand("undo", false, null);
	}
	catch (e) {
		alert("This editor is not supported in your browser");
		return;
	}
	wiki.contentWindow.document.body.innerHTML = wikitext.value;
}


function wikiSelect(id)
{
	var wiki = document.getElementById('wiki');
	var e = document.getElementById(id);
	var sel = e.selectedIndex;

	if(sel != 0)
	{
		wiki.contentWindow.document.execCommand(id, false,
				e.options[sel].value);
		e.selectedIndex = 0;
	}
	wiki.contentWindow.focus();
}


function wikiSubmit()
{
	var wiki = document.getElementById('wiki');
	var wikitext = document.getElementById('wikitext');

	wikitext.value = wiki.contentWindow.document.body.innerHTML;
}
	//--></script>
	<div class="toolbar">
		Style: <select id="formatblock" onChange="wikiSelect(this.id)">
			<option value="<h1>">Heading 1</option>
			<option value="<h2>">Heading 2</option>
			<option value="<h3>">Heading 3</option>
			<option value="<h4>">Heading 4</option>
			<option value="<h5>">Heading 5</option>
			<option value="<h6>">Heading 6</option>
			<option value="<p>" selected="selected">Normal</option>
			<option value="<Pre>">Preformatted</option>
		</select>
		Font size: <select id="fontsize" unselectable="on" onChange="wikiSelect(this.id)">
			<option value="Size">Size</option>
			<option value="1">1</option>
			<option value="2">2</option>
			<option value="3">3</option>
			<option value="4">4</option>
			<option value="5">5</option>
			<option value="6">6</option>
		</select>
	</div>
	<form action="index.php" method="post" onSubmit="wikiSubmit()">
		<input type="hidden" name="module" value="wiki"/>
<?php if(!isset($wiki['id'])) { ?>
		<input type="hidden" name="action" value="insert"/>
<?php } else { ?>
		<input type="hidden" name="action" value="update"/>
		<input type="hidden" name="id" value="<?php echo _html_safe($wiki['id']); ?>"/>
<?php } ?>
		<table width="100%">
<?php if(!isset($wiki['id'])) { ?>
		<tr><td class="field">Title:</td><td><input type="text" name="title" value=""/></td></tr>
<?php } ?>
		<tr><td class="field hidden">Content:</td><td><textarea id="wikitext" class="hidden" name="content"><?php if(isset($wiki['content'])) echo _html_safe($wiki['content']); ?></textarea></td></tr>
		<tr><td colspan="2"><iframe id="wiki" width="100%" height="260px" onLoad="wikiStart()"></iframe></td></tr>
		<tr><td></td><td><input type="submit" name="preview" value="<?php echo _html_safe(PREVIEW); ?>"/> <input type="submit" name="send" value="<?php echo _html_safe(SUBMIT); ?>"/></td></tr>
		</table>
	</form>
</div>
