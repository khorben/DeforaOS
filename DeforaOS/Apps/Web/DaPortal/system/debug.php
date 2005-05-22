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
function _debug_message($level, $message, $visible = 0)
{
	global $debug_messages, $html;

	$debug_messages.="\t".'<div class="'.$level.'"><b>'.(ucfirst($level))
			.':</b> '.htmlspecialchars($message)."</div>\n";
	if($visible && $html)
		print('<div class="debug"><div class="visible error">'
				.' <img src="images/'.$level.'.png"'
				.' alt="error"/>'.htmlspecialchars($message)
				."</div></div>\n");
}


function _error($message, $visible = 1)
{
	_debug_message('error', $message, $visible);
}


function _info($message, $visible = 0)
{
	_debug_message('info', $message, $visible);
}


function _warning($message, $visible = 0)
{
	_debug_message('warning', $message, $visible);
}

?>
