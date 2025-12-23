#pragma once
#ifndef MySDL_DB
#define MySDL_DB

#include <iostream>             // std
#include <vector>               // vector<>
#include <algorithm>            // For std::remove
#include <libpq-fe.h>           // For PosgreSQL DB
#include <unordered_map>        // For unordered_map<type, type>
#include <string>

using namespace std;




/*

=== === === HOW TO USE === === ===

#include <PqSQL.h>
#include <iostream>

int main() {
    // Initialize connection
    if (DB::init("mydatabase", "postgres", "password") != 0) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }
    
    // Prepare a statement
    std::string sql = "SELECT * FROM users WHERE id = $1";
    int stmtId = DB::prepareStatement(sql);
    
    if (stmtId == -1) {
        std::cerr << "Failed to prepare statement!" << std::endl;
        return 1;
    }
    
    // Execute with parameters
    DBResult result;
    if (DB::exec(stmtId, {"42"}, result) == 0) {
        std::cout << "Query successful!" << std::endl;
        std::cout << "Rows returned: " << result.rowCount() << std::endl;
    }
    
    return 0;
}

COMPILATION FLAGS:
  g++ your_app.cpp -lPqSQL -lpq

Or with pkg-config (recommended):
  g++ your_app.cpp \`pkg-config --cflags --libs PqSQL\`

*/




#define SQL_SELECT      0
#define SQL_INSERT      1
#define SQL_CREATE      2
#define SQL_UPDATE      3
#define SQL_ALTER       4
#define SQL_DROP        5
#define SQL_DELETE      6
#define SQL_TRUNCATE    7


class DBResult {
    friend class DB;
private:
    PGresult* result;

public:
    DBResult(PGresult* res = nullptr) : result(res) {}
    ~DBResult() {
        if (result) PQclear(result);
    }

    bool isValid() const { return result != nullptr; };

    int rowCount() const    { return (isValid() ? PQntuples(result) : -1); };
    int columnCount() const { return (isValid() ? PQnfields(result) : -1); };

    string getValue(int row, int column) const {
        if ( !isValid() || row >= rowCount() || column >= columnCount()) return "";
        return PQgetvalue(result, row, column);
    };

    ExecStatusType status() const    { return PQresultStatus(result); };
    const char* errorMessage() const { return PQresultErrorMessage(result); };
};

inline ostream& operator<<(ostream& os, const DBResult& res){
    os << "Result(";
        os << "Valid: "     << res.isValid();
        os << ", Rows: "    << res.rowCount();
        os << ", Columns: " << res.columnCount();
    os << ")";
    return os;
}




class DB{
    private:
    // DATABASE PARAMETERS ====================================================
    static inline string dbName;
    static inline string dbUser;
    static inline string dbPass;
    static inline string dbAddr;
    static inline int    dbPort;

    // DATABASE CONNECTION ====================================================
    static inline PGconn* dbConn = nullptr;

    // STATEMENT STRUCT =======================================================
    struct Statement {
        string name;
        string command;
        int nParams;
        int type;
        bool prepared;
    
        Statement(
            const string& _name = "",
            const string& _command = "",
            const int _numOfParams = 0,
            const int _commandType = SQL_SELECT
        ):  name(_name), 
            command(_command),
            nParams(_numOfParams),
            type(_commandType), 
            prepared(false) 
        {}
    };

    // STATEMENT DATA =========================================================
    static inline std::unordered_map<int, Statement> preparedStatements;
    static inline int previousStatementId = 0;

    // RESULT CHECKER =========================================================
    static bool checkResult(const PGresult* s, const int type);



    public:

    // EXPOSED READ-ONLY CONNECTION ===========================================
    static inline PGconn* const& conn = dbConn;

    /** INIT
     * 
     * Function called once, at the start of the program, 
     * to initiate the connection to the database.
     * 
     * Requires:
     *  -  DataBase Name, 
     *  - Username
     *  - Password
     *  - [OPTIONAL] Addr and Port
     * 
     * @return Returns 0 for success and 1 for error.
    */
    static int init(
        const string& dbName,
        const string& username = "postgres",
        const string& password = "password",
        const string& hostAddr = "127.0.0.1",
        const int& port = 5432
    );

    
    /** PREPARE STATEMENT
     * 
     * This is called once for each function. 
     * 
     * It prepares the command for fast and efficient executuin.
     * It requires the command as String and it 
     * return the ID of the prepared command.
     * 
     * Command can have params showcased with $n ($1, $2, $3, ...)
     * 
     * Func returns positive non 0 number for correct execution, 
     * representing the statement ID, and for any errors -1
     * 
     * @return ID of prepared statement or -1 on error
     */
    static int prepareStatement(string& command);

    /** EXEC
     * 
     * This function executes the prepared command.
     * 
     * It requires the command ID, params as vector of strings and 
     * DBResult where the results will be put.
     * 
     * @return ERROR (0 means no error, positive num is error)
     */
    static int exec(int ID, const vector<string>& params, DBResult& result);


    // HELPER FUNCTION ========================================================

    /** 
     * Cleanup all database resources 
     * and closes connedtion to db (disconnection)
     */
    static void cleanup() {
        preparedStatements.clear();
        previousStatementId = 0;
        
        if (dbConn) {
            PQfinish(dbConn);
            dbConn = nullptr;
        }
    }
    
    /** Check if a statement ID exists */
    static bool statementExists(int ID) {
        return preparedStatements.find(ID) != preparedStatements.end();
    }
    
    /** Get number of prepared statements */
    static int statementCount() {
        return preparedStatements.size();
    }
};

#endif
// Creator: @AndrijaRD