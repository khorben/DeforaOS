		<item>
			<title><?php echo _html_safe($post['title']); ?></title>
			<author><?php echo _html_safe($post['author']); ?></author>
			<pubDate><?php echo _html_safe($post['date']); ?></pubDate>
			<link><?php echo _html_safe($post['link']); ?></link>
			<guid><?php echo _html_safe($post['link']); ?></guid>
			<description><![CDATA[<?php echo $post['content']; ?>]]></description>
		</item>
