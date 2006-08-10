<?php //modules/search/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
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
	if(strlen($args['q']) == 0)
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
