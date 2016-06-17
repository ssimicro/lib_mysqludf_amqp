USE mysql;

CREATE FUNCTION lib_mysqludf_amqp_info RETURNS STRING SONAME 'lib_mysqludf_amqp.so';
