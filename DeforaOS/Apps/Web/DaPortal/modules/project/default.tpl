<h1 class="title project"><?php echo _html_safe(PROJECTS); ?></h1>
<h2 class="title stats"><?php echo _html_safe(STATISTICS); ?></h2>
<p>There are <?php echo $project_cnt; ?> <a href="<?php echo _html_link('project', 'list'); ?>">projects registered</a>.</p>
<?php $keys = array_keys($cols); foreach($keys as $k) { ?>
<a name="<?php echo _html_safe($k); ?>"></a><h3>Bugs by <?php echo _html_safe($cols[$k]); ?></h3>
<table class="bordered">
	<tr>
		<th class="field" colspan="2"></th>
<?php $enums = _sql_enum('daportal_bug', $k); foreach($enums as $e) { ?>
		<th class="field" style="width: <?php echo ceil(100 / (count($enums) + 1)); ?>%"><?php echo _html_safe($e); ?></th>
<?php } ?>
	</tr>
	<tr>
		<th class="field" colspan="2">All</th>
<?php foreach($enums as $e) { ?>
		<td><a href="<?php echo _html_link('project', 'bug_list', FALSE, FALSE, $k.'='.$e); ?>"><?php $cnt = _sql_single('SELECT COUNT(*) FROM daportal_bug WHERE '.$k."='$e'"); echo _html_safe($cnt); ?></a></td>
<?php } ?>
	</tr>
<?php foreach($keys as $l) { if($l == $k) continue; $by = _sql_enum('daportal_bug', $l); $idx = 1; foreach($by as $f) { ?>
	<tr>
<?php if($idx) { $idx = 0; ?>
		<th rowspan="<?php echo count($by); ?>"><?php echo _html_safe($cols[$l]); ?></th>
<?php } ?>
		<th><?php echo _html_safe($f); ?></th>
<?php foreach($enums as $e) { ?>
		<td><a href="<?php echo _html_link('project', 'bug_list', FALSE, FALSE, $k.'='.$e.'&'.$l.'='.$f); ?>"><?php $cnt = _sql_single('SELECT COUNT(*) FROM daportal_bug WHERE '.$k."='$e' AND ".$l."='$f'"); echo _html_safe($cnt); ?></a></td>
<?php } ?>
	</tr>
<?php } ?>
<?php } ?>
</table>
<?php } ?>
