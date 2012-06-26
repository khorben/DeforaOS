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
		$from = array('<br>', '<hr>');
		$to = array('<br/>', '<hr/>');

		$parser = xml_parser_create();
		if(xml_set_element_handler($parser,
					array('HTML', '_filterElementStart'),
					array('HTML', '_filterElementEnd'))
				!== TRUE)
		{
			xml_parser_free($parser);
			return ''; //XXX report error
		}
		xml_set_character_data_handler($parser, array('HTML',
				'_filterCharacterData'));
		HTML::$content = '';
		//give it more chances to validate
		$content = str_ireplace($from, $to, $content);
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

	protected function _filterCharacterData($parser, $data)
	{
		HTML::$content .= $data;
	}

	protected function _filterElementStart($parser, $name, $attributes)
	{
		//XXX report errors
		$tag = strtolower($name);
		if(!isset(HTML::$whitelist[$tag]))
			return;
		HTML::$content .= "<$tag";
		$a = HTML::$whitelist[$tag];
		foreach($attributes as $k => $v)
			if(!isset($a[$k]))
				continue;
			else
				HTML::$content .= " $k=\"$v\"";
		HTML::$content .= ">";
	}

	protected function _filterElementEnd($parser, $name)
	{
		$tag = strtolower($name);
		if(!isset(HTML::$whitelist[$tag]))
		{
			HTML::$valid = FALSE;
			return;
		}
		HTML::$content .= "</$tag>";
	}


	//HTML::validate
	static public function validate($engine, $content)
	{
		$parser = xml_parser_create();
		if(xml_set_element_handler($parser,
					array('HTML', '_validateElementStart'),
					array('HTML', '_validateElementEnd'))
				!== TRUE)
		{
			xml_parser_free($parser);
			return FALSE;
		}
		HTML::$valid = TRUE;
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
			if(!isset($a[$k]))
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
	static protected $content;
	static protected $valid;
	static protected $whitelist = array(
			'a' => array('href', 'name'),
			'b' => array(),
			'br' => array(),
			'div' => array(),
			'h1' => array(),
			'h2' => array(),
			'h3' => array(),
			'h4' => array(),
			'h5' => array(),
			'h6' => array(),
			'hr' => array(),
			'i' => array(),
			'img' => array('alt', 'src'),
			'li' => array(),
			'ol' => array(),
			'span' => array(),
			'p' => array(),
			'u' => array(),
			'ul' => array());
}

?>
