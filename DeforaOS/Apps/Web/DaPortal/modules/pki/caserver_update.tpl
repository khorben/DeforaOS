<div class="pki">
<?php if(isset($title)) { ?>
	<h1 class="title caserver"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<form action="<?php echo _html_link(); ?>" method="post">
		<input type="hidden" name="module" value="pki"/>
<?php if(isset($caserver['id'])) { ?>
		<input type="hidden" name="action" value="caserver_update"/>
		<input type="hidden" name="id" value="<?php echo _html_safe($caserver['id']); ?>"/>
<?php } else { ?>
		<input type="hidden" name="action" value="caserver_insert"/>
<?php if(isset($parent['id'])) { ?>
		<input type="hidden" name="parent" value="<?php echo _html_safe($parent['id']); ?>"/>
<?php } } ?>
		<table>
			<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<?php if(isset($caserver['title'])) echo _html_safe($caserver['title']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><input type="text" name="country" value="<?php if(isset($caserver['country'])) echo _html_safe($caserver['country']); ?>" size="2"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><input type="text" name="state" value="<?php if(isset($caserver['state'])) echo _html_safe($caserver['state']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><input type="text" name="locality" value="<?php if(isset($caserver['locality'])) echo _html_safe($caserver['locality']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><input type="text" name="organization" value="<?php if(isset($caserver['organization'])) echo _html_safe($caserver['organization']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><input type="text" name="section" value="<?php if(isset($caserver['section'])) echo _html_safe($caserver['section']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><input type="text" name="cn" value="<?php if(isset($caserver['cn'])) echo _html_safe($caserver['cn']); ?>"/></td></tr>
			<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><input type="text" name="email" value="<?php if(isset($caserver['email'])) echo _html_safe($caserver['email']); ?>"/></td></tr>
			<tr><td></td><td><input type="submit" value="<?php echo isset($caserver['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>" class="icon submit"/></td></tr>
		</table>
	</form>
</div>
