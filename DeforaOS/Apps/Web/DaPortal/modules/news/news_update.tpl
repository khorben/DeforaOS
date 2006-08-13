<?php if(isset($title)) { ?>
<h1 class="news"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<form class="news" action="index.php" method="post">
	<input type="hidden" name="module" value="news"/>
	<input type="hidden" name="action" value="<?php echo isset($news['id']) ? 'update' : 'submit'; ?>"/>
<?php if(isset($news['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($news['id']); ?>"/>
<?php } ?>
	<table>
		<tr>
			<td class="field">Title:</td>
			<td><input type="text" name="title"<?php if(isset($news['title'])) print(' value="'._html_safe($news['title']).'"'); ?> size="50"/></td>
		</tr>
		<tr>
			<td class="field">Content:</td>
			<td><textarea name="content" cols="50" rows="10"><?php
if(isset($news['content'])) print(_html_safe($news['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" name="preview" value="Preview"/><?php if(isset($news)) { ?> <input type="submit" name="send" value="<?php echo isset($news['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/><?php } ?></td>
		</tr>
	</table>
</form>
