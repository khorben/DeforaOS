<?
function _sort($module, $action, $args, $class, $sort, $name)
{
	if($class == $sort)
	{
		print(_html_safe($name));
		print('<img src="modules/explorer/sort.png" alt="sort"/>');
		return;
	}
	print('<a href="index.php?');
	$link = 'module='.$module.'&action='.$action.$args
			.'&sort='.$class;
	print(_html_safe($link).'">'._html_safe($name).'</a>');
}
?>
	<div id="explorer_<? echo $explorer_id; ?>" class="listing_<? echo _html_safe($view); ?>">
		<div class="header">
			<div class="icon"></div>
			<div class="name"><? if($args['sort']) _sort($args['module'], $args['action'], $args['args'], 'name', $args['sort'], 'Name'); else { ?>Name<? } ?></div>
<? if(isset($args['class'])) { $keys = array_keys($args['class']); foreach($keys as $k) { ?>
			<div class="<? echo _html_safe($k); ?>"><? if($args['sort']) _sort($args['module'], $args['action'], $args['args'], $k, $args['sort'], $args['class'][$k]); else { echo _html_safe($args['class'][$k]); } ?></div>
<? } } ?>
		</div>
