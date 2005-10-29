<? if(isset($title)) { ?>
<h1><img src="modules/news/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<? } ?>
<form class="news" action="index.php" method="post">
	<input type="hidden" name="module" value="news"/>
	<input type="hidden" name="action" value="<? echo isset($news['id']) ? 'update' : 'submit'; ?>"/>
<? if(isset($news['id'])) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($news['id']); ?>"/>
<? } ?>
	<table>
		<tr>
			<td class="field">Title:</td>
			<td><input type="text" name="title"<? if(isset($news['title'])) print(' value="'._html_safe($news['title']).'"'); ?> size="80"/></td>
		</tr>
		<tr>
			<td class="field">Content:</td>
			<td><textarea name="content" cols="80" rows="15"><?
if(isset($news['content'])) print(_html_safe($news['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" name="preview" value="Preview"/><? if(isset($news)) { ?> <input type="submit" name="send" value="<? echo isset($news['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/><? } ?></td>
		</tr>
	</table>
</form>
