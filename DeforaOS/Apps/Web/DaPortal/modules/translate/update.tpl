<h1 class="title translate"><?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="translate"/>
	<input type="hidden" name="action" value="update"/>
	<input type="hidden" name="id" value="<?php echo $content['id']; ?>"/>
	<input type="hidden" name="lang" value="<?php echo $translate['lang_id']; ?>"/>
	<table>
		<th colspan="2"><?php echo _html_safe(ORIGINAL_CONTENT); ?></th>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><?php echo _html_safe($content['title']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(CONTENT); ?>:</td><td><?php echo _html_safe($content['content']); ?></td></tr>
		<th colspan="2"><?php echo _html_safe(TRADUCTION); ?></th>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" size="50" value="<?php echo _html_safe($translate['title']); ?>"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(CONTENT); ?>:</td><td><textarea name="content" cols="50" rows="10"><?php echo _html_safe($translate['content']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(TRANSLATE); ?>"/></td></tr>
	</table>
</form>
