<form action="index.php" method="post">
	<input type="hidden" name="module" value="<? echo _html_safe($module); ?>"/>
	<input type="hidden" name="action" value="<? echo _html_safe($action); ?>"/>
	<table>
<? for($i = 0; $i < count($configs); $i++) { ?>
		<tr><td class="field"><? echo _html_safe($configs[$i]['name']); ?>:</td><td><input type="text" size="30" name="<? echo _html_safe($module.'_'.$configs[$i]['name']); ?>" value="<? echo _html_safe($configs[$i]['value']); ?>"/></td></tr>
<? } ?>
		<tr><td></td><td><input type="submit" value="<? echo _html_safe(UPDATE); ?>"/></td></tr>
	</table>
</form>
