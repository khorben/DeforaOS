<?php
//system/html.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _html_safe($string)
{
	return htmlentities($string);
}


function _html_safe_link($string)
{
	return str_replace('&amp;amp;', '%26', htmlentities($string));
}


function _html_tags($string, $tags = FALSE)
{
	if(!is_array($tags))
		$tags = array('b', '/b', 'br/', 'li', '/li', 'p', '/p',
				'ul', '/ul');
	$string = _html_safe($string);
	foreach($tags as $t)
		$string = str_replace("&lt;$t&gt;", "<$t>", $string);
	return $string;
}


function _start_css_themes($theme)
{
	if(($dir = opendir('themes')) == FALSE)
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

	if(!isset($title) && ($title = _config_get('admin', 'title')) == FALSE)
		$title = 'DaPortal';
	readfile('html/doctype.html');
	print('	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-15"/>
		<title>'.$title.'</title>
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


function _html_template($template)
{
	include('templates/'.$template.'.tpl');
}

?>
