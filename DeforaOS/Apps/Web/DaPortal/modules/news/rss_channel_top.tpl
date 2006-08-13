<?php echo '<?xml version="1.0" encoding="iso-8859-15"?>'; ?>

<rss version="2.0">
	<channel>
		<title><?php echo _html_safe($title); ?></title>
		<link><?php echo _html_safe($link); ?></link>
		<description><?php echo _html_safe($content); ?></description>
		<language><?php global $lang; echo _html_safe($lang); ?></language>
