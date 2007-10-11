<div class="pki">
<?php if(isset($title)) { ?>
	<h1 class="title ca"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<form action="index.php" method="post">
		<input type="hidden" name="module" value="pki"/>
<?php if(isset($ca['id'])) { ?>
		<input type="hidden" name="action" value="ca_update"/>
		<input type="hidden" name="id" value="<?php echo _html_safe($ca['id']); ?>"/>
<?php } else { ?>
		<input type="hidden" name="action" value="ca_insert"/>
<?php } ?>
		<table>
<?php if(isset($parent) && is_array($parent)) { ?>
			<tr><td class="field">Parent:</td><td><select name="parent">
					<option value="">Self-signed</option>
<?php foreach($parent as $p) { ?>
					<option value="<?php echo _html_safe($p['id']); ?>"<?php if(isset($parent_id) && $parent_id == $p['id']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($p['title']); ?></option>
<?php } ?>
				</select></td></tr>
<?php } ?>
			<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<?php if(isset($ca['title'])) echo _html_safe($ca['title']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><input type="text" name="country" value="<?php if(isset($ca['country'])) echo _html_safe($ca['country']); ?>" size="2"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><input type="text" name="state" value="<?php if(isset($ca['state'])) echo _html_safe($ca['state']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><input type="text" name="locality" value="<?php if(isset($ca['locality'])) echo _html_safe($ca['locality']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><input type="text" name="organization" value="<?php if(isset($ca['organization'])) echo _html_safe($ca['organization']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><input type="text" name="section" value="<?php if(isset($ca['section'])) echo _html_safe($ca['section']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><input type="text" name="cn" value="<?php if(isset($ca['cn'])) echo _html_safe($ca['cn']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><input type="text" name="email" value="<?php if(isset($ca['email'])) echo _html_safe($ca['email']); ?>"/></td></tr>
			<tr><td></td><td><input type="submit" value="<?php echo isset($ca['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
		</table>
	</form>
</div>
