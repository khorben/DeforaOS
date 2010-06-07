<?php global $user_id;
if($user_id != 0 || _config_get('wiki', 'anonymous') == TRUE) { ?>
<form action="index.php" method="get">
	<input type="hidden" name="module" value="wiki"/>
	<input type="hidden" name="action" value="insert"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(CREATE_A_PAGE); ?>:</td><td><input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(CREATE); ?>" class="icon submit"/></td></tr>
	</table>
</form>
<?php } ?>
<form action="index.php" method="get">
	<input type="hidden" name="module" value="wiki"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(LOOK_FOR_A_PAGE); ?>:</td><td><input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(SEARCH); ?>" class="icon search"/></td></tr>
	</table>
</form>
<form action="index.php" method="get">
	<input type="hidden" name="module" value="search"/>
	<input type="hidden" name="action" value="advanced"/>
	<input type="hidden" name="incontent" value="1"/>
	<input type="hidden" name="inmodule" value="wiki"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(LOOK_INSIDE_PAGES); ?>:</td><td><input type="text" name="q"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(SEARCH); ?>" class="icon search"/></td></tr>
	</table>
</form>
