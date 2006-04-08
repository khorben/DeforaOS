<? if(isset($title)) { ?>
<h1><img src="modules/article/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<? } ?>
<form class="article" action="index.php" method="post">
	<input type="hidden" name="module" value="article"/>
	<input type="hidden" name="action" value="<? echo isset($article['id']) ? 'update' : 'submit'; ?>"/>
<? if(isset($article['id'])) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($article['id']); ?>"/>
<? } ?>
	<table>
		<tr>
			<td class="field">Title:</td>
			<td><input type="text" name="title"<? if(isset($article['title'])) print(' value="'._html_safe($article['title']).'"'); ?> size="50"/></td>
		</tr>
		<tr>
			<td class="field">Content:</td>
			<td><textarea name="content" cols="50" rows="10"><?
if(isset($article['content'])) print(_html_safe($article['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" name="preview" value="Preview"/><? if(isset($article)) { ?> <input type="submit" name="send" value="<? echo isset($article['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/><? } ?></td>
		</tr>
	</table>
</form>
