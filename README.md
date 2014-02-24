GQTPクライアント
======================
Groonga Query Transfer Protocol support.

利用方法
------

### Groongaライブラリのインストール ###
    
    $ sudo aptitude install -y libgroonga0 libgroonga-dev

### groonga-php-driverのインストール ###
    
    $ git clone https://github.com/Yujiro3/groonga-php-driver.git
    $ cd ./groonga-php-driver
    $ phpize
    $ ./configure
    $ make
    $ sudo -s
    # make install
    # cd /etc/php5/mods-available
    # echo extension=groonga.so > gqtpcli.ini
    # cd /etc/php5/conf.d
    # ln -s ../mods-available/gqtpcli.ini ./20-gqtpcli.ini
    
### 接続 ###

```php
<?php
/* オブジェクト生成 */
$grn = new GQTPcli();

/* サーバ接続 */
$grn->connect('localhost', 10043);
```

### テーブルの作成 ###

```php
$result = $grn->query('table_create --name Site --flags TABLE_HASH_KEY --key_type ShortText', 0);
if (!$result) {
    $error = $grn->getErrorMessage();
    var_export($error);
}
```

### カラムの作成 ###

```php
$result = $grn->query('column_create --table Site --name title --type ShortText');
if (!$result) {
    $error = $grn->getErrorMessage();
    var_export($error);
}
```

### データのロード ###

```php
$grn->query("load --table Site --values [");
$grn->query('{"_key":"http://example.org/","title":"This is test record 1!"}');
$grn->query('{"_key":"http://example.net/","title":"test record 2."}');
$grn->query('{"_key":"http://example.com/","title":"test test record three."}');
$grn->query('{"_key":"http://example.net/afr","title":"test record four."}');
$grn->query('{"_key":"http://example.org/aba","title":"test test test record five."}');
$grn->query('{"_key":"http://example.com/rab","title":"test test test test record six."}');
$grn->query('{"_key":"http://example.net/atv","title":"test test test record seven."}');
$grn->query('{"_key":"http://example.org/gat","title":"test test record eight."}');
$grn->query('{"_key":"http://example.com/vdw","title":"test test record nine."}');
$grn->query("]");
```

### データ一覧の取得 ###

```php
$result = $grn->query('select --table Site');
print_r(json_decode($result, true));
```

### 出力結果 ###

```    
Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [0] => 9
                )

            [1] => Array
                (
                    [0] => Array
                        (
                            [0] => _id
                            [1] => UInt32
                        )

                    [1] => Array
                        (
                            [0] => _key
                            [1] => ShortText
                        )

                    [2] => Array
                        (
                            [0] => title
                            [1] => ShortText
                        )

                )

            [2] => Array
                (
                    [0] => 1
                    [1] => http://example.org/
                    [2] => This is test record 1!
                )

            [3] => Array
                (
                    [0] => 2
                    [1] => http://example.net/
                    [2] => test record 2.
                )

            [4] => Array
                (
                    [0] => 3
                    [1] => http://example.com/
                    [2] => test test record three.
                )

            [5] => Array
                (
                    [0] => 4
                    [1] => http://example.net/afr
                    [2] => test record four.
                )

            [6] => Array
                (
                    [0] => 5
                    [1] => http://example.org/aba
                    [2] => test test test record five.
                )

            [7] => Array
                (
                    [0] => 6
                    [1] => http://example.com/rab
                    [2] => test test test test record six.
                )

            [8] => Array
                (
                    [0] => 7
                    [1] => http://example.net/atv
                    [2] => test test test record seven.
                )

            [9] => Array
                (
                    [0] => 8
                    [1] => http://example.org/gat
                    [2] => test test record eight.
                )

            [10] => Array
                (
                    [0] => 9
                    [1] => http://example.com/vdw
                    [2] => test test record nine.
                )

        )

)
```
    
    

ライセンス
----------
Copyright &copy; 2014 Yujiro Takahashi  
Licensed under the [MIT License][MIT].  
Distributed under the [MIT License][MIT].  

[MIT]: http://www.opensource.org/licenses/mit-license.php
