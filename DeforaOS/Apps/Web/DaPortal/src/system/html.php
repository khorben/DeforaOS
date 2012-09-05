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



//HTML
class HTML
{
	//public
	//methods
	//static
	//useful
	//HTML::filter
	static public function filter($engine, $content)
	{
		$start = array('HTML', '_filterElementStart');
		$end = array('HTML', '_filterElementEnd');
		$filter = array('HTML', '_filterCharacterData');
		$from = array('<br>', '<hr>');
		$to = array('<br/>', '<hr/>');

		$parser = HTML::xmlParser();
		if(xml_set_element_handler($parser, $start, $end) !== TRUE)
		{
			xml_parser_free($parser);
			return ''; //XXX report error
		}
		xml_set_character_data_handler($parser, $filter);
		HTML::$content = '';
		//give it more chances to validate
		$content = str_ireplace($from, $to, $content);
		switch(strtolower(HTML::$charset))
		{
			case 'iso-8859-1':
			case 'iso-8859-15':
				//do not rely on input charset detection
				$content = utf8_encode($content);
				break;
		}
		//give it a root tag if it seems to need one
		if(strncmp('<!DOCTYPE', $content, 9) != 0
				&& strncmp('<?xml', $content, 4) != 0)
			$content = '<root>'.$content.'</root>';
		if(($ret = xml_parse($parser, $content, TRUE)) != 1)
		{
			$error = xml_error_string(xml_get_error_code($parser))
				.' at line '
				.xml_get_current_line_number($parser)
				.', column '
				.xml_get_current_column_number($parser);
			$engine->log('LOG_DEBUG', $error);
		}
		xml_parser_free($parser);
		return HTML::$content;
	}

	static protected function _filterCharacterData($parser, $data)
	{
		HTML::$content .= htmlspecialchars($data, ENT_NOQUOTES);
	}

	static protected function _filterElementStart($parser, $name,
			$attributes)
	{
		//XXX report errors
		$tag = strtolower($name);
		if(!isset(HTML::$whitelist[$tag]))
			return;
		HTML::$content .= "<$tag";
		$a = HTML::$whitelist[$tag];
		foreach($attributes as $k => $v)
		{
			$attr = strtolower($k);
			if(!in_array($attr, $a))
				continue;
			HTML::$content .= ' '.$attr.'="'
				.htmlspecialchars($v, ENT_COMPAT | ENT_HTML401,
						HTML::$charset).'"';
		}
		HTML::$content .= ">";
	}

	static protected function _filterElementEnd($parser, $name)
	{
		$tag = strtolower($name);
		if(!isset(HTML::$whitelist[$tag]) || $tag == 'br')
			return;
		HTML::$content .= "</$tag>";
	}


	//HTML::format
	static public function format($engine, $content)
	{
		$from = '/((ftp:\/\/|http:\/\/|https:\/\/|mailto:)([-+a-zA-Z0-9.:\/_%?!=,;~#@]|&amp;)+)/';
		//FIXME obfuscate e-mail addresses
		$to = '<a href="\1">\1</a>';

		$ret = '<div class="prewrapped">';
		$lines = explode("\n", $content);
		$list = 0;
		foreach($lines as $l)
		{
			$l = htmlspecialchars($l, ENT_COMPAT);
			$l = preg_replace($from, $to, $l);
			if(strlen($l) > 0 && $l[0] == ' ')
			{
				if(strlen($l) > 2 && $l[1] == '*'
						&& $l[2] == ' ')
				{
					//list
					$l = '<li>'.substr($l, 3).'</li>';
					if($list == 0)
					{
						$list = 1;
						$l = '<ul>'.$l;
					}
				}
				else
					//preformatted content
					$l = '<span class="preformatted">'
						.substr($l, 1).'</span>';
			}
			else if($list)
			{
				//close the list if necessary
				$l = '</ul>'.$l;
				$list = 0;
			}
			$ret .= $l.'<br />';
		}
		$ret .= '</div>';
		return $ret;
	}


	//HTML::validate
	static public function validate($engine, $content)
	{
		$parser = HTML::xmlParser();
		if(xml_set_element_handler($parser,
					array('HTML', '_validateElementStart'),
					array('HTML', '_validateElementEnd'))
				!== TRUE)
		{
			xml_parser_free($parser);
			return FALSE;
		}
		HTML::$valid = TRUE;
		switch(strtolower(HTML::$charset))
		{
			case 'iso-8859-1':
			case 'iso-8859-15':
				//do not rely on input charset detection
				$content = utf8_encode($content);
				break;
		}
		if(($ret = xml_parse($parser, $content, TRUE)) != 1)
		{
			$error = xml_error_string(xml_get_error_code($parser))
				.' at line '
				.xml_get_current_line_number($parser)
				.', column '
				.xml_get_current_column_number($parser);
			$engine->log('LOG_DEBUG', $error);
		}
		xml_parser_free($parser);
		return ($ret == 1) ? HTML::$valid : FALSE;
	}

	static protected function _validateElementStart($parser, $name,
			$attributes)
	{
		//XXX report errors
		$tag = strtolower($name);
		if(!isset(HTML::$whitelist[$tag]))
		{
			HTML::$valid = FALSE;
			return;
		}
		$a = HTML::$whitelist[$tag];
		foreach($attributes as $k => $v)
			if(!in_array(strtolower($k), $a))
			{
				HTML::$valid = FALSE;
				return;
			}
	}

	static protected function _validateElementEnd($parser, $name)
	{
	}


	//protected
	//properties
	static protected $charset = FALSE;
	static protected $content;
	static protected $valid;
	static protected $whitelist = array(
			'a' => array('href', 'name', 'rel', 'title'),
			'acronym' => array('class'),
			'b' => array('class'),
			'big' => array('class'),
			'br' => array(),
			'center' => array(),
			'div' => array('class'),
			'h1' => array('class'),
			'h2' => array('class'),
			'h3' => array('class'),
			'h4' => array('class'),
			'h5' => array('class'),
			'h6' => array('class'),
			'hr' => array('class'),
			'i' => array('class'),
			'img' => array('alt', 'src'),
			'li' => array('class'),
			'ol' => array('class'),
			'p' => array('class'),
			'pre' => array('class'),
			'small' => array('class'),
			'span' => array('class'),
			'sub' => array('class'),
			'sup' => array('class'),
			'table' => array('class'),
			'tbody' => array('class'),
			'td' => array('class'),
			'th' => array('class'),
			'tr' => array('class'),
			'tt' => array('class'),
			'u' => array('class'),
			'ul' => array('class'));


	//methods
	//helpers
	//HTML::xmlParser
	static protected function xmlParser($charset = FALSE)
	{
		global $config;

		//for escaping
		if(!defined('ENT_HTML401'))
			define('ENT_HTML401', 0);
		//for encoding
		if(HTML::$charset === FALSE)
			HTML::$charset = $config->getVariable('defaults',
					'charset');
		if($charset === FALSE)
			$charset = HTML::$charset;
		switch(strtolower($charset))
		{
			case 'ascii':
				return xml_parser_create('US-ASCII');
			case 'iso-8859-1':
			case 'iso-8859-15':
				return xml_parser_create('ISO-8859-1');
			case 'utf-8':
				return xml_parser_create('UTF-8');
		}
		return xml_parser_create('');
	}
}

?>
