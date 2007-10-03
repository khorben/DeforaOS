<div class="toolbar">
	<a href="<?php echo _html_link('project', '', $id, $title); ?>" title="<?php echo _html_safe(HOMEPAGE); ?>"><div class="icon home"></div> <?php echo _html_safe(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'browse', $id, $title); ?>" title="<?php echo _html_safe(BROWSE_SOURCE); ?>"><div class="icon browse"></div> <?php echo _html_safe(BROWSE_SOURCE); ?></a>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'timeline', $id, $title); ?>" title="<?php echo _html_safe(TIMELINE); ?>"><div class="icon project"></div> <?php echo _html_safe(TIMELINE); ?></a>
<?php } ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'bug_list', '', '', 'project_id='.$id); ?>" title="<?php echo _html_safe(BUG_REPORTS); ?>"><div class="icon bug"></div> <?php echo _html_safe(BUG_REPORTS); ?></a>
<?php if(_module_id('category') != 0 && _module_id('download') != 0) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'download', $id, $title); ?>" title="<?php echo _html_safe(FILES); ?>"><div class="icon browse"></div> <?php echo _html_safe(FILES); ?></a>
<?php }
if($admin != 0) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'admin', $id, $title); ?>" title="Administration"><div class="icon admin"></div> <?php echo _html_safe(ADMINISTRATION); ?></a>
<?php } ?>
</div>
