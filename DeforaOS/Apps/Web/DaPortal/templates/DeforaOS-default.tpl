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
