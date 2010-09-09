<h1 class="title search"><?php if(isset($args['q']) && strlen($args['q'])) echo _html_safe(SEARCH_RESULTS); else echo _html_safe(ADVANCED_SEARCH); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="search"/>
	<input type="hidden" name="action" value="advanced"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(QUERY); ?>:</td><td colspan="3"><input type="text" name="q" value="<?php if(isset($args['q'])) echo _html_safe(stripslashes($args['q'])); ?>" size="30"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(SEARCH_IN); ?>:</td><td colspan="3"><input type="checkbox" name="intitle"<?php if(isset($args['intitle'])) { ?> checked="checked"<?php } ?>> <?php echo _html_safe(TITLES); ?></input> <input type="checkbox" name="incontent"<?php if(isset($args['incontent'])) { ?> checked="checked"<?php } ?>> <?php echo _html_safe(CONTENTS); ?></input></td></tr>
		<tr><td class="field"><?php echo _html_safe(CONTENT_FROM); ?>:</td><td><input type="text" name="user" value="<?php if(isset($args['user'])) echo _html_safe(stripslashes($args['user'])); ?>" size="15"/></td><td class="field"><?php echo _html_safe(IN_MODULE); ?></td><td><select name="inmodule">
			<option value=""></option>
<?php foreach($modules as $m) { if(!$m['search']) continue; ?>
			<option value="<?php echo _html_safe($m['name']); ?>"<?php if(isset($args['inmodule']) && $args['inmodule'] == $m['name']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($m['title']); ?></option>
<?php } ?></select></td>
		<tr><td></td><td colspan="3"><button type="reset" class="icon reset"><?php echo _html_safe(RESET); ?></button> <input type="submit" value="<?php echo _html_safe(SEARCH); ?>" class="icon search"/></td></tr>
	</table>
	<p><a href="<?php echo _html_link('search', FALSE, FALSE, FALSE, isset($args['q']) ? array('q' => stripslashes($args['q'])) : FALSE); ?>"><span class="icon remove"></span><?php echo _html_safe(SIMPLER_SEARCH).'...'; ?></a></p>
</form>
<hr/>
