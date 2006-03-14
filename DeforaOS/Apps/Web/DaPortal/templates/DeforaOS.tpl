<? global $lang, $user_id, $title, $module_name; $module = $module_name; ?>
<? $text = array();
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
$text['VALIDATE'] = 'Validate';
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
	$text['VALIDATE'] = 'Valider';
}
_lang($text);
?>
<? _module('top'); ?>
		<div class="container">
			<div class="top_search">
				<form action="index.php" method="get">
					<input type="hidden" name="module" value="search"/>
					<input type="text" name="q" value="<? echo _html_safe(SEARCH); ?>..." size="20" onfocus="if(value == '<? echo _html_safe(SEARCH); ?>...') value=''"/>
					<input id="search" type="submit" value="<? echo _html_safe(SEARCH); ?>"/>
				</form>
				<script type="text/javascript">
<!--
document.getElementById('search').style.display='none';
//-->
				</script>
			</div>
			<div class="logo"></div>
			<div class="style1"><a href="index.php">DeforaOS</a> :: <? echo (strlen($module) ? '<a href="index.php?module='.$module.'">'.ucfirst($module).'</a>' : 'Homepage'); ?></div>
<? if($user_id) { _module('menu'); } else { ?>
			<ul class="menu">
				<li><a href="index.php"><? echo _html_safe(ABOUT); ?></a><ul>
					<li><a href="index.php?module=news"><? echo _html_safe(NEWS); ?></a></li>
					<li><a href="index.php?module=project"><? echo _html_safe(PROJECT); ?></a></li>
					<li><a href="roadmap.html"><? echo _html_safe(ROADMAP); ?></a></li>
					</ul></li>
				<li><a href="development.html"><? echo _html_safe(DEVELOPMENT); ?></a><ul>
					<li><a href="policy.html"><? echo _html_safe(POLICY); ?></a></li>
					<li><a href="index.php?module=project&amp;action=list"><? echo _html_safe(PROJECTS); ?></a></li>
					</ul></li>
				<li><a href="index.php?module=project&amp;action=download"><? echo _html_safe(DOWNLOAD); ?></a><ul>
					<li><a href="index.php?module=project&amp;action=installer"><? echo _html_safe(INSTALLER); ?></a></li>
					<li><a href="index.php?module=project&amp;action=package">Packages</a></li>
					</ul></li>
				<li><a href="support.html"><? echo _html_safe(SUPPORT); ?></a><ul>
					<li><a href="documentation.html"><? echo _html_safe(DOCUMENTATION); ?></a></li>
					<li><a href="index.php?module=project&amp;action=bug_list"><? echo _html_safe(REPORTS); ?></a></li>
					</ul></li>
			</ul>
<? } ?>
<? if(is_array(($langs = _sql_array('SELECT lang_id AS id, name'
		.' FROM daportal_lang'
		." WHERE enabled='t' ORDER BY name ASC;")))) { ?>
			<form class="lang" action="index.php" method="post" style="float: right; margin-right: 30px">
				<select name="lang" onchange="submit()">
<? foreach($langs as $l) { ?>
					<option value="<? echo _html_safe($l['id']); ?>"<? if($lang == $l['id']) { ?> selected="selected"<? } ?>><? echo _html_safe($l['name']); ?></option>
<? } ?>
				</select>
				<input id="lang" type="submit" value="Choose"/>
				<script type="text/javascript">
<!--
document.getElementById('lang').style.display='none';
//-->
				</script>
			</form>
<? } ?>
			<div class="main">
<? if(strlen($module)) { _module(); } else { ?>
		<h1>DeforaOS <? echo _html_safe(HOMEPAGE); ?></h1>
<? switch($lang) { ?>
<? case 'fr': ?>
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
Le projet est toujours en <a href="index.php?module=project">phase de
conception</a>.
		</p>

		<h3>Actualit&eacute;s</h3>
		<p>
Mises &agrave; jour incluant les <a href="index.php?module=news">&eacute;tapes
d&eacute;terminantes</a> du projet. Celles-ci seront facilit&eacute;es par
l'&eacute;volution du d&eacute;veloppement du portail web.
		</p>
<? break; case 'en': default: ?>
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
The project is still at its <a href="index.php?module=project">earliest
stage</a>.
		</p>

		<h3>Latest news</h3>
		<p>
Will hopefully be filled with <a href="index.php?module=news">every significant
work made</a> for the project. This will be eased with the implementation of
the site content management system.
		</p>
<? } ?>
<? } ?>
<? _debug(); ?>
			</div>
			<div style="clear: left">&nbsp;</div>
			<div class="style1" style="padding-right: 33px; text-align: right;"><a href="http://validator.w3.org/check/referer"><img src="images/xhtml.png" alt=""/></a> <a href="http://jigsaw.w3.org/css-validator/check/referer"><img src="images/css.png" alt=""/></a></div>
		</div>
