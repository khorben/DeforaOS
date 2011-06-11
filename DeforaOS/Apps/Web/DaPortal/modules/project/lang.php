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



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));


//lang
$text['ACTION'] = 'Action';
$text['ADD_MEMBER_TO_PROJECT'] = 'Add member to project';
$text['ADMINISTRATION'] = 'Administration';
$text['AND_AWAITS_MODERATION'] = 'and awaits moderation';
$text['ASSIGNED_TO'] = 'Assigned to';
$text['BROWSE_REVISIONS'] = 'Browse revisions';
$text['BROWSE_SOURCE'] = 'Browse source';
$text['BUG_REPORT'] = 'Bug report';
$text['BUG_REPORTS'] = 'Bug reports';
$text['BUGS_BY'] = 'Bugs by';
$text['CVS_PATH'] = 'CVS path';
$text['DESCRIPTION'] = 'Description';
$text['DIRECTORY'] = 'Directory';
$text['DOWNLOAD_FILE'] = 'Download file';
$text['FILE'] = 'File';
$text['FILES'] = 'Files';
$text['INVALID_PROJECT'] = 'Invalid project';
$text['MANAGER'] = 'Manager';
$text['MEMBERS'] = 'Members';
$text['MODIFICATION_OF'] = 'Modification of';
$text['MODIFICATION_OF_BUG_HASH'] = 'Modification of bug #';
$text['MODIFICATION_OF_REPLY_TO_BUG_HASH'] = 'Modification of reply to bug #';
$text['NEW_PROJECT'] = 'New project';
$text['NEW_RELEASE'] = 'New release';
$text['NEW_REPORT'] = 'New report';
$text['NEW_SCREENSHOT'] = 'New screenshot';
$text['NO_CVS_REPOSITORY'] = 'This project does not have a CVS repository';
$text['PARENT_DIRECTORY'] = 'Parent directory';
$text['PRIORITY'] = 'Priority';
$text['PRIORITY_CHANGED_TO'] = 'Priority changed to';
$text['PROJECT'] = 'Project';
$text['PROJECT_LIST'] = 'Project list';
$text['PROJECT_NAME'] = 'Project name';
$text['PROJECTS'] = 'Projects';
$text['PROJECTS_ADMINISTRATION'] = 'Projects administration';
$text['PROJECTS_REGISTERED'] = 'project(s) registered';
$text['RELEASES'] = 'Releases';
$text['REPLY_BY'] = 'Reply by';
$text['REPLY_ON'] = 'on';
$text['REPLY_TO_BUG'] = 'Reply to bug';
$text['REPORT_A_BUG'] = 'Report a bug';
$text['REPORT_BUG_FOR'] = 'Report bug for';
$text['REPORT_LIST'] = 'Report list';
$text['REPORTER'] = 'Reporter';
$text['REVISION'] = 'Revision';
$text['SCREENSHOTS'] = 'Screenshots';
$text['SELECT_PROJECT_TO_BUG'] = 'Select project to bug';
$text['SETTINGS'] = 'Settings';
$text['SOURCE_CODE'] = 'Source code';
$text['STATE'] = 'State';
$text['STATISTICS'] = 'Statistics';
$text['STATE_CHANGED_TO'] = 'State changed to';
$text['SUBMITTER'] = 'Submitter';
$text['SYNOPSIS'] = 'Synopsis';
$text['THANK_YOU'] = 'Thank you';
$text['THERE_ARE'] = 'There are';
$text['TYPE_CHANGED_TO'] = 'Type changed to';
$text['TIMELINE'] = 'Timeline';
$text['UPLOAD'] = 'Upload';
$text['YOUR_BUG_IS_SUBMITTED'] = 'Your bug is submitted';

?>
