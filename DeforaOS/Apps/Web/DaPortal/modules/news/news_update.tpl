<form class="news" action="index.php" method="post">
	<input type="hidden" name="module" value="news"/>
	<input type="hidden" name="action" value="submit"/>
	<table>
		<tr>
			<td class="field">Title:</td>
			<td><input type="text" name="title"<? if(isset($news['title'])) print(' value="'._html_safe($news['title']).'"'); ?> size="80"/></td>
		</tr>
		<tr>
			<td class="field">Content:</td>
			<td><textarea name="content" cols="80" rows="20"><?
if(isset($news['content'])) print(_html_safe($news['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" name="preview" value="Preview"/><? if(isset($news)) { ?> <input type="submit" name="send" value="Send"/><? } ?></td>
		</tr>
	</table>
</form>
