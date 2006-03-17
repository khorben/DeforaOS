					<div class="entry" onclick="entry_click(<? echo $i; ?>, event)">
						<input type="checkbox" name="entry_<? echo $i; ?>" value="<? echo html_safe_link($file.'/'.$name); ?>"/>
						<div class="thumbnail"><? if(strlen($link)) { ?><a href="<? echo html_safe_link($link); ?>"><? } ?><img src="<? echo $thumbnail; ?>" alt=""/><? if(strlen($link)) { ?></a><? } ?></div>
						<div class="icon"><? if(strlen($link)) { ?><a href="<? echo html_safe_link($link); ?>"><? } ?><img src="<? echo $icon; ?>" alt=""/><? if(strlen($link)) { ?></a><? } ?></div>
						<div class="name"><? if(strlen($link)) { ?><a href="<? echo html_safe_link($link); ?>"><? } ?><? echo html_safe($name); ?><? if(strlen($link)) { ?></a><? } ?></div>
						<div class="owner"><? echo html_safe_link($owner); ?></div>
						<div class="group"><? echo html_safe_link($group); ?></div>
						<div class="permissions"><? echo html_safe_link($permissions); ?></div>
						<div class="size"><? echo html_safe_link($size); ?></div>
						<div class="date"><? echo html_safe_link($date); ?></div>
					</div>
