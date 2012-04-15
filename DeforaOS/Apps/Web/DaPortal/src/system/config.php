<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
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



//ConfigSection
class ConfigSection
{
	//properties
	//private
	private $variables = array();


	//methods
	//public
	//accessors
	//ConfigSection::getVariable
	public function getVariable($name)
	{
		if(!isset($this->variables[$name]))
			return FALSE;
		return $this->variables[$name];
	}


	//ConfigSection::setVariable
	public function setVariable($name, $value)
	{
		$this->variables[$name] = $value;
	}
}


//Config
class Config
{
	private $sections = array();


	//methods
	//public
	//essential
	//Config::Config
	public function __construct()
	{
		$this->sections[''] = new ConfigSection;
	}


	//accessors
	//Config::getVariable
	public function getVariable($section, $name)
	{
		if($section === FALSE)
			$section = '';
		if(!isset($this->sections[$section]))
			return FALSE;
		return $this->sections[$section]->getVariable($name);
	}


	//Config::setVariable
	public function setVariable($section, $name, $value)
	{
		if($section === FALSE)
			$section = '';
		if(!isset($this->sections[$section]))
			$this->sections[$section] = new ConfigSection;
		$this->sections[$section]->setVariable($name, $value);
	}


	//useful
	//Config::load
	public function load($filename)
	{
		$section = '';

		if(($fp = @fopen($filename, 'r')) === FALSE)
		{
			syslog(LOG_WARNING, $filename
					.': Could not load configuration file');
			return FALSE;
		}
		for($i = 1; ($line = fgets($fp)) !== FALSE; $i++)
		{
			if(preg_match("/^([a-zA-Z0-9-_: \t]+)=(.*)$/", $line,
						$matches) == 1)
				$this->setVariable($section, $matches[1],
						$matches[2]);
			else if(preg_match("/^[ \t]*\[([a-zA-Z0-9-_:\/ \t]+)\]"
						."[ \t]*$/", $line, $matches)
					== 1)
				$section = $matches[1];
			else if(preg_match("/^[ \t]*(#.*$)?$/", $line) == 1)
				continue;
			else
				syslog(LOG_WARNING, $filename.': Line '.$i
						.' could not be parsed');
		}
		fclose($fp);
		return TRUE;
	}
}

?>
