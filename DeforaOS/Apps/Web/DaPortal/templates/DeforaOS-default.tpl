		<h1 class="title home">DeforaOS <?php echo _html_safe(HOMEPAGE); ?></h1>
<?php switch($lang) { ?>
<?php case 'fr': ?>
		<h3 class="title project">A propos du projet</h3>
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

		<h3 class="title news">Actualit&eacute;</h3>
		<p>
<?php _module('news', 'headline', array('npp' => 6)); ?>
<a href="index.php?module=news" title="DeforaOS news">Suite...</a>
		</p>
<?php break; case 'en': default: ?>
		<h3 class="title project">About the project</h3>
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

		<h3 class="title news">Latest news</h3>
		<p>
<?php _module('news', 'headline', array('npp' => 6)); ?>
<a href="index.php?module=news" title="DeforaOS news">More news...</a>
		</p>
<?php } ?>
