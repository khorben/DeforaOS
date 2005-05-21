<?php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


$debug_messages = '';


function _debug()
{
	global $debug, $html, $debug_messages;

	if(!$debug || !$html)
		return;
	print('<div class="debug">'."\n");
	print($debug_messages);
	print('</div>'."\n");
}


//PRE
//	$level is trusted
function _debug_message($level, $message)
{
	global $debug_messages;

	$debug_messages.="\t".'<div class="'.$level.'"><b>'.(ucfirst($level))
			.':</b> '.htmlspecialchars($message)."</div>\n";
}


function _error($message)
{
	_debug_message('error', $message);
}


function _info($message)
{
	_debug_message('info', $message);
}


function _warning($message)
{
	_debug_message('warning', $message);
}

?>
