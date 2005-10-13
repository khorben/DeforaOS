<? global $title, $module_name; $module = $module_name; ?>

<div class="container">
	<div class="menu">
<? _module('menu'); ?>
	</div>
	<div class="main">
<? if(strlen($module)) _module(); else _module('probe'); ?>
<? _debug(); ?>
	</div>
</div>
