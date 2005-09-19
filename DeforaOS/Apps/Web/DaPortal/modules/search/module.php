<?php
//modules/search/module.php



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


function _default_results($q, $field)
{
	$q = explode(' ', $q);
	if(!count($q))
		return array();
	$query = " AND $field LIKE '%".implode("%' AND $field LIKE '%", $q)
			."%'";
	$ret = _sql_array('SELECT content_id AS id, timestamp'
			.', daportal_content.module_id, name AS module'
			.', daportal_content.user_id, title, content, username'
			.' FROM daportal_content, daportal_module'
			.', daportal_user'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.enabled='1'"
			.$query.' ORDER by timestamp DESC;');
	return is_array($ret) ? $ret : array();
}

function search_default($args)
{
	include('search.tpl');
	if(strlen($args['q']) == 0)
		return;
	$res = _default_results($args['q'], 'title');
	$res = array_merge($res, _default_results($args['q'], 'content'));
	//FIXME sort so that duplicates are put on top and unique'd
	$count = count($res);
	include('search_top.tpl');
	$i = 1;
	foreach($res as $q)
	{
		$q['date'] = strftime(DATE_FORMAT, strtotime(substr(
						$q['timestamp'], 0, 19)));
		include('search_entry.tpl');
		$i++;
	}
	include('search_bottom.tpl');
}


function search_system($args)
{
	global $title;

	$title.=' - Search';
}

?>
