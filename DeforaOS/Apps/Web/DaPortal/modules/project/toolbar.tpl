<div class="toolbar">
	<a href="<?php echo _html_link('project', '', $id, $title); ?>" title="<?php echo _html_safe(HOMEPAGE); ?>"><span class="icon home"></span> <?php echo _html_safe(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'browse', $id, $title); ?>" title="<?php echo _html_safe(BROWSE_SOURCE); ?>"><span class="icon browse"></span> <?php echo _html_safe(BROWSE_SOURCE); ?></a>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'timeline', $id, $title); ?>" title="<?php echo _html_safe(TIMELINE); ?>"><span class="icon project"></span> <?php echo _html_safe(TIMELINE); ?></a>
<?php } ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'bug_list', $id, $title); ?>" title="<?php echo _html_safe(BUG_REPORTS); ?>"><span class="icon bug"></span> <?php echo _html_safe(BUG_REPORTS); ?></a>
<?php if(_module_id('category') != 0 && _module_id('download') != 0) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'download', $id, $title); ?>" title="<?php echo _html_safe(FILES); ?>"><span class="icon browse"></span> <?php echo _html_safe(FILES); ?></a>
<?php }
if($admin != 0) { ?>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('project', 'admin', $id, $title); ?>" title="Administration"><span class="icon admin"></span> <?php echo _html_safe(ADMINISTRATION); ?></a>
<?php } ?>
</div>
