<?php global $title, $module_name, $user_id; ?>
<?php _module('top'); ?>
<div id="container">
<?php if($user_id == 0) _module('user', 'login'); else { ?>
<?php _module('menu'); ?>
	<div id="main">
<?php _module(); ?>
	</div>
<?php } ?>
<div style="clear: both"></div>
<?php _debug(); ?>
</div>
