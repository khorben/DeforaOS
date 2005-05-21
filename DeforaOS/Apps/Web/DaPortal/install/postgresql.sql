CREATE TABLE daportal_module (
	module_id SERIAL UNIQUE,
	name VARCHAR(255) UNIQUE NOT NULL,
	enabled BOOLEAN NOT NULL DEFAULT false
);
INSERT INTO daportal_module (name, enabled) VALUES ('admin', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('explorer', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('menu', '1');
INSERT INTO daportal_module (name, enabled) VALUES ('search', '1');


CREATE TABLE daportal_user (
	user_id SERIAL UNIQUE,
	username VARCHAR(255),
	password CHAR(32),
	admin BOOLEAN DEFAULT FALSE,
	PRIMARY KEY (user_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('user', '1');


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


CREATE TABLE daportal_top (
	top_id SERIAL UNIQUE,
	name VARCHAR(255),
	link VARCHAR(255),
	PRIMARY KEY (top_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('top', '1');


CREATE TABLE daportal_project (
	project_id SERIAL UNIQUE,
	name VARCHAR(255),
	FOREIGN KEY (project_id) REFERENCES daportal_content (content_id)
);
INSERT INTO daportal_module (name, enabled) VALUES ('project', '1');
