					<div class="entry" onclick="entry_click(<?php echo $i; ?>, event)">
						<input type="checkbox" name="entry_<?php echo $i; ?>" value="<?php echo html_safe_link($file.'/'.$name); ?>"/>
						<div class="thumbnail"><?php if(strlen($link)) { ?><a href="<?php echo html_safe_link($link); ?>"><?php } ?><img src="<?php echo $thumbnail; ?>" alt=""/><?php if(strlen($link)) { ?></a><?php } ?></div>
						<div class="icon"><?php if(strlen($link)) { ?><a href="<?php echo html_safe_link($link); ?>"><?php } ?><img src="<?php echo $icon; ?>" alt=""/><?php if(strlen($link)) { ?></a><?php } ?></div>
						<div class="name"><?php if(strlen($link)) { ?><a href="<?php echo html_safe_link($link); ?>"><?php } ?><?php echo html_safe($name); ?><?php if(strlen($link)) { ?></a><?php } ?></div>
						<div class="owner"><?php echo html_safe_link($owner); ?></div>
						<div class="group"><?php echo html_safe_link($group); ?></div>
						<div class="permissions"><?php echo html_safe_link($permissions); ?></div>
						<div class="size"><?php echo html_safe_link($size); ?></div>
						<div class="date"><?php echo html_safe_link($date); ?></div>
					</div>
