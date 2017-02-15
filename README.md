# lib\_mysqludf\_amqp

`lib_mysqludf_amqp` is a [MySQL](http://www.mysql.com/) [user-defined function](http://dev.mysql.com/doc/refman/5.7/en/udf-features.html)
library for sending [AMQP](https://www.amqp.org/) messages.

## Requirements

* C compiler and standard build tools (make, sh, ...).
* [MySQL](http://www.mysql.com/)
* [rabbitmq-c](https://github.com/alanxz/rabbitmq-c)
* [libbsd](https://libbsd.freedesktop.org/) (on GNU/Linux)

## Building

These are the instructions for building [releases](https://github.com/ssimicro/lib_mysqludf_amqp/releases). If you are building from non-release source (e.g. `git clone`) and there is no `./configure` script, run `./autogen` first and then follow the instructions below..

```
$ ./configure
$ make
# sudo make install
# service mysql-server restart
$ make installdb
```

## Example

### Hello, World!

Publishes a string `'Hello, World!'` to the `udf` exchange on `localhost:5672` with a routing key of `test` as the user `guest` with the password `guest`. Upon success, the message ID is returned.

```
mysql> SELECT lib_mysqludf_amqp_sendstring('amqp://guest:guest@localhost:5672', 'udf', 'test', 'Hello, World!');
+---------------------------------------------------------------------------------------------------+
| lib_mysqludf_amqp_sendstring('amqp://guest:guest@localhost:5672', 'udf', 'test', 'Hello, World!') |
+---------------------------------------------------------------------------------------------------+
| 000e9ced-9050-4454-8eb4-afe78c336b13                                                              |
+---------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

### Table Watcher

The following publishes JSON objects representing table rows whenever a row is inserted, updated, or deleted.
```
SET @AMQP_URL = 'amqp://guest:guest@localhost:5672';
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
    SET @message_id = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_URL, @AMQP_EXCHANGE, 'accounts.insert', json_object('id', NEW.id, 'username', NEW.username)));
END ;;

DROP TRIGGER IF EXISTS `after_update_on_accounts`;
CREATE DEFINER=`root`@`localhost` TRIGGER `after_update_on_accounts` AFTER UPDATE ON `accounts` FOR EACH ROW BEGIN
    SET @message_id = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_URL, @AMQP_EXCHANGE, 'accounts.update', json_object('id', NEW.id, 'username', NEW.username)));
END ;;

DROP TRIGGER IF EXISTS `after_delete_on_accounts`;
CREATE DEFINER=`root`@`localhost` TRIGGER `after_delete_on_accounts` AFTER DELETE ON `accounts` FOR EACH ROW BEGIN
    SET @message_id = (SELECT lib_mysqludf_amqp_sendjson(@AMQP_URL, @AMQP_EXCHANGE, 'accounts.delete', json_object('id', OLD.id, 'username', OLD.username)));
END ;;

DELIMITER ;

INSERT INTO accounts (username) values ('jdoe');
UPDATE accounts SET username = 'jsmith';
DELETE FROM accounts WHERE id = last_insert_id();
```

## API

### `lib_mysqludf_amqp_info()`

Returns an informational message denoting the package name and version.

#### Parameters

None. Supplying any parameters will result in an error.

#### Returns

A string representation of the package name and version, separated by a single space. For example, `lib_mysqludf_amqp 2.0.0`.

#### Errors

* **invalid arguments** Raised when the function is called with arguments.

#### Example

```
SELECT lib_mysqludf_amqp_info();
```

### `lib_mysqludf_amqp_sendstring(url, exchange, routingKey, message)`

Sends a plain text `message` to the given `exchange` on the provided `hostname` and `port` with the supplied `routingKey` as `username` identified by `password`.

#### Parameters

* `url` (string). `amqp://[$USERNAME[:$PASSWORD]@]$HOST[:$PORT]/[$VHOST]`
* `exchange` (string). The name of the AMQP exchange to publish the message to.
* `routingKey` (string). The routing key for this message.
* `message` (string). The body of the message.

#### Returns

Upon succes, this function returns a string containing the message ID, a [Universally Unique IDentifier (UUID)](https://tools.ietf.org/html/rfc4122) (v4).

Upon failure, either `NULL` is returned or an error is raised.

#### Errors

* **invalid arguments** Raised when the function is called with an invalid number of arguments or when any argument is not of the correct type.
* **socket error** Raised when a socket cannot be allocated.
* **socket open error** Raised when a socket cannot be opened.
* **login error** Raised when authentication against the AMQP server specified by `hostname` and `port` fails using the supplied credentials, `username` and `password`.
* **channel error** Raised when a communications channel cannot be opened between this module and the AMQP server.
* **malloc error** Raised when the memory allocation routine `malloc()` fails.

#### Example

Login as user `guest` with password `guest` to the AMQP server running on `localhost` port `5672` and publish the message `Hello, World!` with routing key `test` to exchange `udf`:

```
SELECT lib_mysqludf_amqp_sendstring('amqp://guest:guest@localhost:5672', 'udf', 'test', 'Hello, World!');
```

### `lib_mysqludf_amqp_sendjson(url, exchange, routingKey, message)`

Sends a JSON `message` to the given `exchange` on the provided `hostname` and `port` with the supplied `routingKey` as `username` identified by `password`.

#### Parameters

* `url` (string). `amqp://[$USERNAME[:$PASSWORD]@]$HOST[:$PORT]/[$VHOST]`
* `exchange` (string). The name of the AMQP exchange to publish the message to.
* `routingKey` (string). The routing key for this message.
* `message` (string). The body of the message, a JSON string.

#### Returns

Upon succes, this function returns a string containing the message ID, a [Universally Unique IDentifier (UUID)](https://tools.ietf.org/html/rfc4122) (v4).

Upon failure, either `NULL` is returned or an error is raised.

#### Errors

* **invalid arguments** Raised when the function is called with an invalid number of arguments or when any argument is not of the correct type.
* **socket error** Raised when a socket cannot be allocated.
* **socket open error** Raised when a socket cannot be opened.
* **login error** Raised when authentication against the AMQP server specified by `hostname` and `port` fails using the supplied credentials, `username` and `password`.
* **channel error** Raised when a communications channel cannot be opened between this module and the AMQP server.
* **malloc error** Raised when the memory allocation routine `malloc()` fails.

#### Example

Login as user `guest` with password `guest` to the AMQP server running on `localhost` port `5672` and publish the message `{ "info": "lib_mysqludf_amqp 0.0.0" }` with routing key `test` to exchange `udf`:

```
SELECT lib_mysqludf_amqp_sendjson('amqp://guest:guest@localhost:5672', 'udf', 'test', json_object('lib_mysqludf_amqp_info', cast(lib_mysqludf_amqp_info() as char)));
```

## License

See [COPYING](https://github.com/ssimicro/lib_mysqludf_amqp/blob/master/COPYING).
