DROP DATABASE IF EXISTS hamradio;
CREATE DATABASE hamradio;
USE hamradio;
DROP TABLE IF EXISTS rigs;
CREATE TABLE rigs (
	rig_id int NOT NULL AUTO_INCREMENT,
	make varchar(15),
	model varchar(15),
	PRIMARY KEY (rig_id)
);
DROP TABLE IF EXISTS ham_contacts;
CREATE TABLE ham_contacts (
	id int NOT NULL AUTO_INCREMENT,
	callsign varchar(10),
	first_name varchar(20),
	last_name varchar(20),
	rig_id int NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY (rig_id) REFERENCES rigs(rig_id)
);
LOAD DATA LOCAL INFILE 'rigs_data'
	INTO TABLE rigs
	FIELDS
		TERMINATED BY ','
		ENCLOSED BY '"'
	LINES
		TERMINATED BY '\n';
LOAD DATA LOCAL INFILE 'ham_contacts_data'
	INTO TABLE ham_contacts
	FIELDS
		TERMINATED BY ','
		ENCLOSED BY '"'
	LINES
		TERMINATED BY '\n';
