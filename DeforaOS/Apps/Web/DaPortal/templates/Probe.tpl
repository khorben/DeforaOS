<?php global $title, $module_name; $module = $module_name; ?>

<div class="container">
<?php _module('menu'); ?>
	<div class="main">
<?php if(strlen($module)) _module(); else _module('probe'); ?>
<?php _debug(); ?>
	</div>
	<div style="clear: both"></div>
</div>
