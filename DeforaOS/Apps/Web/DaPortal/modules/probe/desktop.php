<?php //modules/project/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Monitoring';
$admin = 1;
$list = 1;
$actions = array('host_list' => array('title' => 'Host list',
			'args' => '&action=host_list'));

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
