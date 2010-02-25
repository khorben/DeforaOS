<h1 class="title top"><?php echo _html_safe($title); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="top"/>
	<input type="hidden" name="action" value="<?php echo _html_safe($action); ?>"/>
<?php if(isset($top)) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($top['top_id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="name" value="<?php if(isset($top['name'])) echo _html_safe($top['name']); ?>" size="40"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(ADDRESS); ?>:</td><td><input type="text" name="link" value="<?php if(isset($top['link'])) echo _html_safe($top['link']); ?>" size="40"/></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('top', 'admin'); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" value="<?php echo isset($top) ? UPDATE : CREATE; ?>" class="icon submit"/>
	</table>
</form>
