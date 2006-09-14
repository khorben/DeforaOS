<?php global $lang, $user_id, $title, $module_name; $module = $module_name; ?>
<?php $text = array();
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
?>
<?php _module('top'); ?>
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
<?php if($user_id) { _module('menu'); } else { ?>
			<ul class="menu">
				<li><a href="index.php"><?php echo _html_safe(ABOUT); ?></a><ul>
					<li><a href="index.php?module=news"><?php echo _html_safe(NEWS); ?></a></li>
					<li><a href="index.php?module=project"><?php echo _html_safe(PROJECT); ?></a></li>
					<li><a href="roadmap.html"><?php echo _html_safe(ROADMAP); ?></a></li>
					</ul></li>
				<li><a href="index.php?module=project&amp;action=display&amp;id=11"><?php echo _html_safe(DEVELOPMENT); ?></a><ul>
					<li><a href="policy.html"><?php echo _html_safe(POLICY); ?></a></li>
					<li><a href="index.php?module=project&amp;action=list"><?php echo _html_safe(PROJECTS); ?></a></li>
					</ul></li>
				<li><a href="index.php?module=project&amp;action=download"><?php echo _html_safe(DOWNLOAD); ?></a><ul>
					<li><a href="index.php?module=project&amp;action=installer"><?php echo _html_safe(INSTALLER); ?></a></li>
					<li><a href="index.php?module=project&amp;action=package">Packages</a></li>
					</ul></li>
				<li><a href="support.html"><?php echo _html_safe(SUPPORT); ?></a><ul>
					<li><a href="documentation.html"><?php echo _html_safe(DOCUMENTATION); ?></a></li>
					<li><a href="index.php?module=project&amp;action=bug_list"><?php echo _html_safe(REPORTS); ?></a></li>
					</ul></li>
			</ul>
<?php } ?>
<?php if(is_array(($langs = _sql_array('SELECT lang_id AS id, name'
		.' FROM daportal_lang'
		." WHERE enabled='1' ORDER BY name ASC;")))) { ?>
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
<?php if(strlen($module)) { _module(); } else { ?>
		<h1>DeforaOS <?php echo _html_safe(HOMEPAGE); ?></h1>
<?php switch($lang) { ?>
<?php case 'fr': ?>
		<h3>A propos du projet</h3>
		<p>
Ce projet a pour but d'implémenter un système d'exploitation, basé sur un
micro-kernel. Les principaux objectifs comprennent:
		</p>
		<ul>
			<li>une conception simple;</li>
			<li>un code clair;</li>
			<li>un r&eacute;sultat utilisable.</li>
		</ul>
		<p>
Le <a href="index.php?module=project&amp;id=11">projet</a> est toujours en <a href="index.php?module=project">phase de
conception</a>.
		</p>

		<h3>Actualit&eacute;s</h3>
		<p>
Mises &agrave; jour incluant les <a href="index.php?module=news">&eacute;tapes
d&eacute;terminantes</a> du projet.
		</p>
<?php break; case 'en': default: ?>
		<h3>About the project</h3>
		<p>
This project aims at the implementation of a micro-kernel based operating
system. The primary goals include:
		</p>
		<ul>
			<li>clean design;</li>
			<li>simple code;</li>
			<li>usability.</li>
		</ul>
		<p>
The <a href="index.php?module=project&amp;id=11">project</a> is still at an <a
href="index.php?module=project">early stage</a>.
		</p>

		<h3>Latest news</h3>
		<p>
Eventually filled with <a href="index.php?module=news">every significant
work made</a> for the project.
		</p>
<?php } ?>
<?php } ?>
<?php _debug(); ?>
			</div>
			<div style="clear: left">&nbsp;</div>
			<div class="style1" style="padding-right: 33px; text-align: right;"><a href="http://validator.w3.org/check/referer"><img src="images/xhtml.png" alt="XHTML"/></a> <a href="http://jigsaw.w3.org/css-validator/check/referer"><img src="images/css.png" alt="CSS"/> <a href="index.php?module=news&amp;action=rss"><img src="images/rss.png" alt="RSS"/></a> </a><a href="Defora-cacert.pem"><img src="images/defora-ca.png" alt="Defora CA"/></a></div>
		</div>
