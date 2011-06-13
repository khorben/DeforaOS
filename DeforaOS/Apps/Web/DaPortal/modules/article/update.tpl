<?php if(isset($title)) { ?>
<h1 class="title article"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<form class="article" action="index.php" method="post">
	<input type="hidden" name="module" value="article"/>
	<input type="hidden" name="action" value="<?php echo isset($article['id']) ? 'update' : 'submit'; ?>"/>
<?php if(isset($article['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($article['id']); ?>"/>
<?php } ?>
	<table>
		<tr>
			<td class="field"><?php echo _html_safe(TITLE); ?>:</td>
			<td><input type="text" name="title"<?php if(isset($article['title'])) print(' value="'._html_safe($article['title']).'"'); ?> size="50"/></td>
		</tr>
		<tr>
			<td class="field"><?php echo _html_safe(CONTENT); ?>:</td>
			<td><textarea name="content" cols="50" rows="10"><?php
if(isset($article['content'])) print(_html_safe($article['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><button type="submit" name="preview" class="icon preview"><?php echo _html_safe(PREVIEW); ?></button><?php if(isset($article)) { ?> <button type="submit" name="send" class="icon submit"><?php echo isset($article['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?></button><?php } ?></td>
		</tr>
	</table>
</form>
