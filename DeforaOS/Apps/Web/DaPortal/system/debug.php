<?php //system/debug.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


$debug_messages = '';
$debug_time_before = gettimeofday();


function _debug()
{
	global $debug, $html, $debug_messages, $debug_time_before;

	if(!$debug || !$html)
		return;
	print('<div class="debug"><div class="system">'."\n");
	print($debug_messages);
	$debug_time_after = gettimeofday();
	$sec = $debug_time_after['sec'] - $debug_time_before['sec'];
	$usec = $debug_time_after['usec'] - $debug_time_before['usec'];
	if($usec < 0)
	{
		$usec = $debug_time_before['usec'] - $debug_time_after['usec'];
		$sec++;
	}
	print('<div class="info"><b>Info:</b> Page execution duration: '
			.$sec.'s and '.ceil($usec/1000).'ms</div>'."\n");
	print('</div></div>'."\n");
}


//PRE
//	$level is trusted
function _debug_message($level, $message, $visible = 0)
{
	global $debug, $debug_messages, $html;

	if(!$html)
		return;
	if($visible)
		print('<div class="debug"><div class="visible error">'
				.'<img src="images/'.$level.'.png"'
				.' alt="error"/> '.htmlspecialchars($message)
				."</div></div>\n");
	if(!$debug)
		return;
	$debug_messages.="\t".'<div class="'.$level.'"><b>'.(ucfirst($level))
			.':</b> '.htmlspecialchars($message)."</div>\n";
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
