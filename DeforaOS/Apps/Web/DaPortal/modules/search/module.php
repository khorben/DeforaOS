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
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['ADVANCED_SEARCH'] = 'Advanced search';
$text['CONTENTS'] = 'Contents';
$text['CONTENT_FROM'] = 'Content from';
$text['IN_MODULE'] = 'In module';
$text['RESULTS_FOUND'] = 'result(s) found';
$text['QUERY'] = 'Query';
$text['SEARCH_IN'] = 'Search in';
$text['SEARCH_RESULTS'] = 'Search results';
$text['TITLES'] = 'Titles';
global $lang;
if($lang == 'fr')
{
	$text['RESULTS_FOUND'] = 'résultat(s) trouvés';
	$text['SEARCH_RESULTS'] = 'Résultats de la recherche';
}
_lang($text);


//private
//search_do
function _search_do($q, $intitle, $incontent, $spp, $page, $user = FALSE,
		$module = FALSE, $advanced = FALSE)
{
	$query = stripslashes($q);
	$q = str_replace(array('%', '_'), array('\\\%', '\\\_'), $q);
	$q = explode(' ', $q);
	if(!count($q))
		return;
	$sql = ' FROM daportal_content, daportal_module, daportal_user'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.enabled='1'";
	if($user !== FALSE)
		$sql .= " AND daportal_user.username='$user'";
	if($module !== FALSE)
		$sql .= " AND daportal_module.name='$module'";
	$sql .= " AND (0=1";
	if($intitle)
		$sql .= " OR (title LIKE '%".implode("%' AND title LIKE '%",
						$q)."%')";
	if($incontent)
		$sql .= " OR (content LIKE '%".implode("%' AND content LIKE '%",
					$q)."%')";
	$sql .= ')';
	$count = _sql_single('SELECT COUNT(*)'.$sql);
	include('./modules/search/search_top.tpl');
	$pages = ceil($count / $spp);
	$page = min($page, $pages);
	$res = ($count == 0) ? array() : _sql_array('SELECT content_id AS id'
			.', timestamp, daportal_content.module_id'
			.', name AS module, daportal_content.user_id AS user_id'
			.', title, content, username'.$sql
			.' ORDER by timestamp DESC '
			._sql_offset(($page - 1) * $spp, $spp));
	if(!is_array($res))
		return _error('Unable to search');
	$i = 1 + (($page - 1) * $spp);
	foreach($res as $q)
	{
		$q['date'] = _sql_date($q['timestamp']);
		include('./modules/search/search_entry.tpl');
		$i++;
	}
	include('./modules/search/search_bottom.tpl');
	_html_paging(_html_link('search', $advanced ? 'advanced' : FALSE, FALSE,
				FALSE, array('q' => _html_safe($query),
					'page' => '')), $page, $pages);
}


//public
//functions
//search_advanced
function search_advanced($args)
{
	$modules = _module_list();
	include('./modules/search/search_advanced.tpl');
	if(!isset($args['q']) || strlen($args['q']) == 0)
		return;
	if(!isset($args['page']) || !is_numeric($args['page']))
		$args['page'] = 1;
	$args['intitle'] = (isset($args['intitle'])) ? 1 : 0;
	$args['incontent'] = (isset($args['incontent'])) ? 1 : 0;
	if($args['intitle'] == 0 && $args['incontent'] == 0)
	{
		$args['intitle'] = 1;
		$args['incontent'] = 1;
	}
	$args['user'] = (isset($args['user']) && strlen($args['user']))
		? $args['user'] : FALSE;
	$args['inmodule'] = (isset($args['inmodule'])
			&& strlen($args['inmodule'])) ? $args['inmodule']
		: FALSE;
	return _search_do($args['q'], $args['intitle'], $args['incontent'], 10,
			$args['page'], $args['user'], $args['inmodule'], TRUE);
}


//search_default
function search_default($args)
{
	include('./modules/search/search.tpl');
	if(!isset($args['q']) || strlen($args['q']) == 0)
		return;
	if(!isset($args['page']) || !is_numeric($args['page']))
		$args['page'] = 1;
	return _search_do($args['q'], 1, 1, 10, $args['page']);
}


//search_system
function search_system($args)
{
	global $title;

	$title.=' - '.SEARCH;
}

?>
