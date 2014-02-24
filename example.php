<?php
/* オブジェクト生成 */
$grn = new GQTPcli();

/* サーバ接続 */
$grn->connect('localhost', 10043);

/* テーブルの作成 */
$result = $grn->query('table_create --name Site --flags TABLE_HASH_KEY --key_type ShortText', 0);
if (!$result) {
    $error = $grn->getErrorMessage();
    var_export($error);
}

/* テーブルの表示 */
$result = $grn->query('select --table Site');
echo "テーブル作成後のテーブル内容\n";
print_r(json_decode($result, true));
echo "\n";


/* カラムの作成 */
$result = $grn->query('column_create --table Site --name title --type ShortText');
if (!$result) {
    $error = $grn->getErrorMessage();
    var_export($error);
}
echo "カラム作成後のテーブル内容\n";
$result = $grn->query('select --table Site');
print_r(json_decode($result, true));
echo "\n";

/* データのロード */
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
$result = $grn->query('select --table Site');
echo "データ一覧\n";
print_r(json_decode($result, true));
echo "\n";

/* レコードの取得 */
$result = $grn->query('select --table Site --query _id:1');
echo "レコードの取得\n";
print_r(json_decode($result, true));
echo "\n";


/* テーブルの一覧 */
$result = $grn->query('table_list');
echo "テーブルの一覧\n";
print_r(json_decode($result, true));
echo "\n";


/* テーブルの削除 */
$result = $grn->query('table_remove --name Site');
if (!$result) {
    $error = $grn->getErrorMessage();
    var_export($error);
}

