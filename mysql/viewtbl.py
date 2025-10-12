#!/usr/bin/python3

import mysql.connector

mysql_conn = mysql.connector.connect(host = "localhost", user = "ssviatko", password = "password")
mysql_cursor = mysql_conn.cursor()
mysql_cursor.execute("USE hamradio;")

mysql_query = "SELECT callsign, first_name FROM ham_contacts;"
mysql_cursor.execute(mysql_query)
for (callsign, first_name) in mysql_cursor:
    print("{} is called {}.".format(callsign, first_name))

mysql_query = "SELECT ham_contacts.callsign, ham_contacts.first_name, rigs.make, rigs.model FROM ham_contacts INNER JOIN rigs ON ham_contacts.rig_id=rigs.rig_id;"
mysql_cursor.execute(mysql_query)
for (callsign, first_name, make, model) in mysql_cursor:
    print("{} who goes by callsign {} uses a {} made by {}.".format(first_name, callsign, model, make))

mysql_cursor.close()
mysql_conn.close()

