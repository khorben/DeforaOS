<form action="index.php" method="post">
	<input type="hidden" name="module" value="admin"/>
	<input type="hidden" name="action" value="config"/>
	<table>
<? $module = ''; for($i = 0; $i < count($configs); $i++) { ?>
<? if($configs[$i]['module'] != $module) { $module = $configs[$i]['module']; ?>
		<tr><td colspan="2"><h2><img src="modules/<? echo _html_safe($module); ?>/icon.png" alt=""/> <? echo _html_safe($module); ?></h2></td></tr>
<? } ?>
		<tr><td class="field"><? echo _html_safe($configs[$i]['name']); ?>:</td><td><input type="text" size="30" name="<? echo _html_safe($configs[$i]['module'].'_'.$configs[$i]['name']); ?>" value="<? echo _html_safe($configs[$i]['value']); ?>"/></td></tr>
<? } ?>
		<tr><td></td><td><input type="submit" value="<? echo _html_safe(UPDATE); ?>"/></td></tr>
	</table>
</form>
