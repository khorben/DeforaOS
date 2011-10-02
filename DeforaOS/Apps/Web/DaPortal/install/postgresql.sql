/* $Id$ */
/* install/postgresql.sql */



CREATE TABLE daportal_module (
	module_id SERIAL PRIMARY KEY,
	name VARCHAR(255) UNIQUE NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);

INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('menu', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_config (
	module_id SERIAL PRIMARY KEY,
	module_id INTEGER NOT NULL REFERENCES daportal_module (module_id) ON DELETE CASCADE,
	title VARCHAR(255),
	type VARCHAR(255) CHECK (type IN ('bool', 'int', 'string')) NOT NULL,
	name VARCHAR(255) NOT NULL,
	value_bool BOOLEAN DEFAULT NULL,
	value_int INTEGER DEFAULT NULL,
	value_string VARCHAR(255) DEFAULT NULL
);

INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('1', 'Path to MIME globs file', 'string', 'globs', '/usr/share/mime/globs');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('1', 'Default language', 'string', 'lang', 'en');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('1', 'Default theme', 'string', 'theme', 'DaPortal');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('1', 'Default title', 'string', 'title', 'DaPortal');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('4', 'Highlight search results', 'bool', 'title', FALSE);


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) PRIMARY KEY,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);

INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_user (
	user_id SERIAL PRIMARY KEY,
	username VARCHAR(255) UNIQUE,
	"password" CHAR(32),
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	admin BOOLEAN DEFAULT FALSE,
	email VARCHAR(255) NOT NULL
);

INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('5', 'Allow users to register new accounts', 'bool', 'register', '0');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('5', 'Moderate new user accounts', 'bool', 'manual', '1');
INSERT INTO daportal_user (user_id, username, password, email) VALUES ('0', 'Anonymous', '', '');
INSERT INTO daportal_user (username, password, enabled, admin, email) VALUES ('admin', '5f4dcc3b5aa765d61d8327deb882cf99', '1', '1', 'username@domain.tld');


CREATE TABLE daportal_user_register (
	user_id INTEGER UNIQUE REFERENCES daportal_user (user_id) ON DELETE CASCADE,
	"key" CHAR(32) UNIQUE NOT NULL,
	"timestamp" TIMESTAMP NOT NULL DEFAULT now()
);


CREATE TABLE daportal_content (
	content_id SERIAL PRIMARY KEY,
	timestamp TIMESTAMP NOT NULL DEFAULT now(),
	module_id INTEGER REFERENCES daportal_module (module_id) ON DELETE RESTRICT,
	user_id INTEGER REFERENCES daportal_user (user_id) ON DELETE RESTRICT,
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE daportal_content_lang (
	content_lang_id SERIAL PRIMARY KEY,
	content_id INTEGER REFERENCES daportal_content (content_id),
	lang_id VARCHAR(2) REFERENCES daportal_lang (lang_id),
	title VARCHAR(255),
	content TEXT
);

INSERT INTO daportal_module (name, enabled) VALUES ('content', '1');


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id INTEGER UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	parent INTEGER REFERENCES daportal_content (content_id) ON DELETE CASCADE
);

INSERT INTO daportal_module (name, enabled) VALUES ('comment', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('7', 'Allow anonymous comments', 'bool', 'anonymous', '0');


/* module: top */
CREATE TABLE daportal_top (
	top_id SERIAL PRIMARY KEY,
	name VARCHAR(255),
	link VARCHAR(255)
);

INSERT INTO daportal_module (name, enabled) VALUES ('top', '1');


/* module: project */
CREATE TABLE daportal_project (
	project_id INTEGER UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	synopsis VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL
);
CREATE TABLE daportal_project_user (
	project_user_id SERIAL PRIMARY KEY,
	project_id INTEGER NOT NULL REFERENCES daportal_project (project_id) ON DELETE CASCADE,
	user_id INTEGER NOT NULL REFERENCES daportal_user (user_id) ON DELETE CASCADE
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
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('10', 'Path to the local CVS repository', 'string', 'cvsroot', '');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('10', 'Path to the public CVS repository', 'string', 'repository', '');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('10', 'Allow anonymous bug reports', 'bool', 'anonymous_bug_reports', '0');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('10', 'Allow anonymous bug replies', 'bool', 'anonymous_bug_replies', '0');


/* module: probe */
INSERT INTO daportal_module (name, enabled) VALUES ('probe', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('11', 'Path to the rrdtool executable', 'string', 'rrdtool', 'rrdtool');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('11', 'Path to the RRD database', 'string', 'RRD_repository', '/var/lib/Probe');


/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('12', 'Server hostname', 'string', 'server', '');


/* module: bookmark */
CREATE TABLE daportal_bookmark (
	bookmark_id INTEGER NOT NULL UNIQUE REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	url VARCHAR(256)
);

INSERT INTO daportal_module (name, enabled) VALUES ('bookmark', '1');


/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id SERIAL PRIMARY KEY,
	category_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE CASCADE,
	content_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE CASCADE
);

INSERT INTO daportal_module (name, enabled) VALUES ('category', '1');


/* module: download */
CREATE TABLE daportal_download (
	download_id SERIAL PRIMARY KEY,
	content_id INTEGER NOT NULL REFERENCES daportal_content (content_id) ON DELETE RESTRICT, -- not UNIQUE allows hard links
	parent INTEGER REFERENCES daportal_download (download_id) ON DELETE RESTRICT,
	mode SMALLINT DEFAULT '0420'
);

INSERT INTO daportal_module (name, enabled) VALUES ('download', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('15', 'Path to the download repository', 'string', 'root', '/tmp');


/* module: article */
INSERT INTO daportal_module (name, enabled) VALUES ('article', '1');


/* module: wiki */
INSERT INTO daportal_module (name, enabled) VALUES ('wiki', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('17', 'Allow anonymous users on the wiki', 'bool', 'anonymous', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('17', 'Path to the wiki repository', 'string', 'root', '');
INSERT INTO daportal_config (module_id, title, type, name, value_bool) VALUES ('17', 'Let wiki pages be tagged', 'bool', 'tags', '0');


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

INSERT INTO daportal_module (name, enabled) VALUES ('pki', '1');
INSERT INTO daportal_config (module_id, title, type, name, value_string) VALUES ('18', 'Path to the PKI repository', 'string', 'root', '');


/* module: blog */
INSERT INTO daportal_module (name, enabled) VALUES ('blog', '1');
