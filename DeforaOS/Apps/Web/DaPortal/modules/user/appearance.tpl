<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="appearance"/>
	<table>
		<tr>
			<td class="field">Theme:</td>
			<td><select name="theme">
	<?php foreach($themes as $t) { ?>
				<option<?php if($theme == $t) { ?> selected="selected"<?php } ?>><?php echo $t; ?></option>
	<?php } ?>
			</select></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" value="Submit"/></td>
		</tr>
	</table>
</form>
