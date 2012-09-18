<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



require_once('./system/format.php');


//Mail
class Mail
{
	//public
	//methods
	//static
	//useful
	//Mail::send
	static public function send($engine, $from, $to, $subject, $page,
			$headers = array())
	{
		global $config;

		if($from === FALSE)
			//FIXME try the configuration file as well
			$from = $_SERVER['SERVER_ADMIN'];
		$hdr = "From: $from\n";
		if(($charset = $config->getVariable('defaults', 'charset'))
				=== FALSE)
			$charset = 'utf-8';
		$charset = strtoupper($charset);
		//XXX escape $charset
		$hdr .= "Content-type: text/plain; charset=$charset\r\n";
		if(is_array($headers))
			foreach($headers as $h)
				$hdr .= "$h\n";
		$content = Mail::pageToText($engine, $page);
		//FIXME check $from, $to and $subject for newline characters
		if(mail($to, $subject, $content, $hdr) === FALSE)
			return $engine->log('LOG_ERR', 'Could not send e-mail');
		$engine->log('LOG_DEBUG', 'e-mail sent to '.$to);
		return TRUE;
	}


	//protected
	//methods
	//static
	//useful
	//Mail::pageToHTML
	static protected function pageToHTML($engine, $page)
	{
		if(($format = Format::attachDefault($engine, 'text/html'))
				=== FALSE)
			return FALSE;
		ob_start();
		$format->render($engine, $page);
		$str = ob_get_contents();
		ob_end_clean();
		return $str;
	}


	//Mail::pageToText
	static protected function pageToText($engine, $page)
	{
		if(($format = Format::attachDefault($engine, 'text/plain'))
				=== FALSE)
			return FALSE;
		$format->setParameter('wrap', 72);
		ob_start();
		$format->render($engine, $page);
		$str = ob_get_contents();
		ob_end_clean();
		return $str;
	}
}

?>
