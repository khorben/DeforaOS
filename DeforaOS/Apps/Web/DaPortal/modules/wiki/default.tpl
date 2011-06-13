<table>
<?php global $user_id;
if($user_id != 0 || _config_get('wiki', 'anonymous') == TRUE) { ?>
	<tr>
		<td class="field"><?php echo _html_safe(CREATE_A_PAGE); ?>:</td>
		<td>
			<form action="<?php echo _html_link(); ?>" method="get">
				<input type="hidden" name="module" value="wiki"/>
				<input type="hidden" name="action" value="insert"/>
				<input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/>
				<button type="submit" class="icon submit"><?php echo _html_safe(CREATE); ?></button>
			</form>
		</td>
	</tr>
<?php } ?>
	<tr>
		<td class="field"><?php echo _html_safe(LOOK_FOR_A_PAGE); ?>:</td>
		<td>
			<form action="<?php echo _html_link(); ?>" method="get">
				<input type="hidden" name="module" value="wiki"/>
				<input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/>
				<button type="submit" class="icon search"><?php echo _html_safe(SEARCH); ?></button>
			</form>
		</td>
	</tr>
	<tr>
		<td class="field"><?php echo _html_safe(LOOK_INSIDE_PAGES); ?>:</td>
		<td>
			<form action="<?php echo _html_link(); ?>" method="get">
				<input type="hidden" name="module" value="search"/>
				<input type="hidden" name="action" value="advanced"/>
				<input type="text" name="q"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/>
				<input type="hidden" name="incontent" value="1"/>
				<input type="hidden" name="inmodule" value="wiki"/>
				<button type="submit" class="icon search"><?php echo _html_safe(SEARCH); ?></button>
			</form>
		</td>
	</tr>
</table>
