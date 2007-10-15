<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));


function _default_submenu($module, $level, $entry)
{
	for($i = 0; $i < $level; $i++)
		print('	');
	print('<li>');
	if(isset($entry['args']))
	{
		$args = array();
		parse_str($entry['args'], $args);
		print('<a href="'._html_link(isset($args['module'])
			? $args['module'] : $module, isset($args['action'])
			? $args['action'] : FALSE, isset($args['id'])
			? $args['id'] : FALSE, FALSE, $entry['args']).'">');
	}
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
						'args' => 'action='.$k));
				continue;
			}
			$entry = array('title' => $actions[$k]['title']);
			$entry['args'] = isset($actions[$k]['args'])
				? $actions[$k]['args'] : 'action='.$k;
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
	global $user_id;

	$enabled = " WHERE enabled='1'";
	if(_user_admin($user_id))
		$enabled = '';
	if(($modules = _sql_array('SELECT name FROM daportal_module'.$enabled
			.' ORDER BY name ASC')) == FALSE)
		return _error('No modules to link to');
	print('<ul class="menu">'."\n");
	foreach($modules as $m)
	{
		if(($d = _module_desktop($m['name'])) == FALSE
				|| $d['list'] != 1)
			continue;
		$d['args'] = '';
		_default_submenu($m['name'], 1, $d);
	}
	print('</ul>'."\n");
}

?>
