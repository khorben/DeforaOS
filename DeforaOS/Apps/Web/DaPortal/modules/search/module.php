<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['RESULTS_FOUND'] = 'result(s) found';
$text['SEARCH_RESULTS'] = 'Search results';
global $lang;
if($lang == 'fr')
{
	$text['RESULTS_FOUND'] = 'résultat(s) trouvés';
	$text['SEARCH_RESULTS'] = 'Résultats de la recherche';
}
_lang($text);


function _default_results($q)
{
	return is_array($ret) ? $ret : array();
}

function search_default($args)
{
	include('./modules/search/search.tpl');
	if(!isset($args['q']) || strlen($args['q']) == 0)
		return;
	$q = explode(' ', $args['q']);
	if(!count($q))
		return;
	$sql = ' FROM daportal_content, daportal_module, daportal_user'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.enabled='1'";
	$sql .= " AND ((title LIKE '%".implode("%' AND title LIKE '%", $q)
			."%')";
	$sql .= " OR (content LIKE '%".implode("%' AND content LIKE '%", $q)
			."%'))";
	$spp = 10;
	$page = isset($args['page']) ? $args['page'] : 1;
	$count = _sql_single('SELECT COUNT(*)'.$sql);
	include('./modules/search/search_top.tpl');
	$pages = ceil($count / $spp);
	$page = min($page, $pages);
	$res = $count == 0 ? array() :_sql_array('SELECT content_id AS id'
			.', timestamp, daportal_content.module_id'
			.', name AS module, daportal_content.user_id, title'
			.', content, username'.$sql.' ORDER by timestamp DESC'
			." OFFSET ".(($page-1) * $spp)." LIMIT $spp;");
	if(!is_array($res))
		return _error('Unable to search');
	$i = 1 + (($page-1) * $spp);
	foreach($res as $q)
	{
		$q['date'] = strftime(DATE_FORMAT, strtotime(substr(
						$q['timestamp'], 0, 19)));
		include('./modules/search/search_entry.tpl');
		$i++;
	}
	include('./modules/search/search_bottom.tpl');
	_html_paging('index.php?module=search&amp;q='
			._html_safe_link($args['q']).'&amp;', $page, $pages);
}


function search_system($args)
{
	global $title;

	$title.=' - '.SEARCH;
}

?>
