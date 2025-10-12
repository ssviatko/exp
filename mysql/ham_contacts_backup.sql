USE hamradio;
SELECT * FROM ham_contacts INTO OUTFILE "/home/pplant/mysql/ham_contacts_data.bak"
	FIELDS TERMINATED BY ',' ENCLOSED BY '"'
	LINES TERMINATED BY '\n';
SELECT * FROM rigs INTO OUTFILE "/home/pplant/mysql/rigs_data.bak"
	FIELDS TERMINATED BY ',' ENCLOSED BY '"'
	LINES TERMINATED BY '\n';
