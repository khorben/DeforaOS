<form action="index.php" method="post">
	<input type="hidden" name="module" value="<?php echo _html_safe($module); ?>"/>
	<input type="hidden" name="action" value="<?php echo _html_safe($action); ?>"/>
	<table>
<?php for($i = 0; $i < count($configs); $i++) { ?>
		<tr><td class="field"><?php echo _html_safe($configs[$i]['title']); ?>:</td><td><?php switch($configs[$i]['type'])
	{
		case 'bool': ?><input type="checkbox" name="<?php echo _html_safe($module.'_'.$configs[$i]['name']); ?>"<?php if(isset($configs[$i]['value']) && $configs[$i]['value'] != FALSE) { ?> checked="checked"<?php } ?>/><?php
		break;
		case 'string':
		default: ?><input type="text" size="30" name="<?php echo _html_safe($module.'_'.$configs[$i]['name']); ?>" value="<?php echo _html_safe($configs[$i]['value']); ?>"/><?php
		break; ?><?php } ?></td></tr>
<?php } ?>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(UPDATE); ?>" class="icon submit"/></td></tr>
	</table>
</form>
