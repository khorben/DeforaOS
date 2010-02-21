<?php if(isset($title)) { ?>
<h1 class="title blog"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<form class="blog" action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="blog"/>
	<input type="hidden" name="action" value="<?php echo isset($post['id']) ? 'update' : 'insert'; ?>"/>
<?php if(isset($post['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($post['id']); ?>"/>
<?php } ?>
	<table>
		<tr>
			<td class="field"><?php echo _html_safe(TITLE); ?>:</td>
			<td><input type="text" name="title"<?php if(isset($post['title'])) print(' value="'._html_safe($post['title']).'"'); ?> size="50"/></td>
		</tr>
		<?php if(isset($post['timestamp'])) { ?>
		<tr>
			<td class="field"><?php echo _html_safe(DATE); ?>:</td>
			<td><input type="text" name="timestamp" value="<?php echo(_html_safe($post['timestamp'])); ?>" size="20"/></td>
		</tr>
		<?php } ?>
		<tr>
			<td class="field"><?php echo _html_safe(CONTENT); ?>:</td>
			<td><textarea name="content" cols="50" rows="10"><?php
if(isset($post['content'])) print(_html_safe($post['content']));
			?></textarea></td>
		</tr>
		<tr>
			<td></td>
			<td><a href="<?php echo _html_link('blog', FALSE, isset($post['id']) ? $post['id'] : FALSE, isset($post['id']) ? $post['title'] : FALSE); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" name="preview" value="<?php echo _html_safe(PREVIEW); ?>" class="icon preview"/><?php if(isset($post)) { ?> <input type="submit" name="send" value="<?php echo isset($post['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>" class="icon submit"/><?php } ?></td>
		</tr>
	</table>
</form>
