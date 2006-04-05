CREATE TABLE daportal_module (
	module_id SERIAL UNIQUE,
	name VARCHAR(255) UNIQUE NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);
INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('menu', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_config (
	module_id SERIAL,
	name VARCHAR(255) NOT NULL,
	value VARCHAR(255) NOT NULL,
	PRIMARY KEY (module_id, name),
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id)
);
INSERT INTO daportal_config (module_id, name, value) VALUES ('1', 'lang', 'en');
INSERT INTO daportal_config (module_id, name, value) VALUES ('1', 'title', 'DaPortal');


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) NOT NULL,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false,
	PRIMARY KEY (lang_id)
);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_user (
	user_id SERIAL UNIQUE,
	username VARCHAR(255) UNIQUE,
	"password" CHAR(32),
	enabled BOOLEAN NOT NULL DEFAULT FALSE,
	admin BOOLEAN DEFAULT FALSE,
	email VARCHAR(255) NOT NULL,
	PRIMARY KEY (user_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_user (user_id, username, password, email) VALUES ('0', 'Anonymous', '', '');
INSERT INTO daportal_user (username, password, enabled, admin, email) VALUES ('admin', '5f4dcc3b5aa765d61d8327deb882cf99', '1', '1', 'username@domain.tld');


CREATE TABLE daportal_user_register (
	user_id INTEGER UNIQUE,
	"key" CHAR(32) UNIQUE NOT NULL,
	"timestamp" TIMESTAMP NOT NULL DEFAULT now(),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);


CREATE TABLE daportal_content (
	content_id SERIAL UNIQUE,
	timestamp TIMESTAMP NOT NULL DEFAULT now(),
	module_id INTEGER,
	user_id INTEGER,
	title VARCHAR(255),
	content TEXT,
	enabled BOOLEAN NOT NULL DEFAULT false,
	PRIMARY KEY (content_id),
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('content', '1');


/* module: news */
INSERT INTO daportal_module (name, enabled) VALUES ('news', '1');


/* module: comment */
CREATE TABLE daportal_comment (
	comment_id SERIAL UNIQUE,
	parent SERIAL,
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (parent) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('comment', '1');


/* module: top */
CREATE TABLE daportal_top (
	top_id SERIAL UNIQUE,
	name VARCHAR(255),
	link VARCHAR(255),
	PRIMARY KEY (top_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('top', '1');


/* module: project */
CREATE TABLE daportal_project (
	project_id SERIAL UNIQUE,
	name VARCHAR(255) NOT NULL,
	cvsroot VARCHAR(255) NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
);
CREATE TABLE daportal_project_user (
	project_id serial NOT NULL,
	user_id serial NOT NULL,
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id),
	FOREIGN KEY (user_id) REFERENCES daportal_user (user_id)
);
CREATE TABLE daportal_bug (
	bug_id SERIAL UNIQUE,
	content_id SERIAL UNIQUE,
	project_id SERIAL,
	state varchar(11) CHECK (state IN ('New', 'Assigned', 'Closed', 'Fixed', 'Implemented')) NOT NULL DEFAULT 'New',
	type varchar(13) CHECK (type IN ('Major', 'Minor', 'Functionality', 'Feature')) NOT NULL,
	priority VARCHAR(6) CHECK (priority IN ('Urgent', 'High', 'Medium', 'Low')) NOT NULL DEFAULT 'Medium',
	assigned INTEGER,
	PRIMARY KEY (bug_id),
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (project_id) REFERENCES daportal_project (project_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('project', '1');
INSERT INTO daportal_config (module_id, name, value) VALUES ('9', 'cvsroot', '');


/* module: probe */
CREATE TABLE daportal_probe_host (
	host_id SERIAL UNIQUE,
	FOREIGN KEY (host_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('probe', '1');
INSERT INTO daportal_config (module_id, name, value) VALUES ('10', 'RRD_repository', '/var/lib/Probe');


/* module: webmail */
INSERT INTO daportal_module (name, enabled) VALUES ('webmail', 1);
INSERT INTO daportal_config (module_id, name, value) VALUES ('11', 'server', '');

/* module: bookmark */
CREATE TABLE daportal_bookmark (
	bookmark_id SERIAL UNIQUE,
	url VARCHAR(256),
	FOREIGN KEY (bookmark_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('bookmark', '1');

/* module: category */
CREATE TABLE daportal_category_content (
	category_content_id SERIAL UNIQUE,
	category_id SERIAL,
	content_id SERIAL,
	PRIMARY KEY (category_content_id),
	FOREIGN KEY (category_id) REFERENCES daportal_content (content_id),
	FOREIGN KEY (content_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('category', '1');
