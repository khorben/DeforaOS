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
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

function _file_csrc($line)
{
	static $comment = 0;

	if(strlen($line) == 0)
		return $line;
	$line = preg_replace('/(&quot;[^(&quot;)]+&quot;)/',
			'<span class="string">\1</span>', $line);
	$line = preg_replace("/('(\\\\?.|[^'])')/",
			'<span class="character">\1</span>', $line);
	if($line[0] == '#')
	{
		$line = '<span class="preprocessed">'.substr($line, 0, -1)
			.'</span>'."\n";
		return $line;
	}
	$line = preg_replace('/(^|[^a-zA-Z0-9_])(break|case|continue|default'
				.'|do|else|for|goto|if|return|sizeof|switch'
				.'|until|while'
				.')($|[^a-zA-Z0-9_])/',
			'\1<span class="keyword">\2</span>\3', $line);
	$line = preg_replace('/(^|[^a-zA-Z0-9_])('
				.'char|DIR|double|FILE|float|int'
				.'|int8_t|int16_t|int32_t|int64_t'
				.'|long|short|size_t|ssize_t|signed|static'
				.'|struct|typedef|union|unsigned|va_list|void'
				.'|uint8_t|uint16_t|uint32_t|uint64_t'
				.')($|[^a-zA-Z0-9_])/',
			'\1<span class="type">\2</span>\3', $line);
	/* FIXME fails on quoted strings, use prefix.line.suffix to escape hl */
	if($comment == 0 && strstr($line, '/*'))
	{
		$p = strpos($line, '/*');
		$line = substr($line, 0, $p).'<span class="comment">'
			.substr($line, $p);
		$comment = 1;
	}
	if($comment != 0 && strstr($line, '*/'))
	{
		$p = strpos($line, '*/');
		$line = substr($line, 0, $p+2).'</span>'.substr($line, $p+2);
		$comment = 0;
	}
	return $line;
}

function _file_makefile($line)
{
	$line = preg_replace('/\s(#.*$)/',
			'<span class="comment">\1</span>', $line);
	$line = preg_replace('/^([a-zA-Z]+)(\s*=)/',
			'<span class="variable">\1</span>\2', $line);
	$line = preg_replace('/(^[a-zA-Z]+:)/',
			'<span class="variable">\1</span>', $line);
	$line = preg_replace('/(\$\([a-zA-Z_]+\))/',
			'<span class="variable">\1</span>', $line);
	return $line;
}

function _file_php($line)
{
	static $comment = 0;

	$line = preg_replace('/(&quot;[^(&quot;)]+&quot;)/',
			'<span class="string">\1</span>', $line);
	$line = preg_replace("/('(.|[^']*)')/",
			'<span class="string">\1</span>', $line);
	$line = preg_replace('/(^|[^a-zA-Z0-9_])(break|case|continue|default'
				.'|do|else|for|foreach|if|return|switch'
				.'|until|while|=&gt;|&amp;&amp;|\|\|'
				.')($|[^a-zA-Z0-9_])/',
			'\1<span class="keyword">\2</span>\3', $line);
	$line = preg_replace('/(^|[^a-zA-Z0-9_])('
				.'array|function|global|include|include_once'
				.'|require|require_once|static'
				.')($|[^a-zA-Z0-9_])/',
			'\1<span class="type">\2</span>\3', $line);
	$line = preg_replace('/(\/\/.*$)/',
			'<span class="comment">\1</span>', $line);
	/* FIXME fails on quoted strings, use prefix.line.suffix to escape hl */
	if($comment == 0 && strstr($line, '/*'))
	{
		$p = strpos($line, '/*');
		$line = substr($line, 0, $p).'<span class="comment">'
			.substr($line, $p);
		$comment = 1;
	}
	if($comment != 0 && strstr($line, '*/'))
	{
		$p = strpos($line, '*/');
		$line = substr($line, 0, $p+2).'</span>'.substr($line, $p+2);
		$comment = 0;
	}
	return $line;
}

?>
