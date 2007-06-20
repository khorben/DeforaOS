			<ul class="menu">
				<li><a href="/index.php"><?php echo _html_safe(ABOUT); ?></a><ul>
					<li><a href="<?php echo _html_link('news'); ?>"><?php echo _html_safe(NEWS); ?></a></li>
					<li><a href="<?php echo _html_link('project'); ?>"><?php echo _html_safe(PROJECT); ?></a></li>
					<li><a href="/roadmap.php"><?php echo _html_safe(ROADMAP); ?></a></li>
					</ul></li>
				<li><a href="<?php echo _html_link('project', '', 11); ?>"><?php echo _html_safe(DEVELOPMENT); ?></a><ul>
					<li><a href="/policy.php"><?php echo _html_safe(POLICY); ?></a></li>
					<li><a href="<?php echo _html_link('project', 'list'); ?>"><?php echo _html_safe(PROJECTS); ?></a></li>
					</ul></li>
				<li><a href="<?php echo _html_link('project', 'download'); ?>"><?php echo _html_safe(DOWNLOAD); ?></a><ul>
					<li><a href="<?php echo _html_link('project', 'installer'); ?>"><?php echo _html_safe(INSTALLER); ?></a></li>
					<li><a href="<?php echo _html_link('project', 'package'); ?>"><?php echo _html_safe(PACKAGES); ?></a></li>
					<li><a href="<?php echo _html_link('category', '', 293); ?>"><?php echo _html_safe(SCREENSHOTS); ?></a></li>
					</ul></li>
				<li><a href="/support.php"><?php echo _html_safe(SUPPORT); ?></a><ul>
					<li><a href="/documentation.php"><?php echo _html_safe(DOCUMENTATION); ?></a></li>
					<li><a href="<?php echo _html_link('project', 'bug_list'); ?>"><?php echo _html_safe(REPORTS); ?></a></li>
					</ul></li>
			</ul>
