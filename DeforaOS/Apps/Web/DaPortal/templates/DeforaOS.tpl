<?php global $lang, $user_id, $title, $module_name; $module = $module_name;
$text = array();
$text['DEVELOPMENT'] = 'Development';
$text['DOCUMENTATION'] = 'Documentation';
$text['DOWNLOAD'] = 'Download';
$text['INSTALLER'] = 'Installer';
$text['POLICY'] = 'Policy';
$text['PROJECT'] = 'Project';
$text['PROJECTS'] = 'Projects';
$text['REPORTS'] = 'Reports';
$text['ROADMAP'] = 'Roadmap';
$text['SUPPORT'] = 'Support';
if($lang == 'de')
{
	$text['PROJECT'] = 'Projekt';
	$text['PROJECTS'] = 'Projekte';
}
else if($lang == 'fr')
{
	$text['DEVELOPMENT'] = 'Développement';
	$text['DOWNLOAD'] = 'Télécharger';
	$text['POLICY'] = 'Objectifs';
	$text['PROJECT'] = 'Projet';
	$text['PROJECTS'] = 'Projets';
	$text['REPORTS'] = 'Rapports';
	$text['ROADMAP'] = 'Progression';
}
_lang($text);
_module('top'); ?>
		<div class="container">
			<div class="top_search">
				<form action="index.php" method="get">
					<div>
						<input type="hidden" name="module" value="search"/>
						<input type="text" name="q" value="<?php echo _html_safe(SEARCH); ?>..." size="20" onfocus="if(value == '<?php echo _html_safe(SEARCH); ?>...') value=''"/>
						<input id="search" type="submit" value="<?php echo _html_safe(SEARCH); ?>"/>
					</div>
				</form>
				<script type="text/javascript">
<!--
document.getElementById('search').style.display='none';
//-->
				</script>
			</div>
			<div class="logo"></div>
			<div class="style1"><a href="index.php">DeforaOS</a> :: <?php echo strlen($module) ? '<a href="index.php?module='.(_html_safe_link($module)).'">'.(_html_safe(ucfirst($module))).'</a>' : '<a href="index.php">Homepage</a>'; ?></div>
<?php if($user_id) _module('menu'); else include('./templates/DeforaOS-menu.tpl');
if(is_array(($langs = _sql_array('SELECT lang_id AS id, name FROM '
		." daportal_lang WHERE enabled='1' ORDER BY name ASC;")))) { ?>
			<form class="lang" action="index.php" method="post" style="float: right; margin-right: 30px">
				<select name="lang" onchange="submit()">
<?php foreach($langs as $l) { ?>
					<option value="<?php echo _html_safe($l['id']); ?>"<?php if($lang == $l['id']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($l['name']); ?></option>
<?php } ?>
				</select>
				<input id="lang" type="submit" value="Choose"/>
				<script type="text/javascript">
<!--
document.getElementById('lang').style.display='none';
//-->
				</script>
			</form>
<?php } ?>
			<div class="main">
<?php if(strlen($module)) _module(); else include('./templates/DeforaOS-default.tpl');
_debug(); readfile('./templates/DeforaOS-bottom.tpl'); ?>
