#include <stdio.h>
#include <stdlib.h>
#include </usr/include/mysql/mysql.h>

int main(int argc, char **argv)
{
	MYSQL msconn;
	MYSQL_RES *res_ptr;
	MYSQL_ROW sqlrow;
	int c,res;

	mysql_init(&msconn);
	
	// syntax is: mysql host user pass dbname
	if (mysql_real_connect(&msconn,argv[1],argv[2],argv[3],argv[4],0,NULL,0))
		printf("Connection success!\n");
	else {
		printf("Connection failed.\n");
		if (mysql_errno(&msconn))
			fprintf(stderr,"Connection error %d: %s\n",mysql_errno(&msconn),mysql_error(&msconn));
	}

	res=mysql_query(&msconn,"SELECT * FROM ham_contacts");
	if (res)
		printf("Select error: %s\n",mysql_error(&msconn));
	else {
		res_ptr=mysql_use_result(&msconn);
		if (res_ptr) {
			printf("Retrieved %lu rows.\n",(unsigned long)mysql_num_rows(res_ptr));
			while ((sqlrow=mysql_fetch_row(res_ptr))) {
				for (c=0; c<mysql_field_count(&msconn); c++)
					printf("%s ",sqlrow[c]);
				printf("\n");
			}
		}
	}

	mysql_close(&msconn);
	return 0;
}
