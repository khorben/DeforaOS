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
	type VARCHAR(255) NOT NULL,
	name VARCHAR(255) NOT NULL,
	value_bool BOOLEAN DEFAULT NULL,
	value_int INTEGER DEFAULT NULL,
	value_string VARCHAR(255) DEFAULT NULL,
	PRIMARY KEY (module_id, name),
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id)
);
CREATE TABLE daportal_config_enum_type (
	name VARCHAR(255)
);
INSERT INTO daportal_config_enum_type (name) VALUES ('bool');
INSERT INTO daportal_config_enum_type (name) VALUES ('int');
INSERT INTO daportal_config_enum_type (name) VALUES ('string');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('1', 'string', 'globs', '/usr/share/mime/globs');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('1', 'string', 'lang', 'en');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('1', 'string', 'title', 'DaPortal');


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
INSERT INTO daportal_config (module_id, type, name, value_bool) VALUES ('5', 'bool', 'register', '0');
INSERT INTO daportal_config (module_id, type, name, value_bool) VALUES ('5', 'bool', 'manual', '1');
INSERT INTO daportal_user (user_id, username, password, email) VALUES ('0', 'Anonymous', '', '');
INSERT INTO daportal_user (username, password, enabled, admin, email) VALUES ('admin', '5f4dcc3b5aa765d61d8327deb882cf99', '1', '1', 'username@domain.tld');

CREATE TABLE daportal_user_register (
	user_id INTEGER,
	key CHAR(32) UNIQUE NOT NULL,
	timestamp TIMESTAMP DEFAULT NULL,
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
CREATE TRIGGER daportal_user_register_insert_timestamp AFTER INSERT ON daportal_user_register
BEGIN
	UPDATE daportal_user_register SET timestamp = datetime('now') WHERE user_id = NEW.user_id;
END;

CREATE TABLE daportal_content (
	content_id INTEGER PRIMARY KEY,
	timestamp TIMESTAMP DEFAULT NULL,
	module_id INTEGER,
	user_id INTEGER,
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
CREATE TRIGGER daportal_content_insert_timestamp AFTER INSERT ON daportal_content
BEGIN
	UPDATE daportal_content SET timestamp = datetime('now') WHERE content_id = NEW.content_id;
END;
INSERT INTO daportal_module (name, enabled) VALUES ('content', '1');


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id INTEGER,
	parent INTEGER,
	FOREIGN KEY (comment_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('comment', '1');
INSERT INTO daportal_config (module_id, type, name, value_bool) VALUES ('8', 'bool', 'anonymous', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id INTEGER PRIMARY KEY,
	name VARCHAR(255),
	link VARCHAR(255)
);
INSERT INTO daportal_module (name, enabled) VALUES ('top', '1');


/* module: project */
CREATE TABLE daportal_project (
	project_id INTEGER,
	synopsis VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
);
CREATE TABLE daportal_project_user (
	project_user_id INTEGER PRIMARY KEY,
	project_id INTEGER NOT NULL,
	user_id INTEGER NOT NULL,
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
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('10', 'string', 'cvsroot', '');


/* module: probe */
CREATE TABLE daportal_probe_host (
	host_id INTEGER,
	FOREIGN KEY (host_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('probe', '1');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('11', 'string', 'RRD_repository', '/tmp');


/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id INTEGER PRIMARY KEY,
	category_id INTEGER NOT NULL,
	content_id INTEGER NOT NULL,
	FOREIGN KEY (category_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('category', '1');


/* module: download */
CREATE TABLE daportal_download (
	download_id INTEGER PRIMARY KEY,
	content_id INTEGER NOT NULL,
	parent INTEGER,
	mode SMALLINT DEFAULT '0420',
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_download (download_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('download', '1');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('13', 'string', 'root', '/tmp');


/* module: article */
INSERT INTO daportal_module (name, enabled) VALUES ('article', '1');

/* module: wiki */
INSERT INTO daportal_module (name, enabled) VALUES ('wiki', '1');
INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('15', 'string', 'root', '');

/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', '1');

/* module: pki */
INSERT INTO daportal_module (name, enabled) VALUES ('pki', '1');

CREATE TABLE daportal_ca (
	ca_id INTEGER,
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255),
	FOREIGN KEY (ca_id) REFERENCES daportal_content (content_id)
);

CREATE TABLE daportal_caclient (
	caclient_id INTEGER,
	ca_id INTEGER,
	country VARCHAR(2),
	state VARCHAR(255),
	locality VARCHAR(255),
	organization VARCHAR(255),
	section VARCHAR(255),
	cn VARCHAR(255),
	email VARCHAR(255),
	FOREIGN KEY (caclient_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (ca_id) REFERENCES daportal_ca (ca_id)
);

INSERT INTO daportal_config (module_id, type, name, value_string) VALUES ('16', 'string', 'root', '');
