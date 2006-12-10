<?php //system/lang.php



//check url
if(strcmp($_SERVER['SCRIPT_NAME'], $_SERVER['PHP_SELF']) != 0
		|| !ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


function _lang($text)
{
	$keys = array_keys($text);
	foreach($keys as $k)
		if(!defined($k))
			define($k, $text[$k]);
}


function _lang_check($lang)
{
	return _sql_single('SELECT enabled FROM daportal_lang'
			." WHERE enabled='1' AND lang_id='$lang'");
}


if(isset($_POST['lang']) && _lang_check($_POST['lang']))
{
	session_start();
	$lang = $_POST['lang'];
	$_SESSION['lang'] = $lang;
}
else if(isset($_SESSION['lang']) && _lang_check($_SESSION['lang']))
	$lang = $_SESSION['lang'];
else if(isset($_SERVER['HTTP_ACCEPT_LANGUAGE']))
{
	for($hal = $_SERVER['HTTP_ACCEPT_LANGUAGE'];
			ereg('^,?([a-zA-Z]+)(;q=[0-9.]+)?(.*)$', $hal, $regs);
			$hal = $regs[3])
	{
		if(!_lang_check($regs[1]))
			continue;
		$lang = $regs[1];
		break;
	}
}
if(!isset($lang) && !($lang = _config_get('admin', 'lang'))
		&& !_lang_check($lang))
	$lang = 'en';
$locale = $lang.'_'.strtoupper($lang);
if(!setlocale(LC_ALL, $locale.'@euro', $locale, $lang))
	_warning('Unable to set locale');
//lang
$text = array();
$text['_BY_'] = ' by ';
$text['_FOR_'] = ' for ';
$text['ABOUT'] = 'About';
$text['ADMINISTRATOR'] = 'Administrator';
$text['AUTHOR'] = 'Author';
$text['BACK'] = 'Back';
$text['CONTENT'] = 'Content';
$text['DATE'] = 'Date';
$text['DATE_FORMAT'] = '%A, %B %e %Y, %H:%M';
$text['DELETE'] = 'Delete';
$text['DESCRIPTION'] = 'Description';
$text['DISABLE'] = 'Disable';
$text['DISABLED'] = 'Disabled';
$text['DOWNLOADS'] = 'Downloads';
$text['EDIT'] = 'Edit';
$text['ENABLE'] = 'Enable';
$text['ENABLED'] = 'Enabled';
$text['FILTER'] = 'Filter';
$text['FORWARD'] = 'Forward';
$text['HOMEPAGE'] = 'Homepage';
$text['INVALID_ARGUMENT'] = 'Invalid argument';
$text['LOGIN'] = 'Login';
$text['LOGOUT'] = 'Logout';
$text['MEMBERS'] = 'Members';
$text['MESSAGE'] = 'Message';
$text['MODIFY'] = 'Modify';
$text['NAME'] = 'Name';
$text['NEWS'] = 'News';
$text['NO'] = 'No';
$text['PASSWORD'] = 'Password';
$text['PERMISSION_DENIED'] = 'Permission denied';
$text['PREVIEW'] = 'Preview';
$text['READ'] = 'Read';
$text['REFRESH'] = 'Refresh';
$text['REPLY'] = 'Reply';
$text['SEARCH'] = 'Search';
$text['SEND'] = 'Send';
$text['SUBMIT'] = 'Submit';
$text['TITLE'] = 'Title';
$text['TYPE'] = 'Type';
$text['UPDATE'] = 'Update';
$text['USERNAME'] = 'Username';
$text['YES'] = 'Yes';
if($lang == 'de')
{
	include('./system/lang.de.php');
}
else if($lang == 'fr')
{
	include('./system/lang.fr.php');
}
_lang($text);

?>
