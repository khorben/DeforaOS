CREATE TABLE daportal_module (
	module_id SERIAL UNIQUE,
	name VARCHAR(255) UNIQUE NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);
INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('menu', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_lang (
	lang_id VARCHAR(2) NOT NULL,
	name VARCHAR(255) NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false,
	PRIMARY KEY (lang_id)
);
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('en', 'English', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('fr', 'Français', '1');
INSERT INTO daportal_lang (lang_id, name, enabled) VALUES ('de', 'Deutsch', '1');


CREATE TABLE daportal_config (
	module_id SERIAL,
	name VARCHAR(255) NOT NULL,
	value VARCHAR(255) NOT NULL,
	FOREIGN KEY (module_id) REFERENCES daportal_module (module_id)
);


CREATE TABLE daportal_user (
	user_id SERIAL UNIQUE,
	username VARCHAR(255) UNIQUE,
	password CHAR(32),
	admin BOOLEAN DEFAULT FALSE,
	email VARCHAR(255) NOT NULL,
	PRIMARY KEY (user_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');
INSERT INTO daportal_user (username, password, admin, email) VALUES ('admin', 'password', '1', 'username@domain.tld');


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
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
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
