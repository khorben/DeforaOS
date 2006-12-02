<?php //modules/project/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Monitoring';
$admin = 1;
$list = 1;
$actions = array('graph_list' => array('title' => 'Graphs'),
		'host_list' => array('title' => 'Hosts',
			'args' => '&action=host_list'));

$actions['graph_list']['actions'] = array();
$actions['graph_list']['actions'][] = array('title' => 'Uptime',
		'args' => '&type=uptime');
$actions['graph_list']['actions'][] = array('title' => 'Load average',
		'args' => '&type=load');
$actions['graph_list']['actions'][] = array('title' => 'Memory usage',
		'args' => '&type=ram');
$actions['graph_list']['actions'][] = array('title' => 'Swap usage',
		'args' => '&type=swap');
$actions['graph_list']['actions'][] = array('title' => 'Logged users',
		'args' => '&type=users');
$actions['graph_list']['actions'][] = array('title' => 'Process count',
		'args' => '&type=procs');
$actions['graph_list']['actions'][] = array('title' => 'Network traffic',
		'args' => '&type=iface');
$actions['graph_list']['actions'][] = array('title' => 'Volume usage',
		'args' => '&type=vol');

$hosts = _sql_array('SELECT title, host_id AS id'
		.' FROM daportal_probe_host, daportal_content'
		." WHERE enabled='1' AND"
		.' daportal_probe_host.host_id=daportal_content.content_id'
		.' ORDER BY title ASC');
if(is_array($hosts))
{
	$actions['host_list']['actions'] = array();
	foreach($hosts as $h)
		$actions['host_list']['actions'][] = array(
				'title' => $h['title'],
				'args' => '&id='.$h['id']);
}

?>
