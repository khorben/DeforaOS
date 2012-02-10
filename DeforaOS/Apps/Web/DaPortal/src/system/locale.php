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



//Locale
class Locale
{
	//public
	//methods
	//static
	//useful
	//Locale::init
	static public function init($engine, $package = FALSE)
	{
		global $config;
		$package = ($package !== FALSE) ? $package : 'DaPortal';

		if(($prefix = $config->getVariable(FALSE, 'prefix')) === FALSE)
			$prefix = '/usr/local';
		$path = $prefix.'/share/locale';
		$charset = $config->getVariable('defaults', 'charset');
		if(($locale = $config->getVariable('defaults', 'locale'))
				=== FALSE)
			//FIXME the charset may be part of $locale here
			$locale = getenv('LC_ALL');
		//initialize gettext
		if(!function_exists('gettext'))
			//FIXME define stub functions (or import a library)
			return;
		if($locale !== FALSE && $charset !== FALSE)
		{
			strtoupper($charset);
			$locales = array($locale.'.'.$charset, $locale);
			putenv('LC_ALL='.$locale);
		}
		else
			$locales = $locale;
		if(setlocale(LC_ALL, $locales) === FALSE)
			$engine->log('LOG_WARNING',
					'The locale may not have been set');
		bindtextdomain('DaPortal', $path);
		textdomain('DaPortal');
	}
}

?>
