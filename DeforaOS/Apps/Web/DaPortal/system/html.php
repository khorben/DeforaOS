<?php //system/html.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

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
			print('<a href="'.$link.'page='.$i.'">'.$i."</a>\n");
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
		else if($line[0] == ' ')
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
		$line = preg_replace('/((ftp|http):\/\/[-a-zA-Z0-9.\/_%]+)/',
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


function _html_safe_link($string)
{
	return str_replace('&amp;amp;', '%26', htmlentities($string));
}


function _start_css_themes($theme)
{
	if(($dir = @opendir('themes')) == FALSE)
		return;
	while(($de = readdir($dir)))
	{
		if(($len = strlen($de)) < 5)
			continue;
		if(substr($de, -4) != '.css')
			continue;
		$name = substr($de, 0, $len-4);
		if($name == $theme)
			continue;
		print("\t\t".'<link rel="alternate stylesheet" href="themes/'
				.$name.'.css" title="'.$name.'"/>'."\n");
	}
	closedir($dir);
}

function _html_start()
{
	global $title, $theme;

	readfile('html/doctype.html');
	print('	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-15"/>
		<title>'._html_safe($title).'</title>
		<link rel="stylesheet" href="themes/'.$theme.'.css" title="'
.$theme.'"/>'."\n");
	_start_css_themes($theme);
	print('	</head>
	<body>'."\n");
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
		$string = str_replace("&lt;$t&gt;", "<$t>", $string);
	return $string;
}


function _html_template($template)
{
	if(!@include('templates/'.$template.'.tpl'))
		return _error('Could not include template');
}

?>
