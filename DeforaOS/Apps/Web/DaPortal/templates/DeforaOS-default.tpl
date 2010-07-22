<?php
$text = array();
$text['DEFORAOS_PROJECT'] = 'DeforaOS Project';
$text['LATEST_NEWS'] = 'Latest news';
$text['LATEST_WIKI_CHANGES'] = 'Latest wiki changes';
$text['MORE_NEWS'] = 'More news';
$text['MULTI_PURPOSE_OPERATING_SYSTEM'] = 'Multi-purpose Operating System';
$text['PUBLIC_WIKI'] = 'Public wiki';
if($lang == 'fr')
{
	$text['DEFORAOS_PROJECT'] = 'Projet DeforaOS';
	$text['LATEST_NEWS'] = 'Actualités';
	$text['LATEST_WIKI_CHANGES'] = 'Dernières modifications du wiki';
	$text['MORE_NEWS'] = 'Suite';
	$text['MULTI_PURPOSE_OPERATING_SYSTEM'] = "Système d'exploitation flexible";
	$text['PUBLIC_WIKI'] = 'Wiki public';
}
_lang($text);
?>
		<h1 class="title home"><?php echo _html_safe(DEFORAOS_PROJECT); ?></h1>
		<h3 class="title about"><?php echo _html_safe(MULTI_PURPOSE_OPERATING_SYSTEM); ?></h3>
<?php switch($lang) { ?>
<?php case 'fr': ?>
		<p>
Infrastructure communiquante, sécurisée et indépendante du noyau (kernel) utilisé.
		</p>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>1. Base innovante</strong></p>
<p>Le but principal est de permettre un accès <b>ubiquitaire</b>, <b>sécurisé</b>
et <b>transparent</b> à ses ressources. Elles ne sont pas limitées aux données
dans ce contexte, avec la possibilité d'échanger des applications fonctionnant à
distance.</p>
		</div>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>2. Structure portable</strong></p>
<p>Le projet contient un environnement compatible POSIX, tout en étant capable de
fonctionner sur une base de système Linux, *BSD ou Solaris. Développé en pensant
d'abord à sa <b>simplicité</b> et son <b>efficacité</b>, il peut également
répondre aux contraintes des plate-formes <b>embarquées</b>.</p>
		</div>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>3. Environnement de bureau</strong></p>
<p>Beaucoup d'importance est également accordée à l'<b>ergonomie</b>, la
<b>cohérence</b> et l'<b>intégration</b>. En conséquence, le projet propose un
certain nombre d'applications graphiques. Bien que déjà utiles sur les systèmes
actuels, elles vont directement exploiter les spécificités de DeforaOS.</p>
		</div>
		<div style="clear: left; padding: 0.4em; max-width: 54em">
		<p><strong>4. Plate-forme de pointe pour les besoins actuels</strong></p>
<p>Ce projet <b>expérimental</b> a pour <b>ambition</b> de résoudre autant de
problèmes récurrents des systèmes d'exploitation actuels que possible, par une
<b>révision de la conception</b> de la plupart de leurs composants.<br/>
Notamment, bien que le système actuel soit indépendant du noyau utilisé, le
développement d'un micro-noyau dédié pourrait avoir du sens à terme.</p>
		</div>
<?php break; case 'en': default: ?>
		<p>
Kernel-independent, networked and secure infrastructure:
		</p>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>1. Innovative foundation</strong></p>
<p>The main goal is to provide <b>ubiquitous</b>, <b>secure</b> and
<b>transparent</b> access to one's resources. This is not limited to data, with
the possibility to resume applications running remotely.</p>
		</div>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>2. Cross-platform framework</strong></p>
<p>The system features a POSIX-compliant environment, and is already able to
function on top of most Linux, *BSD or Solaris-based systems. Developed with
<b>simplicity</b> and <b>efficiency</b> in mind, it is believed to suit modern
<b>embedded</b> platforms as well.</p>
		</div>
		<div style="float: left; padding: 0.4em; max-width: 18em">
		<p><strong>3. Desktop environment</strong></p>
<p>The project is also focused on <b>usability</b>, <b>coherence</b> and
<b>integration</b>, and therefore featuring a number of desktop applications.
Although already intended to be useful on current systems, they eventually
benefit from DeforaOS' own set of features.</p>
		</div>
		<div style="clear: left; padding: 0.4em; max-width: 54em">
		<p><strong>4. Cutting-edge platform for today's needs</strong></p>
<p>This <b>experimental</b> project has the <b>ambition</b> to address a number of
recurring issues with contemporary Operating Systems, with the <b>re-design</b>
of most of their components.<br/>
Additionally, even though the system is kernel-agnostic at the moment, a
dedicated micro-kernel may also be useful at some later stage.</p>
		</div>
<?php } ?>

		<div style="float: left; padding: 0.4em; max-width: 28em">
		<h3 class="title news"><?php echo _html_safe(LATEST_NEWS); ?></h3>
<?php _module('news', 'headline', array('npp' => 6)); ?>
<a href="<?php echo _html_link('news'); ?>" title="DeforaOS news"><span class="icon add"><?php echo _html_safe(MORE_NEWS); ?>...</span></a>
		</div>
		<div style="float: left; padding: 0.4em; max-width: 28em">
		<h3 class="title wiki"><?php echo _html_safe(LATEST_WIKI_CHANGES); ?></h3>
<?php _module('wiki', 'recent', array('npp' => 6)); ?>
<a href="<?php echo _html_link('wiki'); ?>" title="DeforaOS wiki"><span class="icon add"><?php echo _html_safe(PUBLIC_WIKI); ?>...</span></a>
		</div>
		<div style="clear: left"></div>
