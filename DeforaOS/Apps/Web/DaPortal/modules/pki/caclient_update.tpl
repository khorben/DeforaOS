<div class="pki">
<?php if(isset($title)) { ?>
	<h1 class="title caclient"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<form action="<?php echo _html_link(); ?>" method="post">
		<input type="hidden" name="module" value="pki"/>
<?php if(isset($caclient['id'])) { ?>
		<input type="hidden" name="action" value="caclient_update"/>
		<input type="hidden" name="id" value="<?php echo _html_safe($caclient['id']); ?>"/>
<?php } else { ?>
		<input type="hidden" name="action" value="caclient_insert"/>
<?php if(isset($parent['id'])) { ?>
		<input type="hidden" name="parent" value="<?php echo _html_safe($parent['id']); ?>"/>
<?php } } ?>
		<table>
			<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<?php if(isset($caclient['title'])) echo _html_safe($caclient['title']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><input type="text" name="country" value="<?php if(isset($caclient['country'])) echo _html_safe($caclient['country']); ?>" size="2"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><input type="text" name="state" value="<?php if(isset($caclient['state'])) echo _html_safe($caclient['state']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><input type="text" name="locality" value="<?php if(isset($caclient['locality'])) echo _html_safe($caclient['locality']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><input type="text" name="organization" value="<?php if(isset($caclient['organization'])) echo _html_safe($caclient['organization']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><input type="text" name="section" value="<?php if(isset($caclient['section'])) echo _html_safe($caclient['section']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><input type="text" name="cn" value="<?php if(isset($caclient['cn'])) echo _html_safe($caclient['cn']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><input type="text" name="email" value="<?php if(isset($caclient['email'])) echo _html_safe($caclient['email']); ?>"/></td></tr>
			<tr><td></td><td><input type="submit" value="<?php echo isset($caclient['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
		</table>
	</form>
</div>
