<?php //$Id$
//Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>
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



//private
//variables
global $xml_blacklisted, $xml_attrib_whitelist, $xml_tag_whitelist,
       $xml_content;
$xml_blacklisted = 1;
$xml_attrib_whitelist = array('alt', 'border', 'class', 'colspan', 'height',
		'href', 'size', 'src', 'style', 'title', 'width');
$xml_tag_whitelist = array('a', 'acronym', 'b', 'big', 'br', 'center', 'div',
		'font', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
		'hr', 'i', 'img', 'li', 'ol', 'p', 'pre', 'span', 'sub', 'sup',
		'table', 'tbody', 'td', 'th', 'tr', 'tt', 'u', 'ul');
$xml_content = '';


//functions
function _xml_text($content, &$error = '')
{
	global $xml_content;

	$xml_content = '';
	//FIXME factorize this code with validation
	$content = str_replace(array('<br>', '<hr>'), array('<br/>', '<hr/>'),
			$content);
	$content = str_replace(array('&lt;', '&gt;', '&quot;', '&'),
			array('<', '>', '"', '&amp;'), htmlentities($content));
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$content = '<div>'.$content.'</div>';
	$parser = xml_parser_create(); //encoding should not matter
	xml_set_character_data_handler($parser, '_xml_text_update_data');
	if(($ret = xml_parse($parser, '<div>'.$content.'</div>', TRUE)) != 1)
		$error = xml_error_string(xml_get_error_code($parser))
			.' at line '.xml_get_current_line_number($parser)
			.', column '.xml_get_current_column_number($parser);
	xml_parser_free($parser);
	if($ret == 1)
		return str_replace(array('&amp;', '&lt;', '&gt;'),
				array('&', '<', '>'), $xml_content);
	$xml_content = '';
	return FALSE;
}

function _xml_text_update_data($parser, $data)
{
	global $xml_content;

	_warning($data, 0);
	$xml_content.=$data;
}


function _xml_validate($content, &$error = '')
{
	global $xml_blacklisted, $xml_error;

	$content = str_replace(array('<br>', '<hr>'), array('<br/>', '<hr/>'),
			$content);
	$content = str_replace(array('&lt;', '&gt;', '&quot;', '&'),
			array('<', '>', '"', '&amp;'), htmlentities($content));
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$content = '<div>'.$content.'</div>';
	$parser = xml_parser_create(); //encoding should not matter
	if(xml_set_element_handler($parser, '_xml_validate_element_start',
				'_xml_validate_element_end') != TRUE)
	{
		xml_parser_free($parser);
		return FALSE;
	}
	$xml_blacklisted = 0;
	$xml_error = '';
	if(($ret = xml_parse($parser, $content, TRUE)) != 1)
		$error = xml_error_string(xml_get_error_code($parser))
			.' at line '.xml_get_current_line_number($parser)
			.', column '.xml_get_current_column_number($parser);
	xml_parser_free($parser);
	if($xml_blacklisted != 0 || strlen($xml_error) != 0)
	{
		$error = $xml_error;
		return FALSE;
	}
	$xml_blacklisted = 1;
	if($ret == 1)
		return TRUE;
	_error('XML error: '.$error, 0);
	return FALSE;
}

function _xml_validate_element_start($parser, $name, $attribs)
{
	global $xml_blacklisted, $xml_error, $xml_attrib_whitelist,
	       $xml_tag_whitelist;

	//return immediately if already detected as invalid
	//FIXME disable check in parser instead if possible
	if($xml_blacklisted != 0)
		return;
	//check the element name
	$wcnt = count($xml_tag_whitelist);
	for($i = 0; $i < $wcnt; $i++)
		if(strcasecmp($name, $xml_tag_whitelist[$i]) == 0)
			break;
	if($i == $wcnt) //tag not found
	{
		$xml_blacklisted = 1;
		$xml_error = 'The tag "'.$name.'" is forbidden';
		return;
	}
	//check every attribute
	$keys = array_keys($attribs);
	$attr = array();
	$i = 0;
	foreach($keys as $k)
		$attr[$i++] = $k;
	$acnt = count($attr);
	$wcnt = count($xml_attrib_whitelist);
	for($i = 0; $i < $acnt; $i++)
	{
		$a = $attr[$i];
		for($j = 0; $j < $wcnt; $j++)
			if(strcasecmp($a, $xml_attrib_whitelist[$j]) == 0)
				break;
		if($j == $wcnt) //attrib not found
			break;
	}
	if($i == $acnt)
		return;
	$xml_error = 'The attribute "'.$a.'" is forbidden';
	$xml_blacklisted = 1;
}

function _xml_validate_element_end($parser, $name)
{
}

?>
