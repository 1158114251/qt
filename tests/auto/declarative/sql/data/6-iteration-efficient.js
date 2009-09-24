var db = openDatabase("QmlTestDB", "", "Test database from Qt autotests", 1000000);
var r=0;

db.transaction(
    function(tx) {
        tx.executeSql('SELECT * FROM Greeting', [],
            function(tx, rs) {
                var r1=""
                rs.rows.forwardOnly = true;
                for(var i=0; rs.rows[i]; ++i) {
                    r1 += rs.rows[i].salutation + ", " + rs.rows[i].salutee + ";"
                }
                if (r1 != "hello, world;hello, world;hello, world;hello, world;")
                    r = "SELECTED DATA WRONG: "+r1;
            },
            function(tx, error) { if (r==0) r="SELECT FAILED: "+error.message }
        );
    },
    function(tx, error) { if (r==0) r="TRANSACTION FAILED: "+error.message },
    function(tx, result) { if (r==0) r="passed" }
);


function test()
{
    if (r == 0) r = "transaction_not_finished";
    return r;
}
