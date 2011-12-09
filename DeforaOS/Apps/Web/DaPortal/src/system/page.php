<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//PageElement
class PageElement
{
	//members
	private $type;
	private $children = array();
	private $properties = array();


	//essential
	public function __construct($type, $properties = FALSE)
	{
		$this->type = $type;
		if(is_array($properties))
			foreach($properties as $key => $value)
				$this->setProperty($key, $value);
	}


	//accessors
	//PageElement::getChildren
	public function getChildren()
	{
		return $this->children;
	}


	public function getProperties()
	{
		return $this->properties;
	}


	public function getProperty($name, $default = FALSE)
	{
		if(!isset($this->properties[$name]))
			return $default;
		return $this->properties[$name];
	}


	public function getType()
	{
		return $this->type;
	}


	public function setProperty($name, $value)
	{
		$this->properties[$name] = $value;
	}


	public function setContent($content)
	{
		$this->content = $content;
	}


	//useful
	//PageElement::append
	public function append($type, $properties = FALSE)
	{
		$element = new PageElement($type);
		if(is_array($properties))
			foreach($properties as $key => $value)
				$element->setProperty($key, $value);
		return $this->appendElement($element);
	}


	//PageElement::appendElement
	public function appendElement($element)
	{
		$this->children[] = $element;
		return $element;
	}
}


//Page
class Page extends PageElement
{
	//essential
	public function __construct($properties = FALSE)
	{
		parent::__construct('page', $properties);
	}
}

?>
