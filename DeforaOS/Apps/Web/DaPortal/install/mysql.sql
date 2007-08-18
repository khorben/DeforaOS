/* install/mysql.sql */



CREATE TABLE daportal_module (
	module_id int(11) NOT NULL auto_increment,
	name varchar(255) NOT NULL,
	enabled tinyint(4) NOT NULL default '0',
	PRIMARY KEY (module_id),
	UNIQUE KEY name (name)
) TYPE=InnoDB;
INSERT INTO daportal_module (name, enabled) VALUES ('admin', 1);
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', 1);
INSERT INTO daportal_module (name, enabled) VALUES ('menu', 1);
INSERT INTO daportal_module (name, enabled) VALUES ('search', 1);


CREATE TABLE daportal_config (
	module_id int(11) NOT NULL auto_increment,
	name varchar(255) NOT NULL,
	value varchar(255) NOT NULL,
	KEY module_id (module_id)
) TYPE=InnoDB;
ALTER TABLE daportal_config
	ADD CONSTRAINT daportal_config_ibfk_1 FOREIGN KEY (module_id) REFERENCES daportal_module (module_id);

INSERT INTO daportal_config (module_id, name, value) VALUES (1, 'globs', '/usr/share/mime/globs');
INSERT INTO daportal_config (module_id, name, value) VALUES (1, 'lang', 'en');
INSERT INTO daportal_config (module_id, name, value) VALUES (1, 'title', 'DaPortal');


CREATE TABLE daportal_lang (
	lang_id char(2) NOT NULL,
	name varchar(255) NOT NULL,
	enabled tinyint(4) NOT NULL default '0',
	PRIMARY KEY (lang_id)
) TYPE=MyISAM;
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', 1);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', 1);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', 1);


CREATE TABLE daportal_user (
	user_id int(11) NOT NULL auto_increment,
	username varchar(255) NOT NULL,
	password varchar(32) NOT NULL,
	enabled tinyint(4) NOT NULL default '0',
	admin tinyint(4) NOT NULL default '0',
	email varchar(255) NOT NULL,
	PRIMARY KEY (user_id),
	UNIQUE KEY username (username)
) TYPE=InnoDB;
INSERT INTO daportal_module (name, enabled) VALUES ('user', 1);
INSERT INTO daportal_config (module_id, name, value) VALUES (5, 'register', '1');
INSERT INTO daportal_user (user_id, username, `password`, enabled, admin, email) VALUES (0, 'Anonymous', '', 0, 0, '');
INSERT INTO daportal_user (user_id, username, `password`, enabled, admin, email) VALUES (1, 'admin', '5f4dcc3b5aa765d61d8327deb882cf99', 1, 1, 'username@domain.tld');


CREATE TABLE daportal_user_register (
	user_id int(11) NOT NULL default '0',
	`key` char(32) NOT NULL,
	timestamp datetime(14) NOT NULL,
	UNIQUE KEY user_id (user_id, `key`)
) TYPE=InnoDB;
ALTER TABLE daportal_user_register
	ADD CONSTRAINT daportal_user_register_ibfk_1 FOREIGN KEY (user_id) REFERENCES daportal_user (user_id);


CREATE TABLE daportal_content (
	content_id int(11) NOT NULL auto_increment,
	timestamp datetime(14) NOT NULL,
	module_id int(11) NOT NULL,
	user_id int(11) NOT NULL,
	title varchar(255) NOT NULL,
	content text NOT NULL,
	enabled tinyint(4) NOT NULL default '0',
	PRIMARY KEY  (content_id),
	KEY module_id (module_id, user_id),
	KEY user_id (user_id)
) TYPE=InnoDB;
ALTER TABLE daportal_content
	ADD CONSTRAINT daportal_content_ibfk_1 FOREIGN KEY (module_id) REFERENCES daportal_module (module_id),
	ADD CONSTRAINT daportal_content_ibfk_2 FOREIGN KEY (user_id) REFERENCES daportal_user (user_id);
INSERT INTO daportal_module (name, enabled) VALUES ('content', 1);


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', 1);


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id int(11) NOT NULL default '0',
	parent int(11) default NULL,
	KEY parent (parent),
	KEY comment_id (comment_id)
) TYPE=InnoDB;
ALTER TABLE daportal_comment
	ADD CONSTRAINT daportal_comment_ibfk_2 FOREIGN KEY (parent) REFERENCES daportal_content (content_id),
	ADD CONSTRAINT daportal_comment_ibfk_1 FOREIGN KEY (comment_id) REFERENCES daportal_content (content_id);
INSERT INTO daportal_module (name, enabled) VALUES ('comment', 1);
INSERT INTO daportal_config (module_id, name, value) VALUES ('7', 'anonymous', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id int(11) NOT NULL auto_increment,
	name varchar(255) NOT NULL,
	link varchar(255) NOT NULL,
	PRIMARY KEY (top_id)
) TYPE=MyISAM;
INSERT INTO daportal_module (name, enabled) VALUES ('top', 1);


/* module: project */


/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', 1);


/* module: bookmark */


/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id int(11) NOT NULL default '0',
	category_id int(11) NOT NULL,
	content_id int(11) NOT NULL,
	PRIMARY KEY (category_content_id),
	KEY content_id (content_id),
	KEY category_id (category_id)
) TYPE=InnoDB;
ALTER TABLE daportal_category_content
	ADD CONSTRAINT daportal_category_content_ibfk_2 FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	ADD CONSTRAINT daportal_category_content_ibfk_1 FOREIGN KEY (category_id) REFERENCES daportal_content (content_id);


/* module: download */
CREATE TABLE daportal_download (
	download_id int(11) NOT NULL auto_increment,
	content_id int(11) NOT NULL,
	parent int(11) default NULL,
	`mode` smallint(6) NOT NULL default '420',
	PRIMARY KEY (download_id),
	KEY content_id (content_id, parent)
	) TYPE=InnoDB;
INSERT INTO daportal_module (name, enabled) VALUES ('download', 1);
INSERT INTO daportal_config (module_id, name, value) VALUES ('15', 'root', '/tmp');

