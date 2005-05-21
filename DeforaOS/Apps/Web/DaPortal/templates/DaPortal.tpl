<? global $title, $module_name; ?>
<? echo "$title :: ".(strlen($module_name) ? $module_name : "Homepage"); ?>

<? _module('menu'); ?>
<? _module(); ?>
<? _debug(); ?>
