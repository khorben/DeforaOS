<?php echo '<?xml version="1.0" encoding="iso-8859-15"?>'; ?>

<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
	<channel>
		<title><?php echo _html_safe($title); ?></title>
		<link><?php echo _html_safe($link); ?></link>
		<atom:link href="<?php echo _html_safe($atomlink); ?>" rel="self" type="application/rss+xml"/>
		<description><?php echo _html_safe($content); ?></description>
		<language><?php global $lang; echo _html_safe($lang); ?></language>
