#include <iostream>

#include <mysql.h>

MYSQL *conn = NULL;

void handle_error()
{
	fprintf(stderr, "error: %s\n", mysql_error(conn));
	mysql_close(conn);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL)
		handle_error();
	printf("initialized.\n");
	if (mysql_real_connect(conn, "localhost", "hamuser", "hamuser", NULL, 0, NULL, 0) == NULL)
//	if (mysql_real_connect(conn, "localhost", "admin", "Miuxt8/Wfd", NULL, 0, NULL, 0) == NULL)
		handle_error();
	printf("connected.\n");
	if (mysql_query(conn, "USE hamradio"))
		handle_error();
	printf("using hamradio\n");
	if (mysql_query(conn, "SELECT * FROM ham_contacts"))
		handle_error();
	printf("selected from ham_contacts\n");
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL)
		handle_error();
	printf("retrieved result set\n");
	int num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		printf("row: ");
 		for (int i = 0; i < num_fields; ++i) {
			printf("%s ", row[i] ? row[i] : "NULL");
		}
		printf("\n");
	}
	mysql_free_result(result);

	mysql_close(conn);

	return 0;
}

