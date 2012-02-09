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
	static public function send(&$engine, $from, $to, $subject, $page,
			$headers = array())
	{
		if($from === FALSE)
			//FIXME try the configuration file as well
			$from = $_SERVER['SERVER_ADMIN'];
		$hdr = "From: $from\n";
		//FIXME obtain the proper charset
		$hdr .= "Content-type: text/plain; charset=UTF-8\r\n";
		if(is_array($headers))
			foreach($headers as $h)
				$hdr .= "$h\n";
		$format = Format::attachDefault($engine, 'text/plain');
		ob_start();
		$format->render($engine, $page);
		$content = ob_get_contents();
		ob_end_clean();
		//FIXME check $from, $to and $subject for newline characters
		if(mail($to, $subject, $content, $hdr) === FALSE)
			$engine->log('LOG_ERR', 'Could not send e-mail');
		else
			$engine->log('LOG_DEBUG', 'e-mail sent to '.$to);
	}
}

?>
