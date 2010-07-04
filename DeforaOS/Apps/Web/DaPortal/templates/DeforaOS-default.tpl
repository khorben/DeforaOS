<?php
$text = array();
$text['ABOUT_THE_PROJECT'] = 'About the project';
$text['LATEST_NEWS'] = 'Latest news';
$text['LATEST_WIKI_CHANGES'] = 'Latest wiki changes';
$text['MORE_NEWS'] = 'More news';
$text['PUBLIC_WIKI'] = 'Public wiki';
if($lang == 'fr')
{
	$text['ABOUT_THE_PROJECT'] = '� propos du projet';
	$text['LATEST_NEWS'] = 'Actualit�s';
	$text['LATEST_WIKI_CHANGES'] = 'Derni�res modifications du wiki';
	$text['MORE_NEWS'] = 'Suite';
	$text['PUBLIC_WIKI'] = 'Wiki public';
}
_lang($text);
?>
		<h1 class="title home">DeforaOS <?php echo _html_safe(HOMEPAGE); ?></h1>
		<h3 class="title about"><?php echo _html_safe(ABOUT_THE_PROJECT); ?></h3>
<?php switch($lang) { ?>
<?php case 'fr': ?>
		<p>
<strong>DeforaOS est un syst�me d'exploitation (OS)</strong> comprenant
plusieurs objectifs, dont l'impl�mentation d'une infrastructure communiquante,
s�curis�e et ind�pendante du noyau (kernel) utilis�.
		</p>
		<table cellpadding="4px">
		<tr><th>1. Base innovante</th><th>2. Cross-platform framework</th><th>3. Environnement de bureau</th></tr>
		<tr>
		<td>
Le but principal est de permettre un acc�s <b>ubiquitaire</b>, <b>s�curis�</b>
et <b>transparent</b> � ses ressources. Elles ne sont pas limit�es aux donn�es
dans ce contexte, avec la possibilit� d'�changer des applications fonctionnant �
distance.
		</td>
		<td>
Le projet contient un environnement compatible POSIX, tout en �tant capable de
fonctionner sur une base de syst�me Linux, *BSD ou Solaris. D�velopp� en pensant
d'abord � sa <b>simplicit�</b> et son <b>efficacit�</b>, il peut �galement
r�pondre aux contraintes des plate-formes <b>embarqu�es</b>.
		</td>
		<td>
Beaucoup d'importance est �galement accord�e � l'<b>ergonomie</b>, la
<b>coh�rence</b> et l'<b>int�gration</b>. En cons�quence, le projet propose un
certain nombre d'applications graphiques. Bien que d�j� utiles sur les syst�mes
actuels, elles vont directement exploiter les sp�cificit�s de DeforaOS.
		</td>
		</tr>
		<tr><th colspan="3">4. Cutting-edge platform for today's needs</th></tr>
		<tr><td colspan="3">
Ce projet <b>exp�rimental</b> a pour <b>ambition</b> de r�soudre autant de
probl�mes r�currents des syst�mes d'exploitation actuels que possible, par une
<b>r�vision de la conception</b> de la plupart de leurs composants.<br/>
Notamment, bien que le syst�me actuel soit ind�pendant du noyau utilis�, le
d�veloppement d'un micro-noyau d�di� pourrait avoir du sens � terme.
		</td></tr>
		</table>
<?php break; case 'en': default: ?>
		<p>
<strong>DeforaOS is a multi-purpose Operating System</strong> project, aiming at
the implementation of a kernel-independent, networked and secure infrastructure.
		</p>
		<table cellpadding="4px">
		<tr><th>1. Innovative foundation</th><th>2. Cross-platform framework</th><th>3. Desktop environment</th></tr>
		<tr>
		<td>
The main goal is to provide <b>ubiquitous</b>, <b>secure</b> and
<b>transparent</b> access to one's resources. This is not limited to data, with
the possibility to resume applications running remotely.
		</td>
		<td>
The system features a POSIX-compliant environment, and is already able to
function on top of most Linux, *BSD or Solaris-based systems. Developed with
<b>simplicity</b> and <b>efficiency</b> in mind, it is believed to suit modern
<b>embedded</b> platforms as well.
		</td>
		<td>
The project is also focused on <b>usability</b>, <b>coherence</b> and
<b>integration</b>, and therefore featuring a number of desktop applications.
Although already intended to be useful on current systems, they eventually
benefit from DeforaOS' own set of features.
		</td>
		</tr>
		<tr><th colspan="3">4. Cutting-edge platform for today's needs</th></tr>
		<tr><td colspan="3">
This <b>experimental</b> project has the <b>ambition</b> to address a number of
recurring issues with contemporary Operating Systems, with the <b>re-design</b>
of most of their components.<br/>
Additionally, even though the system is kernel-agnostic at the moment, a
dedicated micro-kernel may also be useful at some later stage.
		</td></tr>
		</table>
<?php } ?>

		<table>
		<tr><td style="vertical-align: top"><h3 class="title news"><?php echo _html_safe(LATEST_NEWS); ?></h3>
		<p>
<?php _module('news', 'headline', array('npp' => 6)); ?>
<a href="<?php echo _html_link('news'); ?>" title="DeforaOS news"><span class="icon add"><?php echo _html_safe(MORE_NEWS); ?>...</span></a>
		</p></td>
		<td style="vertical-align: top"><h3 class="title wiki"><?php echo _html_safe(LATEST_WIKI_CHANGES); ?></h3>
		<p>
<?php _module('wiki', 'recent', array('npp' => 6)); ?>
<a href="<?php echo _html_link('wiki'); ?>" title="DeforaOS wiki"><span class="icon add"><?php echo _html_safe(PUBLIC_WIKI); ?>...</span></a>
		</p></td></tr>
		</table>
