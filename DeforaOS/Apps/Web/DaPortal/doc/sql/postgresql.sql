/* $Id$ */
/* This file is part of DeforaOS Web DaPortal */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */



BEGIN TRANSACTION;


CREATE TABLE daportal_module (
	module_id SERIAL PRIMARY KEY,
	name VARCHAR(255) UNIQUE NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);

INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_config (
	config_id SERIAL PRIMARY KEY,
	module_id INTEGER NOT NULL REFERENCES daportal_module (module_id) ON DELETE CASCADE,
	title VARCHAR(255),
	type VARCHAR(255) CHECK (type IN ('bool', 'int', 'string')) NOT NULL,
	name VARCHAR(255) NOT NULL,
	value_bool BOOLEAN DEFAULT NULL,
	value_int INTEGER DEFAULT NULL,
	value_string VARCHAR(255) DEFAULT NULL
);


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) PRIMARY KEY,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);

INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_group (
	group_id SERIAL PRIMARY KEY,
	groupname VARCHAR(255) UNIQUE,
	enabled BOOLEAN DEFAULT FALSE
);
INSERT INTO daportal_group (group_id, groupname, enabled) VALUES ('0', 'nogroup', '1');


CREATE TABLE daportal_user (
	user_id SERIAL PRIMARY KEY,
	username VARCHAR(255) UNIQUE,
	group_id INTEGER NOT NULL DEFAULT 0,
	"password" VARCHAR(255),
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	admin BOOLEAN DEFAULT FALSE,
	fullname VARCHAR(255) DEFAULT '',
	email VARCHAR(255) NOT NULL
);

CREATE INDEX daportal_user_username_index ON daportal_user (username) WHERE enabled='1';

INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_user (user_id, username, password, email) VALUES ('0', 'Anonymous', '', '');
INSERT INTO daportal_user (username, password, enabled, admin, email) VALUES ('admin', '$1$?0p*PI[G$kbHyE5VE/S32UrV88Unz/1', '1', '1', 'username@domain.tld');


CREATE TABLE daportal_user_register (
	user_register_id SERIAL PRIMARY KEY,
	user_id INTEGER UNIQUE REFERENCES daportal_user (user_id) ON DELETE CASCADE,
	token VARCHAR(255) UNIQUE NOT NULL,
	"timestamp" TIMESTAMP NOT NULL DEFAULT now()
);

CREATE INDEX daportal_user_register_token_index ON daportal_user_register (token);


CREATE TABLE daportal_user_reset (
	user_reset_id SERIAL PRIMARY KEY,
	user_id INTEGER UNIQUE REFERENCES daportal_user (user_id) ON DELETE CASCADE,
	token VARCHAR(255) UNIQUE NOT NULL,
	"timestamp" TIMESTAMP NOT NULL DEFAULT now()
);

CREATE INDEX daportal_user_reset_token_index ON daportal_user_reset (token);


CREATE TABLE daportal_content (
	content_id SERIAL PRIMARY KEY,
	timestamp TIMESTAMP NOT NULL DEFAULT now(),
	module_id INTEGER REFERENCES daportal_module (module_id) ON DELETE RESTRICT,
	user_id INTEGER REFERENCES daportal_user (user_id) ON DELETE RESTRICT,
	group_id INTEGER DEFAULT 0 REFERENCES daportal_group (group_id) ON DELETE RESTRICT,
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	public BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE INDEX daportal_content_title_index ON daportal_content (title) WHERE enabled='1' AND public='1';

CREATE TABLE daportal_content_lang (
	content_lang_id SERIAL PRIMARY KEY,
	content_id INTEGER REFERENCES daportal_content (content_id),
	lang_id VARCHAR(2) REFERENCES daportal_lang (lang_id),
	title VARCHAR(255),
	content TEXT
);


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id INTEGER UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	parent INTEGER REFERENCES daportal_content (content_id) ON DELETE CASCADE
);

INSERT INTO daportal_module (name, enabled) VALUES ('comment', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id SERIAL PRIMARY KEY,
	name VARCHAR(255),
	link VARCHAR(255)
);

INSERT INTO daportal_module (name, enabled) VALUES ('top', '0');


/* module: project */
CREATE TABLE daportal_project (
	project_id INTEGER UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	synopsis VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL
);

CREATE TABLE daportal_project_download (
	project_download_id SERIAL PRIMARY KEY,
	project_id INTEGER NOT NULL REFERENCES daportal_project (project_id),
	download_id INTEGER NOT NULL REFERENCES daportal_content (content_id)
);

