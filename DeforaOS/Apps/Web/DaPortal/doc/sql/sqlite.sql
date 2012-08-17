/* $Id$ */
/* This file is part of DeforaOS Web DaPortal */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */



BEGIN TRANSACTION;


CREATE TABLE daportal_module (
	module_id INTEGER PRIMARY KEY,
	name VARCHAR(255) UNIQUE,
	enabled TINYINT(4) NOT NULL DEFAULT '0'
);

INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_config (
	config_id INTEGER PRIMARY KEY,
	module_id INTEGER NOT NULL,
	title VARCHAR(255) NOT NULL,
	type VARCHAR(255) NOT NULL,
	name VARCHAR(255) NOT NULL,
	value_bool BOOLEAN DEFAULT NULL,
	value_int INTEGER DEFAULT NULL,
	value_string VARCHAR(255) DEFAULT NULL,
	UNIQUE (module_id, name),
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id)
);
CREATE TABLE daportal_config_enum_type (
	name VARCHAR(255)
);
INSERT INTO daportal_config_enum_type (name) VALUES ('bool');
INSERT INTO daportal_config_enum_type (name) VALUES ('int');
INSERT INTO daportal_config_enum_type (name) VALUES ('string');


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) PRIMARY KEY,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT FALSE
);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_group (
	group_id INTEGER PRIMARY KEY,
	groupname VARCHAR(255) UNIQUE,
	enabled BOOLEAN DEFAULT FALSE
);
INSERT INTO daportal_group (group_id, groupname, enabled) VALUES ('0', 'nogroup', '1');


