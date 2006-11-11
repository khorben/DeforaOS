<h1 class="title probe"><?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="probe"/>
	<input type="hidden" name="action" value="<?php echo isset($host) ? 'host_update' : 'host_insert'; ?>"/>
<?php if(isset($host)) { ?>
	<input type="hidden" name="id" value="<?php if(isset($host['id'])) echo _html_safe($host['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field">Hostname&nbsp;:</td><td><input type="text" name="hostname" value="<?php if(isset($host['hostname'])) echo _html_safe($host['hostname']); ?>"/></td></tr>
		<tr><td class="field">Comment&nbsp;:</td><td><textarea name="comment"><?php if(isset($host['comment'])) echo _html_safe($host['comment']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo isset($host) ? 'Update' : 'Create'; ?>"/></td></tr>
	</table>
</form>
