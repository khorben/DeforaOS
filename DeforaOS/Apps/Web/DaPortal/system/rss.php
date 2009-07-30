<?php //$Id$
//Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>
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


/* rss */
function _rss($title, $link, $atomlink, $content, $entries)
{
	global $lang;

	require_once('./system/html.php');
	echo '<?xml version="1.0" encoding="iso-8859-15"?>
<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
	<channel>
		<title>'._html_safe($title).'</title>
		<link>'._html_safe($link).'</link>
		<atom:link href="'._html_safe($atomlink).'" rel="self" type="application/rss+xml"/>
		<description>'._html_safe($content).'</description>
		<language>'._html_safe($lang).'</language>'."\n";
	foreach($entries as $e)
		echo '		<item>
			<title>'._html_safe($e['title']).'</title>
			<author>'._html_safe($e['author']).'</author>
			<pubDate>'._html_safe($e['date']).'</pubDate>
			<link>'._html_safe($e['link']).'</link>
			<guid>'._html_safe($e['link']).'</guid>
			<description><![CDATA['.$e['content'].']]></description>
		</item>'."\n";
	echo '	</channel>
</rss>'."\n";
}

?>
