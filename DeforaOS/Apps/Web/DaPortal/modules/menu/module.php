<?php //modules/menu/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function _default_submenu($module, $level, $entry)
{
	for($i = 0; $i < $level; $i++)
		print('	');
	print('<li>');
	if(isset($entry['args']))
		print('<a href="index.php?module='.$module
				._html_safe_link($entry['args']).'">');
	print(_html_safe($entry['title']));
	if(isset($entry['args']))
		print('</a>');
	if(isset($entry['actions']) && is_array($entry['actions']))
	{
		print("<ul>\n");
		$actions =& $entry['actions'];
		$keys = array_keys($actions);
		foreach($keys as $k)
		{
			if(!is_array($actions[$k]))
			{
				_default_submenu($module, $level+1, array(
						'title' => $actions[$k],
						'args' => '&action='.$k));
				continue;
			}
			$entry = array('title' => $actions[$k]['title']);
			$entry['args'] = isset($actions[$k]['args'])
				? $actions[$k]['args'] : '&action='.$k;
			if(isset($actions[$k]['actions']))
				$entry['actions'] = $actions[$k]['actions'];
			_default_submenu($module, $level+1, $entry);
		}
		for($i = 0; $i < $level+1; $i++)
			print('	');
		print('</ul>');
	}
	print("</li>\n");
}


function menu_default()
{
	if(($modules = _sql_array('SELECT name FROM daportal_module'
			." WHERE enabled='1' ORDER BY name ASC;")) == FALSE)
		return _error('No modules to link to');
	print('<ul class="menu">'."\n");
	foreach($modules as $m)
	{
		$list = 0;
		$actions = FALSE;
		include('./modules/'.$m['name'].'/desktop.php');
		if(!$list)
			continue;
		_default_submenu($m['name'], 1, array('title' => $title,
					'args' => '', 'actions' => $actions));
	}
	print('</ul>'."\n");
}

?>
