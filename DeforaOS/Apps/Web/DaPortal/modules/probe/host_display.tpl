<h1><img src="modules/probe/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<!-- FIXME hardcoded -->
<?
$graphs = array('uptime' => 'Uptime', 'load' => 'Load average',
		'ram' => 'Memory usage', 'swap' => 'Swap usage',
		'users' => 'Logged users', 'procs' => 'Process count',
		'eth0' => 'Network traffic');
$times = array('hour', 'day', 'week');
?>
<div class="toolbar">
	<a href="index.php?module=probe&amp;action=host_modify&amp;id=<? echo _html_safe($host['id']); ?>">Modify</a>
	· <a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>">Summary</a>
<? $keys = array_keys($graphs); foreach($keys as $k) { ?>
	· <a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($k); ?>" title="<? echo _html_safe($graphs[$k]); ?>"><? echo _html_safe($graphs[$k]); ?></a>
<? } ?>
</div>
<div class="comment"><? echo _html_safe($host['comment']); ?></div>
<center>
<table>
<? if(array_key_exists($args['graph'], $graphs)) { ?>
<tr><td><h2><? echo _html_safe($graphs[$args['graph']]); ?></h2></td></tr>
<? foreach($times as $t) { ?>
<tr><td><img src="<? echo _host_graph($host['hostname'], $args['graph'], $t); ?>" alt=""/></td></tr>
<? } ?>
<? } else { ?>
<? $keys = array_keys($graphs); for($i = 0, $cnt = count($keys); $i < $cnt; $i++) { ?>
	<tr>
		<td><h3><? echo _html_safe($graphs[$keys[$i]]); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($keys[$i]); ?>"><img src="<? echo _host_graph($host['hostname'], $keys[$i], 'hour'); ?>" alt=""/></a></td>
<? $i++; if($i == $cnt) { ?>
		<td></td>
	</tr>
<? break; } else { ?>
		<td><h3><? echo _html_safe($graphs[$keys[$i]]); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($keys[$i]); ?>"><img src="<? echo _host_graph($host['hostname'], $keys[$i], 'hour'); ?>" alt=""/></a></td>
	</tr>
<? } ?>
<? } ?>
<? } ?>
</table>
</center>
