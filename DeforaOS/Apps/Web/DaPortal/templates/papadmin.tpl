<?php global $title, $module_name, $user_id; ?>
<div id="container">
<?php if($user_id == 0) _module('user', 'login'); else { ?>
<?php _module('top'); ?>

<?php _module('menu'); ?>
	<div id="main">
<?php _module(); ?>
	</div>
<?php } ?>
<div style="clear: both"></div>
<? _debug(); ?>
</div>