CREATE TABLE daportal_user (
	user_id INTEGER PRIMARY KEY,
	username VARCHAR(255) UNIQUE,
	group_id INTEGER NOT NULL DEFAULT 0,
	password VARCHAR(255),
	enabled BOOLEAN DEFAULT FALSE,
	admin BOOLEAN DEFAULT FALSE,
	fullname VARCHAR(255) DEFAULT '',
	email VARCHAR(255) NOT NULL,
	FOREIGN KEY (group_id) REFERENCES daportal_group (group_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_user (user_id, username, password, enabled, fullname, email) VALUES ('0', 'Anonymous', '', '1', 'Anonymous user', '');
INSERT INTO daportal_user (username, password, enabled, admin, fullname, email) VALUES ('admin', '$1$?0p*PI[G$kbHyE5VE/S32UrV88Unz/1', '1', '1', 'Administrator', 'username@domain.tld');

CREATE TABLE daportal_user_register (
	user_register_id INTEGER PRIMARY KEY,
	user_id INTEGER,
	token VARCHAR(255) UNIQUE NOT NULL,
	timestamp TIMESTAMP DEFAULT NULL,
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
CREATE TRIGGER daportal_user_register_insert_timestamp AFTER INSERT ON daportal_user_register
BEGIN
	UPDATE daportal_user_register SET timestamp = datetime('now') WHERE user_id = NEW.user_id;
END;

CREATE TABLE daportal_user_reset (
	user_reset_id INTEGER PRIMARY KEY,
	user_id INTEGER,
	token VARCHAR(255) UNIQUE NOT NULL,
	timestamp TIMESTAMP DEFAULT NULL,
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
CREATE TRIGGER daportal_user_reset_insert_timestamp AFTER INSERT ON daportal_user_reset
BEGIN
	UPDATE daportal_user_reset SET timestamp = datetime('now') WHERE user_id = NEW.user_id;
END;

CREATE TABLE daportal_content (
	content_id INTEGER PRIMARY KEY,
	timestamp TIMESTAMP DEFAULT NULL,
	module_id INTEGER,
	user_id INTEGER,
	group_id INTEGER DEFAULT '0',
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	public BOOLEAN NOT NULL DEFAULT FALSE,
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id),
	FOREIGN KEY (group_id) REFERENCES daportal_group (group_id)
);
CREATE TRIGGER daportal_content_insert_timestamp AFTER INSERT ON daportal_content
BEGIN
	UPDATE daportal_content SET timestamp = datetime('now') WHERE content_id = NEW.content_id;
END;
CREATE TABLE daportal_content_lang (
	content_lang_id INTEGER PRIMARY KEY,
	content_id INTEGER,
	lang_id VARCHAR(2),
	title VARCHAR(255),
	content TEXT,
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (lang_id) REFERENCES daportal_lang (lang_id)
);


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id INTEGER,
	parent INTEGER,
	FOREIGN KEY (comment_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('comment', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id INTEGER PRIMARY KEY,
	name VARCHAR(255),
	link VARCHAR(255)
);
INSERT INTO daportal_module (name, enabled) VALUES ('top', '0');


/* module: project */
CREATE TABLE daportal_project (
	project_id INTEGER,
	synopsis VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
);

CREATE TABLE daportal_project_download (
	project_download_id INTEGER PRIMARY KEY,
	project_id INTEGER NOT NULL,
	download_id INTEGER NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id),
	FOREIGN KEY (download_id) REFERENCES daportal_download (content_id)
);

CREATE TABLE daportal_project_screenshot (
	project_screenshot_id INTEGER PRIMARY KEY,
	project_id INTEGER NOT NULL,
	download_id INTEGER NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id),
	FOREIGN KEY (download_id) REFERENCES daportal_download (content_id)
);

CREATE TABLE daportal_project_user (
	project_user_id INTEGER PRIMARY KEY,
	project_id INTEGER NOT NULL,
	user_id INTEGER NOT NULL,
	admin BOOLEAN NOT NULL DEFAULT FALSE,
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);

CREATE TABLE daportal_bug_enum_state (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_enum_state (name) VALUES ('New');
INSERT INTO daportal_bug_enum_state (name) VALUES ('Assigned');
INSERT INTO daportal_bug_enum_state (name) VALUES ('Closed');
INSERT INTO daportal_bug_enum_state (name) VALUES ('Fixed');
INSERT INTO daportal_bug_enum_state (name) VALUES ('Implemented');
INSERT INTO daportal_bug_enum_state (name) VALUES ('Re-opened');

CREATE TABLE daportal_bug_enum_type (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_enum_type (name) VALUES ('Major');
INSERT INTO daportal_bug_enum_type (name) VALUES ('Minor');
INSERT INTO daportal_bug_enum_type (name) VALUES ('Functionality');
INSERT INTO daportal_bug_enum_type (name) VALUES ('Feature');

CREATE TABLE daportal_bug_enum_priority (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_enum_priority (name) VALUES ('Urgent');
INSERT INTO daportal_bug_enum_priority (name) VALUES ('High');
INSERT INTO daportal_bug_enum_priority (name) VALUES ('Medium');
INSERT INTO daportal_bug_enum_priority (name) VALUES ('Low');

CREATE TABLE daportal_bug (
	bug_id INTEGER PRIMARY KEY,
	content_id INTEGER,
	project_id INTEGER,
	state VARCHAR(255) DEFAULT 'New',
	type VARCHAR(255),
	priority VARCHAR(255) DEFAULT 'Medium',
	assigned INTEGER,
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id),
	FOREIGN KEY (state) REFERENCES daportal_bug_enum_state (name),
	FOREIGN KEY (type) REFERENCES daportal_bug_enum_type (name),
	FOREIGN KEY (priority) REFERENCES daportal_bug_enum_type (name)
);

CREATE TABLE daportal_bug_reply_enum_state (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('New');
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('Assigned');
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('Closed');
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('Fixed');
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('Implemented');
INSERT INTO daportal_bug_reply_enum_state (name) VALUES ('Re-opened');

CREATE TABLE daportal_bug_reply_enum_type (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_reply_enum_type (name) VALUES ('Major');
INSERT INTO daportal_bug_reply_enum_type (name) VALUES ('Minor');
INSERT INTO daportal_bug_reply_enum_type (name) VALUES ('Functionality');
INSERT INTO daportal_bug_reply_enum_type (name) VALUES ('Feature');

CREATE TABLE daportal_bug_reply_enum_priority (
	name VARCHAR(255)
);
INSERT INTO daportal_bug_reply_enum_priority (name) VALUES ('Urgent');
INSERT INTO daportal_bug_reply_enum_priority (name) VALUES ('High');
INSERT INTO daportal_bug_reply_enum_priority (name) VALUES ('Medium');
INSERT INTO daportal_bug_reply_enum_priority (name) VALUES ('Low');

CREATE TABLE daportal_bug_reply (
	bug_reply_id INTEGER PRIMARY KEY,
	content_id INTEGER,
	bug_id INTEGER,
	state VARCHAR(255),
	type VARCHAR(255),
	priority VARCHAR(255),
	assigned INTEGER,
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (bug_id) REFERENCES daportal_bug (bug_id)
	FOREIGN KEY (state) REFERENCES daportal_bug_reply_enum_state (name),
	FOREIGN KEY (type) REFERENCES daportal_bug_reply_enum_type (name),
	FOREIGN KEY (priority) REFERENCES daportal_bug_reply_enum_type (name)
);

INSERT INTO daportal_module (name, enabled) VALUES ('project', '1');


/* module: probe */
CREATE TABLE daportal_probe_host (
	host_id INTEGER,
	FOREIGN KEY (host_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('probe', '0');


/* module: bookmark */
CREATE TABLE daportal_bookmark (
	bookmark_id INTEGER,
	url VARCHAR(256),
	FOREIGN KEY (bookmark_id) REFERENCES daportal_content (content_id)
);

INSERT INTO daportal_module (name, enabled) VALUES ('bookmark', '0');


/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id INTEGER PRIMARY KEY,
	category_id INTEGER NOT NULL,
	content_id INTEGER NOT NULL,
	FOREIGN KEY (category_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('category', '0');


/* module: download */
CREATE TABLE daportal_download (
	download_id INTEGER PRIMARY KEY,
	content_id INTEGER NOT NULL,
	parent INTEGER,
	mode SMALLINT DEFAULT '420',
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_download (download_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('download', '1');


/* module: article */
INSERT INTO daportal_module (name, enabled) VALUES ('article', '1');


/* module: wiki */
INSERT INTO daportal_module (name, enabled) VALUES ('wiki', '1');


/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', '0');


/* module: pki */
INSERT INTO daportal_module (name, enabled) VALUES ('pki', '0');

CREATE TABLE daportal_ca (
	ca_id INTEGER,
	parent INTEGER DEFAULT NULL,
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255),
	FOREIGN KEY (ca_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_ca (ca_id)
);

CREATE TABLE daportal_caclient (
	caclient_id INTEGER,
	parent INTEGER,
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255),
	FOREIGN KEY (caclient_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_ca (ca_id)
);

CREATE TABLE daportal_caserver (
	caserver_id INTEGER,
	parent INTEGER,
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255),
	FOREIGN KEY (caserver_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_ca (ca_id)
);


/* module: browser */
INSERT INTO daportal_module (name, enabled) VALUES ('browser', '1');


/* module: translate */
INSERT INTO daportal_module (name, enabled) VALUES ('translate', '0');


/* module: blog */
CREATE TABLE daportal_blog_user (
	blog_user_id INTEGER,
	theme VARCHAR(255),
	FOREIGN KEY (blog_user_id) REFERENCES daportal_content (content_id)
);

CREATE TABLE daportal_blog_content (
	blog_content_id INTEGER,
	comment BOOLEAN DEFAULT FALSE,
	FOREIGN KEY (blog_content_id) REFERENCES daportal_content (content_id)
);

INSERT INTO daportal_module (name, enabled) VALUES ('blog', '1');


COMMIT;
