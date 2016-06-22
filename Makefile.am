ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = lib_mysqludf_amqp.la
lib_mysqludf_amqp_la_SOURCES = lib_mysqludf_amqp.c
lib_mysqludf_amqp_la_CFLAGS = -DMYSQL_SERVER @MYSQL_CFLAGS@ @RABBITMQ_CFLAGS@
lib_mysqludf_amqp_la_LDFLAGS = -module -avoid-version -no-undefined @MYSQL_LDFLAGS@ @RABBITMQ_LIBS@

installdb: uninstalldb
	mysql -u root -p < ./installdb.sql;

uninstalldb:
	mysql -u root -p < ./uninstalldb.sql

version:
	mysql -u root -p --execute="SELECT lib_mysqludf_amqp_info();"
