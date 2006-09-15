<div class="toolbar">
	<a href="index.php?module=project&amp;id=<?php echo _html_safe_link($id); ?>"><img src="modules/user/home.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	&middot; <a href="index.php?module=project&amp;action=browse&amp;id=<?php echo _html_safe_link($id); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(BROWSE_SOURCE); ?></a>
	&middot; <a href="index.php?module=project&amp;action=timeline&amp;id=<?php echo _html_safe_link($id); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(TIMELINE); ?></a>
<?php } ?>
	&middot; <a href="index.php?module=project&amp;action=bug_list&amp;project_id=<?php echo _html_safe_link($id); ?>"><img src="modules/project/bug.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(BUG_REPORTS); ?></a>
<?php if($admin != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=admin&amp;id=<?php echo _html_safe_link($id); ?>"><img src="modules/admin/icon.png" alt="" style="height: 16px; width: 16px"/> Administration</a>
<?php if($enabled != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=disable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1"><img src="icons/16x16/disabled.png" alt=""/> Disable</a>
<?php } else { ?>
	&middot; <a href="index.php?module=project&amp;action=enable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1"><img src="icons/16x16/enabled.png" alt=""/> Enable</a>
<?php } } ?>
</div>
