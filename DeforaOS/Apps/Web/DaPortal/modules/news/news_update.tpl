<?php if(isset($title)) { ?>
<h1 class="title news"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<form class="news" action="index.php" method="post">
	<input type="hidden" name="module" value="news"/>
	<input type="hidden" name="action" value="<?php echo isset($news['id']) ? 'update' : 'submit'; ?>"/>
<?php if(isset($news['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($news['id']); ?>"/>
<?php } ?>
	<table>
		<tr>
			<td class="field"><?php echo _html_safe(TITLE); ?>:</td>
			<td><input type="text" name="title"<?php if(isset($news['title'])) print(' value="'._html_safe($news['title']).'"'); ?> size="50"/></td>
		</tr>
		<tr>
			<td class="field"><?php echo _html_safe(CONTENT); ?>:</td>
			<td><textarea name="content" cols="50" rows="10"><?php
if(isset($news['content'])) print(_html_safe($news['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><a href="<?php echo _html_link('news'); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="submit" name="preview" class="icon preview"><?php echo _html_safe(PREVIEW); ?></button><?php if(isset($news)) { ?> <button type="submit" name="send" class="icon submit"><?php echo isset($news['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?></button><?php } ?></td>
		</tr>
	</table>
</form>
