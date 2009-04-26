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



//PRE	module, action and params are trusted
function _html_link($module, $action = FALSE, $id = FALSE, $title = FALSE,
		$params = FALSE)
{
	$link = _module_link($module, $action, $id, $title, $params);
	$link = htmlentities($link);
	$link = str_replace('&amp;amp;', '%26', $link);
	return $link;
}


//PRE  module, action and params are trusted
function _html_link_full($module, $action = FALSE, $id = FALSE, $title = FALSE,
		$params = FALSE)
{
	$link = _module_link_full($module, $action, $id, $title, $params);
	$link = htmlentities($link);
	$link = str_replace('&amp;amp;', '%26', $link);
	return $link;
}


function _html_paging($link, $page, $count)
{
	print('<div class="paging">'."\n");
	for($i = 1; $i <= $count; $i++)
	{
		print('	');
		if($i != 1)
			print('| ');
		if($i == $page)
			print($page."\n");
		else
			print('<a href="'.$link.$i.'">'.$i."</a>\n");
	}
	print("</div>\n");
}


function _html_pre($string)
{
	$strings = explode("\n", $string);
	$string = '';
	$list = 0;
	$pre = 0;
	for($i = 0, $cnt = count($strings); $i < $cnt; $i++)
	{
		$line = htmlentities($strings[$i]);
		if(strncmp($line, ' * ', 3) == 0)
		{
			if($list == 0)
			{
				$list = 1;
				if($pre == 1)
					$string.="</pre>\n";
				$pre = 0;
				$string.="<ul>\n";
			}
			$line = '<li>'.substr($line, 3)."</li>\n";
		}
		else if($list != 0)
		{
			$list = 0;
			$line ="</ul>\n".$line;
		}
		else if(isset($line[0]) && $line[0] == ' ')
		{
			$line = substr($line, 1);
			if($pre == 0)
			{
				$pre = 1;
				$string.='<pre>';
			}
		}
		else if($pre == 1)
		{
			$pre = 0;
			$string.="</pre>\n";
		}
		else
			$line.="<br/>\n";
		$line = preg_replace('/((ftp|http|https):'
				.'\/\/([-+a-zA-Z0-9.\/_%?=,;~#]|&amp;)+)/',
				'<a href="\1">\1</a>', $line);
		$string.=$line;
	}
	if($list != 0)
		$string.="</ul>\n";
	return $string;
}


function _html_safe($string)
{
	return htmlentities($string);
}


function _start_css_themes($theme)
{
	if(($dir = opendir(dirname($_SERVER['SCRIPT_FILENAME']).'/themes')) 
			== FALSE)
		return;
	while(($de = readdir($dir)))
	{
		if(($len = strlen($de)) < 5
				|| substr($de, -4) != '.css')
			continue;
		$name = substr($de, 0, $len-4);
		if($name == $theme)
			continue;
		print("\t\t".'<link rel="alternate stylesheet" href="'
				.rtrim(dirname($_SERVER['SCRIPT_NAME']), '/')
				.'/themes/'.$name.'.css" title="'.$name.'"/>'
				."\n");
	}
	closedir($dir);
}

function _html_start()
{
	global $title, $theme, $friendlylinks;

	readfile('./html/doctype.html');
	print('	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-15"/>
		<title>'._html_safe($title).'</title>
		<link rel="stylesheet" href="'
		.rtrim(dirname($_SERVER['SCRIPT_NAME']), '/').'/themes/'.$theme
		.'.css" title="'.$theme.'"/>'."\n");
	_start_css_themes($theme);
	if(is_readable($_SERVER['DOCUMENT_ROOT'].'/favicon.ico'))
		print('		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/>'."\n");
	if($friendlylinks == 1)
	{
		print("\t\t<base href=\"");
		if(isset($_SERVER['HTTPS']))
		{
			print('https://'.$_SERVER['SERVER_NAME']);
			if($_SERVER['SERVER_PORT'] != '443')
				print(':'.$_SERVER['SERVER_PORT']);
		}
		else
		{
			print('http://'.$_SERVER['SERVER_NAME']);
			if($_SERVER['SERVER_PORT'] != '80')
				print(':'.$_SERVER['SERVER_PORT']);
		}
		print(dirname($_SERVER['SCRIPT_NAME']).'"'."/>\n");
	}
	print("\t</head>\n\t<body>\n");
}


function _html_stop()
{
	print("\t</body>\n</html>");
}


function _html_tags($string, $tags = FALSE)
{
	if(!is_array($tags))
		$tags = array('b', '/b', 'br/', 'li', '/li', 'p', '/p', 'ul',
				'/ul');
	$string = _html_safe($string);
	foreach($tags as $t)
		if($t == 'a')
			$string = preg_replace(
'/&lt;a href=&quot;(([-a-zA-Z0-9:._+%~\/?=]|&amp;)+)&quot;&gt;/',
					'<a href="\1">', $string);
		else if($t == 'img')
			$string = preg_replace(
'/&lt;img src=&quot;(.*)&quot; alt=&quot;(.*)&quot;\/&gt;/',
				'<img src="\1" alt="\2"/>', $string);
		else
			$string = str_replace("&lt;$t&gt;", "<$t>", $string);
	return $string;
}


function _html_template($template)
{
	if(!include('./templates/'.$template.'.tpl'))
		return _error('Could not include template');
}

?>
