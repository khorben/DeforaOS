<?php //modules/menu/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function menu_default()
{
	if(($modules = _sql_array('SELECT name FROM daportal_module'
			." WHERE enabled='1' ORDER BY name ASC;")) == FALSE)
		return error('No modules to link to');
	print('<ul class="menu">'."\n");
	foreach($modules as $m)
	{
		$title = $m['name'];
		$list = 0;
		$actions = FALSE;
		include('./modules/'.$m['name'].'/desktop.php');
		if(!$list)
			continue;
		print("\t".'<li><a href="index.php?module='.$m['name']
				.'">'.$title.'</a>');
		if(is_array($actions) && count($actions))
		{
			print("<ul>\n");
			$keys = array_keys($actions);
			foreach($keys as $k)
				print("\t\t\t".'<li><a href="index.php?module='
						.$m['name'].'&amp;action='
						._html_safe_link($k).'">'
						._html_safe($actions[$k])
						.'</a></li>'."\n");
			print("\t\t</ul>");
		}
		print('</li>'."\n");
	}
	print('</ul>'."\n");
}

?>