CREATE TABLE daportal_project_screenshot (
	project_screenshot_id SERIAL PRIMARY KEY,
	project_id INTEGER NOT NULL REFERENCES daportal_project (project_id),
	download_id INTEGER NOT NULL REFERENCES daportal_content (content_id)
);

CREATE TABLE daportal_project_user (
	project_user_id SERIAL PRIMARY KEY,
	project_id INTEGER NOT NULL REFERENCES daportal_project (project_id) ON DELETE CASCADE,
	user_id INTEGER NOT NULL REFERENCES daportal_user (user_id) ON DELETE CASCADE,
	admin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE daportal_bug (
	bug_id SERIAL PRIMARY KEY,
	content_id INTEGER UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	project_id INTEGER REFERENCES daportal_project (project_id) ON DELETE CASCADE,
	state varchar(11) CHECK (state IN ('New', 'Assigned', 'Closed', 'Fixed', 'Implemented', 'Re-opened')) NOT NULL DEFAULT 'New',
	type varchar(13) CHECK (type IN ('Major', 'Minor', 'Functionality', 'Feature')) NOT NULL,
	priority VARCHAR(6) CHECK (priority IN ('Urgent', 'High', 'Medium', 'Low')) NOT NULL DEFAULT 'Medium',
	assigned INTEGER DEFAULT NULL REFERENCES daportal_user (user_id) ON DELETE SET NULL
);
CREATE TABLE daportal_bug_reply (
	bug_reply_id SERIAL PRIMARY KEY,
	content_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	bug_id INTEGER NOT NULL REFERENCES daportal_bug (bug_id) ON DELETE CASCADE,
	state varchar(11) CHECK (state IN ('New', 'Assigned', 'Closed', 'Fixed', 'Implemented', 'Re-opened')),
	type varchar(13) CHECK (type IN ('Major', 'Minor', 'Functionality', 'Feature')),
	priority VARCHAR(6) CHECK (priority IN ('Urgent', 'High', 'Medium', 'Low')),
	assigned INTEGER DEFAULT NULL REFERENCES daportal_user (user_id) ON DELETE SET NULL
);

INSERT INTO daportal_module (name, enabled) VALUES ('project', '1');


/* module: probe */
INSERT INTO daportal_module (name, enabled) VALUES ('probe', '0');


/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', '0');


/* module: bookmark */
CREATE TABLE daportal_bookmark (
	bookmark_id INTEGER NOT NULL UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	url VARCHAR(256)
);

INSERT INTO daportal_module (name, enabled) VALUES ('bookmark', '0');


/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id SERIAL PRIMARY KEY,
	category_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	content_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE CASCADE
);

INSERT INTO daportal_module (name, enabled) VALUES ('category', '0');


/* module: download */
CREATE TABLE daportal_download (
	download_id SERIAL PRIMARY KEY,
	content_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE RESTRICT, -- not UNIQUE allows hard links
	parent INTEGER REFERENCES daportal_download (download_id) ON DELETE RESTRICT,
	mode SMALLINT DEFAULT '420'
);

INSERT INTO daportal_module (name, enabled) VALUES ('download', '1');


/* module: article */
INSERT INTO daportal_module (name, enabled) VALUES ('article', '1');


/* module: wiki */
INSERT INTO daportal_module (name, enabled) VALUES ('wiki', '1');


/* module: pki */
CREATE TABLE daportal_ca (
	ca_id INTEGER NOT NULL UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	parent INTEGER DEFAULT NULL REFERENCES daportal_ca (ca_id),
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255)
);

CREATE TABLE daportal_caclient (
	caclient_id INTEGER NOT NULL UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	parent INTEGER DEFAULT NULL REFERENCES daportal_ca (ca_id),
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255)
);

CREATE TABLE daportal_caserver (
	caserver_id INTEGER NOT NULL UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	parent INTEGER DEFAULT NULL REFERENCES daportal_ca (ca_id),
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255)
);

INSERT INTO daportal_module (name, enabled) VALUES ('pki', '0');


/* module: browser */
INSERT INTO daportal_module (name, enabled) VALUES ('browser', '1');


/* module: translate */
INSERT INTO daportal_module (name, enabled) VALUES ('translate', '0');


/* module: blog */
CREATE TABLE daportal_blog_user (
	blog_user_id INTEGER NOT NULL REFERENCES daportal_content (content_id),
	theme VARCHAR(255)
);

CREATE TABLE daportal_blog_content (
	blog_content_id INTEGER REFERENCES daportal_content (content_id),
	comment BOOLEAN DEFAULT FALSE
);

INSERT INTO daportal_module (name, enabled) VALUES ('blog', '1');


COMMIT;
