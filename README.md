# lib\_mysqludf\_amqp

`lib_mysqludf_amqp` is a [MySQL](http://www.mysql.com/) [user-defined function](http://dev.mysql.com/doc/refman/5.7/en/udf-features.html)
library for sending [AMQP](https://www.amqp.org/) messages.

## Status

The project is currently in its infancy. It should not be used in production until the dust has settled and proper testing is complete.

## Requirements

* C compiler and standard build tools (make, sh, ...).
* Autotools and friends (autoconf, automake, libtool, pkg-config, ...).
* [MySQL](http://www.mysql.com/)
* [rabbitmq-c](https://github.com/alanxz/rabbitmq-c)

## Building

```
$ ./autogen
$ ./make
# sudo make install
# service mysql-server restart
$ make installdb
```

## Example

```
SET @AMQP_HOST = 'localhost';
SET @AMQP_PORT = 5672;
SET @AMQP_USER = 'guest';
SET @AMQP_PASS = 'guest';
SET @AMQP_EXCHANGE = 'udf';

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
    `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
    `username` varchar(64) NOT NULL,
     PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='Customer Accounts';

DELIMITER ;;

DROP TRIGGER IF EXISTS `after_insert_on_accounts`;
CREATE DEFINER=`root`@`localhost` TRIGGER `after_insert_on_accounts` AFTER INSERT ON `accounts` FOR EACH ROW BEGIN
    SET @amqp_result = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_HOST, @AMQP_PORT, @AMQP_USER, @AMQP_PASS, @AMQP_EXCHANGE, 'accounts.insert', json_object('id', NEW.id, 'username', NEW.username)));
END ;;

DROP TRIGGER IF EXISTS `after_update_on_accounts`;
CREATE DEFINER=`root`@`localhost` TRIGGER `after_update_on_accounts` AFTER UPDATE ON `accounts` FOR EACH ROW BEGIN
    SET @amqp_result = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_HOST, @AMQP_PORT, @AMQP_USER, @AMQP_PASS, @AMQP_EXCHANGE, 'accounts.update', json_object('id', NEW.id, 'username', NEW.username)));
END ;;

DROP TRIGGER IF EXISTS `after_delete_on_accounts`;
CREATE DEFINER=`root`@`localhost` TRIGGER `after_delete_on_accounts` AFTER DELETE ON `accounts` FOR EACH ROW BEGIN
    SET @amqp_result = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_HOST, @AMQP_PORT, @AMQP_USER, @AMQP_PASS, @AMQP_EXCHANGE, 'accounts.delete', json_object('id', OLD.id, 'username', OLD.username)));
END ;;

DELIMITER ;

INSERT INTO accounts (username) values ('jdoe');
UPDATE accounts SET username = 'jsmith';
DELETE FROM accounts WHERE id = last_insert_id();
```

## API

### `lib_mysqludf_amqp_info()`

Returns an informational message denoting the package name and version. For example, `lib_mysqludf_amqp 0.0.0`.

#### Example

```
SELECT lib_mysqludf_amqp_info();
```

### `lib_mysqludf_amqp_sendstring(hostname, port, username, password, exchange, routingKey, message)`

Sends a plain text `message` to the given `exchange` on the provided `hostname` and `port` with the supplied `routingKey` as `username` identified by `password`.

#### Example

Login as user `guest` with password `guest` to the AMQP server running on `localhost` port `5672` and publish the message `Hello, World!` with routing key `test` to exchange `udf`:

```
SELECT lib_mysqludf_amqp_sendstring(@AMQP_HOST, @AMQP_PORT, @AMQP_USER, @AMQP_PASS, @AMQP_EXCHANGE, 'test', 'Hello, World!');
```

### `lib_mysqludf_amqp_sendjson(hostname, port, username, password, exchange, routingKey, message)`

Sends a JSON `message` to the given `exchange` on the provided `hostname` and `port` with the supplied `routingKey` as `username` identified by `password`.

#### Example

Login as user `guest` with password `guest` to the AMQP server running on `localhost` port `5672` and publish the message `{ "info": "lib_mysqludf_amqp 0.0.0" }` with routing key `test` to exchange `udf`:

```
SELECT lib_mysqludf_amqp_sendjson(@AMQP_HOST, @AMQP_PORT, @AMQP_USER, @AMQP_PASS, @AMQP_EXCHANGE, 'test', json_object('lib_mysqludf_amqp_info', cast(lib_mysqludf_amqp_info() as char)));
```

## License

See [COPYING](https://github.com/ssimicro/lib_mysqludf_amqp/blob/master/COPYING).
