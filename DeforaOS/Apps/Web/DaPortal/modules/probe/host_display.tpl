<h1 class="title probe"><?php echo _html_safe($title); ?></h1>
<div class="toolbar">
	<a href="index.php?module=probe&amp;action=host_display&amp;id=<?php echo _html_safe($host['id']); ?>">Summary</a>
<?php foreach($graphs as $g) { ?>
	&middot; <a href="index.php?module=probe&amp;action=host_display&amp;id=<?php echo _html_safe($host['id']); ?>&amp;graph=<?php echo _html_safe($g['graph']); ?>" title="<?php echo _html_safe($g['title']); ?>"><?php echo _html_safe($g['title']); ?></a>
<?php } ?>
</div>
<div style="padding: 4px">
<?php global $user_id; require_once('system/user.php'); if(_user_admin($user_id) == 1) { ?>
	<a href="index.php?module=probe&amp;action=host_modify&amp;id=<?php echo _html_safe($host['id']); ?>"><input type="button" value="Modify" style="padding-right: 4px"/></a>
<?php } ?>
	View last: <form action="index.php" method="get" style="display: inline">
		<input type="hidden" name="module" value="probe"/>
		<input type="hidden" name="action" value="host_display"/>
		<input type="hidden" name="id" value="<?php echo $args['id']; ?>"/>
		<select name="time" onchange="submit()">
			<option value="hour"<?php if($time == 'hour') { ?> selected="selected"<?php } ?>>hour</option>
			<option value="day"<?php if($time == 'day') { ?> selected="selected"<?php } ?>>day</option>
			<option value="week"<?php if($time == 'week') { ?> selected="selected"<?php } ?>>week</option>
		</select>
		<input id="probe_view" type="submit" value="View"/>
		<script type="text/javascript"><!--
document.getElementById('probe_view').style.display='none';
//--></script>
	</form>
</div>
<div class="comment"><?php echo _html_safe($host['comment']); ?></div>
<center>
<table>
<?php $i = 0; foreach($graphs as $g) {
	if(isset($graph) && $g['graph'] != $graph) continue;
	if(isset($graph) || !$i % 2) { ?>
	<tr>
<?php } ?>
		<td><h3><?php echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<?php echo _html_safe($host['id']); ?>&amp;graph=<?php echo _html_safe($g['graph']); ?>"><img src="<?php echo _host_graph($host['hostname'], $g['graph'], $g['time'], isset($g['param']) ? $g['param'] : ''); ?>" alt=""/></a></td>
		<td></td>
<?php if(isset($graph)) { ?>
	</tr>
	<tr>
		<td><h3><?php echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<?php echo _html_safe($host['id']); ?>&amp;graph=<?php echo _html_safe($g['graph']); ?>"><img src="<?php echo _host_graph($host['hostname'], $g['graph'], 'day', isset($g['param']) ? $g['param'] : ''); ?>" alt=""/></a></td>
		<td></td>
	</tr>
	<tr>
		<td><h3><?php echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<?php echo _html_safe($host['id']); ?>&amp;graph=<?php echo _html_safe($g['graph']); ?>"><img src="<?php echo _host_graph($host['hostname'], $g['graph'], 'week', isset($g['param']) ? $g['param'] : ''); ?>" alt=""/></a></td>
		<td></td>
<?php }
if(isset($graph) || $i % 2) { ?>
	</tr>
<?php }
$i++; } ?>
</table>
</center>
