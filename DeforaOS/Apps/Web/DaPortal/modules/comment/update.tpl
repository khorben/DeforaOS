<a name="edit"></a>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="comment"/>
	<input type="hidden" name="action" value="<?php echo isset($comment['id']) ? 'update' : 'submit'; ?>"/>
<?php if(isset($parent)) { ?>
	<input type="hidden" name="parent" value="<?php echo _html_safe($parent); ?>"/>
<?php } ?>
	<table>
		<tr>
			<td class="field">Title&nbsp;:</td>
			<td><input type="text" name="title" value="<?php echo _html_safe($comment['title']); ?>" size="80"/></td>
		</tr>
		<tr>
			<td class="field">Content&nbsp;:</td>
			<td><textarea name="content" cols="80" rows="15"><?php echo _html_safe($comment['content']); ?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" name="preview" value="Preview"/> <input type="submit" name="submit" value="<?php echo _html_safe(SEND); ?>"/></td>
		</tr>
	</table>
</form>
