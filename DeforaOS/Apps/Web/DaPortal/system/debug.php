<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



$debug_messages = '';
//FIXME force a timezone or use a different function than gettimeofday()
$debug_time_before = gettimeofday();


function _debug()
{
	global $debug, $html, $debug_messages, $debug_time_before;

	if(!$debug || !$html)
		return;
	$debug_time_after = gettimeofday();
	$sec = $debug_time_after['sec'] - $debug_time_before['sec'];
	$usec = $debug_time_after['usec'] - $debug_time_before['usec'];
	if($usec < 0)
	{
		$usec = -$usec;
		$sec++;
	}
	_info('Page execution duration: '.$sec.'s and '.ceil($usec / 1000).'ms',
			FALSE);
	print('<div class="debug system">'."\n");
	print($debug_messages);
}


//PRE
//	$level is trusted
function _debug_message($level, $title, $message, $visible = FALSE)
{
	global $debug, $debug_messages, $html;

	if(!$html)
		return;
	if($visible)
		print('<div class="debug"><div class="visible '.$level.'">'
				.'<b>'.htmlspecialchars($title)
				.'</b><br/>'.htmlspecialchars($message)
				."</div></div>\n");
	if(!$debug)
		return;
	$debug_messages.="\t".'<div class="'.$level.'"><b>'.(ucfirst($level))
			.':</b> '.htmlspecialchars($message)."</div>\n";
}


function _error($message, $visible = TRUE)
{
	global $engine;

	$engine->log('LOG_ERR', $message);
	if($visible)
		_debug_message('error', 'Error', $message, $visible);
}


function _info($message, $visible = FALSE)
{
	global $engine;

	$engine->log('LOG_WARN', $message);
	if($visible)
		_debug_message('info', 'Information', $message, $visible);
}


function _warning($message, $visible = FALSE)
{
	global $engine;

	$engine->log('LOG_WARN', $message);
	if($visible)
		_debug_message('warning', 'Warning', $message, $visible);
}

?>
