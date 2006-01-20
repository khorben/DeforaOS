<h1><img src="modules/probe/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<div class="toolbar">
	<a href="index.php?module=probe&amp;action=host_modify&amp;id=<? echo _html_safe($host['id']); ?>">Modify</a>
	· <a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>">Summary</a>
<? foreach($graphs as $g) { ?>
	· <a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($g['graph']); ?>" title="<? echo _html_safe($graphs[$k]); ?>"><? echo _html_safe($g['title']); ?></a>
<? } ?>
</div>
<div class="comment"><? echo _html_safe($host['comment']); ?></div>
<center>
<table>
<? $i = 0; foreach($graphs as $g) { ?>
<? if(isset($graph) && $g['graph'] != $graph) continue; ?>
<? if(isset($graph) || !$i % 2) { ?>
	<tr>
<? } ?>
		<td><h3><? echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($g['graph']); ?>"><img src="<? echo _host_graph($host['hostname'], $g['graph'], 'hour', $g['param']); ?>" alt=""/></a></td>
		<td></td>
<? if(isset($graph)) { ?>
	</tr>
	<tr>
		<td><h3><? echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($g['graph']); ?>"><img src="<? echo _host_graph($host['hostname'], $g['graph'], 'day', $g['param']); ?>" alt=""/></a></td>
		<td></td>
	</tr>
	<tr>
		<td><h3><? echo _html_safe($g['title']); ?></h3><a href="index.php?module=probe&amp;action=host_display&amp;id=<? echo _html_safe($host['id']); ?>&amp;graph=<? echo _html_safe($g['graph']); ?>"><img src="<? echo _host_graph($host['hostname'], $g['graph'], 'week', $g['param']); ?>" alt=""/></a></td>
		<td></td>
<? } ?>
<? if(isset($graph) || $i % 2) { ?>
	</tr>
<? } ?>
<? $i++; } ?>
</table>
</center>
