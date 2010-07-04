<?php //$Id$
//Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));

//lang
$text['ALIGN_CENTER'] = 'Aligner au centre';
$text['ALIGN_JUSTIFY'] = 'Aligner justifi�';
$text['ALIGN_LEFT'] = 'Aligner � gauche';
$text['ALIGN_RIGHT'] = 'Aligner � droite';
$text['BOLD'] = 'Gras';
$text['COPY'] = 'Copier';
$text['CREATE'] = 'Cr�er';
$text['CREATE_A_PAGE'] = 'Cr�er une page';
$text['CUT'] = 'Couper';
$text['DOCUMENT_NOT_VALID'] = 'Document non valide';
$text['FONT'] = 'Fonte';
$text['ITALIC'] = 'Italique';
$text['LOOK_FOR_A_PAGE'] = 'Chercher une page';
$text['LOOK_INSIDE_PAGES'] = 'Chercher dans les pages';
$text['NEW_WIKI_PAGE'] = 'Nouvelle page Wiki';
$text['PASTE'] = 'Coller';
$text['RECENT_CHANGES'] = 'Changements r�cents';
$text['REDO'] = 'Refaire';
$text['SIZE'] = 'Taille';
$text['STRIKE'] = 'Barr�';
$text['SUBSCRIPT'] = 'Indice';
$text['SUPERSCRIPT'] = 'Exposant';
$text['UNDERLINE'] = 'Soulign�';
$text['UNDO'] = 'Annuler';
$text['WIKI_ADMINISTRATION'] = 'Administration du wiki';
$text['WIKI_PAGE_PREVIEW'] = 'Pr�visualisation de page Wiki';
$text['WIKI_PAGES_LIST'] = 'Liste des pages Wiki';
$text['WIKI_SEARCH'] = 'Recherche wiki';

?>
