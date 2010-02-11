<h1 class="title content"><?php echo _html_safe($title); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="content"/>
	<input type="hidden" name="action" value="update"/>
	<input type="hidden" name="id" value="<?php echo $id; ?>"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<?php echo _html_safe($content['title']); ?>" size="50"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(DATE); ?>:</td><td><input type="text" name="timestamp" value="<?php echo _html_safe($content['timestamp']); ?>" size="20"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(AUTHOR); ?>:</td><td><select name="user_id">
<?php foreach($users as $u) { ?>
				<option value="<?php echo _html_safe($u['user_id']); ?>"<?php if($content['user_id'] == $u['user_id']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($u['username']); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td class="field"><?php echo _html_safe(CONTENT); ?>:</td><td><textarea name="content" cols="50" rows="10"><?php echo _html_safe($content['content']); ?></textarea></td></tr>
		<tr><td class="field"><?php echo _html_safe(ENABLED); ?>:</td><td><select name="enabled">
				<option value="0"<?php if($content['enabled'] == SQL_FALSE) echo ' selected="selected"'; ?>><?php echo _html_safe(NO); ?></option>
				<option value="1"<?php if($content['enabled'] == SQL_TRUE) echo ' selected="selected"'; ?>><?php echo _html_safe(YES); ?></option>
			</select></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('content', 'admin'); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="reset" class="icon reset"><?php echo _html_safe(RESET); ?></button> <input type="submit" value="<?php echo _html_safe(UPDATE); ?>" class="icon submit"/></td></tr>
	</table>
</form>
