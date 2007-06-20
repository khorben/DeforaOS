<div class="toolbar">
	<a href="<?php echo _html_link('project', '', $id); ?>" title="<?php echo _html_safe(HOMEPAGE); ?>"><div class="icon home"></div> <?php echo _html_safe(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	&middot; <a href="<?php echo _html_link('project', 'browse', $id); ?>" title="<?php echo _html_safe(BROWSE_SOURCE); ?>"><div class="icon browse"></div> <?php echo _html_safe(BROWSE_SOURCE); ?></a>
	&middot; <a href="<?php echo _html_link('project', 'timeline', $id); ?>" title="<?php echo _html_safe(TIMELINE); ?>"><div class="icon project"></div> <?php echo _html_safe(TIMELINE); ?></a>
<?php } ?>
	&middot; <a href="<?php echo _html_link('project', 'bug_list', '', '', 'project_id='.$id); ?>" title="<?php echo _html_safe(BUG_REPORTS); ?>"><div class="icon bug"></div> <?php echo _html_safe(BUG_REPORTS); ?></a>
<?php if(_module_id('category') != 0 && _module_id('download') != 0) { ?>
	&middot; <a href="<?php echo _html_link('project', 'download', $id); ?>" title="<?php echo _html_safe(FILES); ?>"><div class="icon browse"></div> <?php echo _html_safe(FILES); ?></a>
<?php }
if($admin != 0) { ?>
	&middot; <a href="<?php echo _html_link('project', 'admin', $id); ?>" title="Administration"><div class="icon admin"></div> Administration</a>
<?php } ?>
</div>
