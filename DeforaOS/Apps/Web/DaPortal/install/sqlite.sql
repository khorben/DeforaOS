/* install/sqlite.sql */



CREATE TABLE daportal_module (
	module_id INTEGER PRIMARY KEY,
	name VARCHAR(255) UNIQUE,
	enabled TINYINT(4) NOT NULL DEFAULT '0'
);
INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('menu', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_config (
	module_id INTEGER NOT NULL,
	name VARCHAR(255) NOT NULL,
	value VARCHAR(255) NOT NULL,
	PRIMARY KEY (module_id, name),
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id)
);
INSERT INTO daportal_config (module_id, name, value) VALUES ('1', 'lang', 'en');
INSERT INTO daportal_config (module_id, name, value) VALUES ('1', 'title', 'DaPortal');


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) PRIMARY KEY,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT FALSE
);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_user (
	user_id INTEGER PRIMARY KEY,
	username VARCHAR(255) UNIQUE,
	password CHAR(32),
	enabled BOOLEAN DEFAULT FALSE,
	admin BOOLEAN DEFAULT FALSE,
	email VARCHAR(255) NOT NULL
);
INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_config (module_id, name, value) VALUES ('5', 'register', 't');
INSERT INTO daportal_user (user_id, username, password, email) VALUES ('0', 'Anonymous', '', '');
INSERT INTO daportal_user (username, password, enabled, admin, email) VALUES ('admin', '5f4dcc3b5aa765d61d8327deb882cf99', '1', '1', 'username@domain.tld');

CREATE TABLE daportal_user_register (
	user_id INTEGER,
	key CHAR(32) UNIQUE NOT NULL,
	timestamp TIMESTAMP NOT NULL DEFAULT now,
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);


CREATE TABLE daportal_content (
	content_id INTEGER PRIMARY KEY,
	timestamp TIMESTAMP NOT NULL DEFAULT now,
	module_id INTEGER,
	user_id INTEGER,
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('content', '1');


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id INTEGER PRIMARY KEY,
	parent INTEGER,
	FOREIGN KEY (comment_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('comment', '1');
INSERT INTO daportal_config (module_id, name, value) VALUES ('7', 'anonymous', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id INTEGER PRIMARY KEY,
	name VARCHAR(255),
	link VARCHAR(255)
);
INSERT INTO daportal_module (name, enabled) VALUES ('top', '1');


/* module: project */
CREATE TABLE daportal_project (
	project_id INTEGER PRIMARY KEY,
	name VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('project', '1');
