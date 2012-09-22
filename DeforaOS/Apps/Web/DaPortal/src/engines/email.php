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



require_once('./engines/cli.php');
require_once('./system/mail.php');
require_once('./system/user.php');


//EmailEngine
class EmailEngine extends CliEngine
{
	//essential
	//EmailEngine::match
	public function match()
	{
		//never match by default
		return 0;
	}


	//useful
	//EmailEngine::render
	public function render($page)
	{
		$cred = $this->getCredentials();
		$user = new User($this, $cred->getUserId(),
				$cred->getUsername());

		if($user->getUserId() == 0)
		{
			fprintf(STDERR, "%s\n", "daportal: Could not determine"
					." the e-mail address");
			return;
		}
		$email = $user->getFullname().' <'.$user->getEmail().'>';
		$template = Template::attachDefault($this);
		if($template !== FALSE)
			$page = $template->render($this, $page);
		if(($output = Format::attachDefault($this, $this->getType()))
					=== FALSE)
			fprintf(STDERR, "%s\n", "daportal: Could not determine"
					." the proper output format");
		else if(Mail::send($this, $email, $email,
				$page->getProperty('title'), $page) === FALSE)
			fprintf(STDERR, "%s\n", "daportal: Could not send"
					." the message");
	}
}

?>
