<div class="toolbar">
	<a href="index.php?module=project&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(HOMEPAGE); ?>"><div class="icon home"></div> <?php echo _html_safe_link(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	&middot; <a href="index.php?module=project&amp;action=browse&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(BROWSE_SOURCE); ?>"><div class="icon browse"></div> <?php echo _html_safe_link(BROWSE_SOURCE); ?></a>
	&middot; <a href="index.php?module=project&amp;action=timeline&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(TIMELINE); ?>"><div class="icon project"></div> <?php echo _html_safe_link(TIMELINE); ?></a>
<?php } ?>
	&middot; <a href="index.php?module=project&amp;action=bug_list&amp;project_id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(BUG_REPORTS); ?>"><div class="icon bug"></div> <?php echo _html_safe_link(BUG_REPORTS); ?></a>
<?php if(_module_id('category') != 0 && _module_id('download') != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=download&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(FILES); ?>"><div class="icon browse"></div> <?php echo _html_safe_link(FILES); ?></a>
<?php }
if($admin != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=admin&amp;id=<?php echo _html_safe_link($id); ?>" title="Administration"><div class="icon admin"></div> Administration</a>
<?php if($enabled != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=disable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1" title="<?php echo _html_safe_link(DISABLE); ?>"><div class="icon disabled"></div> <?php echo _html_safe_link(DISABLE); ?></a>
<?php } else { ?>
	&middot; <a href="index.php?module=project&amp;action=enable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1" title="<?php echo _html_safe_link(ENABLE); ?>"><div class="icon enabled"></div> <?php echo _html_safe_link(ENABLE); ?></a>
<?php } } ?>
</div>
