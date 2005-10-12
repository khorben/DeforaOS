<? global $title, $module_name; $module = $module_name; ?>

<? _module('menu'); ?>
<div class="main">
<? if(strlen($module)) _module(); else _module('probe'); ?>
<? _debug(); ?>
</div>
