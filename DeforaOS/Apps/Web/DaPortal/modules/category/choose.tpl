<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="category"/>
	<input type="hidden" name="action" value="link_insert_new"/>
	<input type="hidden" name="content_id" value="<?php echo _html_safe($args['id']); ?>"/>
	<span class="field">Or enter one directly:</span> <input type="text" name="title" size="15"/> <button type="submit" class="icon add"><?php echo _html_safe(ADD); ?></button> <a href="<?php echo _html_link($module, FALSE, $id, $title); ?>"><button class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a>
</form>
