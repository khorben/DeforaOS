<h1 class="title bookmark"><?php echo _html_safe($bookmark['title']); ?></h1>
<table>
	<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><?php echo _html_safe($bookmark['title']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(ADDRESS); ?>:</td><td><a href="<?php echo _html_safe($bookmark['url']); ?>"><?php echo _html_safe($bookmark['url']); ?></a></td></tr>
<?php if($user_id == $bookmark['user_id']) { ?>
	<tr><td></td><td><a href="<?php echo _html_link('bookmark', 'modify', $bookmark['id'], $bookmark['title']); ?>"><button type="button" class="icon edit"><?php echo _html_safe(MODIFY); ?></button></a></td></tr>
<?php } ?>
</table>
<?php if(_module_id('category')) {
	_module('category', 'get', array('id' => $bookmark['id']));
} ?>
