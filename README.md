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

## API

### `lib_mysqludf_amqp_info()`

Returns an informational message denoting the package name and version. For example, `lib_mysqludf_amqp 0.0.0`.

#### Example

```
SELECT lib_mysqludf_amqp_info();
```

### `lib_mysqludf_amqp_sendstring(hostname, port, username, password, exchange, routingKey, message)`

Sends a `message` to the given `exchange` on the provided `hostname` and `port` with the supplied `routingKey` as `username` identified by `password`.

#### Example

Login as user `guest` with password `guest` to the AMQP server running on `localhost` port `5672` and publush the message `Hello, World!` with routing key `test` to exchange `udf`:

```
SELECT lib_mysqludf_amqp_sendstring('localhost', 5672, 'guest', 'guest', 'udf', 'test', 'Hello, World!');
```

## License

See [COPYING](https://github.com/ssimicro/lib_mysqludf_amqp/blob/master/COPYING).
