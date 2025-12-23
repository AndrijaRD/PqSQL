#include "./PqSQL.h"




int countParameters(const std::string& command) {
    int maxParam = 0;
    const char* ptr = command.c_str();
    
    while (*ptr) {
        if (*ptr == '$' && std::isdigit(static_cast<unsigned char>(ptr[1]))) {
            // Parse the number directly
            int num = 0;
            ptr++;  // Skip '$'
            
            while (std::isdigit(static_cast<unsigned char>(*ptr))) {
                num = num * 10 + (*ptr - '0');
                ptr++;
            }
            
            if (num > maxParam) maxParam = num;
        } else {
            ptr++;
        }
    }
    
    return maxParam;
}

int getSQLCommandType(const std::string& sql) {
    // Skip whitespace
    size_t i = 0;
    while (i < sql.size() && std::isspace(sql[i])) i++;
    
    if (i >= sql.size()) return -1;
    
    // Check the first character (uppercase)
    char c = sql[i] & ~0x20; // Quick uppercase (only works for A-Z)
    
    // Check command types based on first character
    switch (c) {
        case 'S': return SQL_SELECT;   // SELECT
        case 'I': return SQL_INSERT;   // INSERT
        case 'U': return SQL_UPDATE;   // UPDATE
        case 'D': {
            // Could be DELETE or DROP, check second character
            if (i + 1 < sql.size()) {
                char c2 = sql[i + 1] & ~0x20;
                if (c2 == 'E') return SQL_DELETE;  // DELETE
                if (c2 == 'R') return SQL_DROP;    // DROP
            }
            break;
        }
        case 'C': return SQL_CREATE;   // CREATE
        case 'A': return SQL_ALTER;    // ALTER
        case 'T': return SQL_TRUNCATE; // TRUNCATE
        case 'W': return SQL_SELECT;   // WITH (CTE)
    }
    
    return -1; // Unknown command type
}



int DB::init(
    const string& db_name,
    const string& username,
    const string& password,
    const string& hostAddr,
    const int& port
){
    dbName = db_name;
    dbUser = username;
    dbPass = password;
    dbAddr = hostAddr;
    dbPort = port;
    
    string connInfo = "dbname=" + dbName +
                      " user=" + dbUser +
                      " password=" + dbPass + 
                      " hostaddr=" + dbAddr +
                      " port=" + to_string(port);
    
    dbConn = PQconnectdb(connInfo.c_str());
    if (PQstatus(dbConn) != CONNECTION_OK) {
        PQfinish(dbConn);
	    cout << "connInfo: " << connInfo << endl;
        return 1;
    }

    return 0;
}




int DB::prepareStatement(string& command){
    if (!dbConn) {
        cerr << "Database not initialized. Call DB::init() first." << endl;
        return -1;
    }
    
    if (command.empty()) {
        cerr << "Empty command provided." << endl;
        return -1;
    }

    // CHECK IF COMMAND HAS ALREADY BEEN PREPARED =======================================
    for (const auto& [id, stmt] : preparedStatements) {
        if (stmt.command == command) {
            return id;  // Return existing ID
        }
    }

    // This now becomes previous statement
    previousStatementId++;
    int stmtID = previousStatementId; // To stop the confusion, using stmtID insted of previousStatementId
    

    // FILL OUT THE STATEMENT
    Statement newStmt;
    newStmt.command = command;
    newStmt.nParams = countParameters(command);
    newStmt.type = getSQLCommandType(command);
    newStmt.name = "stmt_" + stmtID;

    if (newStmt.type == -1) {
        cerr << "Unknown or unsupported SQL command type." << endl;
        return -1;
    }


    PGresult* res = PQprepare(
        dbConn, 
        newStmt.name.c_str(), 
        newStmt.command.c_str(), 
        newStmt.nParams, 
        nullptr
    );
    
    if(!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        if(res) PQclear(res);
        previousStatementId--;
        return -1;
    }

    newStmt.prepared = true;
    preparedStatements[stmtID] = newStmt;

    PQclear(res);
    return stmtID;
}



/** Check Result
 * 
 * Checks if result is fine. RETURNS TRUE.
 * 
 * @returns True if everything is fine and False if there is problem
 */
bool DB::checkResult(const PGresult* res, const int type){
    if (res == nullptr) {
        return false; // Null result is always an error
    }

    ExecStatusType status = PQresultStatus(res);

    switch (type)
    {
        case SQL_SELECT:
            return status == PGRES_TUPLES_OK;

        case SQL_INSERT:
        case SQL_UPDATE:
        case SQL_DELETE:
        case SQL_TRUNCATE:
            return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK;

        case SQL_CREATE:
        case SQL_ALTER:
        case SQL_DROP:
            return status == PGRES_COMMAND_OK;

        default:
            return false; // Unknown query type
    }

    
}




int DB::exec(int ID, const vector<string>& params, DBResult& result){
    if (!dbConn) {
        cerr << "Database not initialized." << endl;
        return 1;
    }
    
    auto it = preparedStatements.find(ID);
    if (it == preparedStatements.end()) {
        cerr << "Statement ID " << ID << " not found." << endl;
        return 2;
    }
    
    Statement& stmt = it->second;
    if (!stmt.prepared) {
        cerr << "Statement " << ID << " is not prepared." << endl;
        return 3;
    }
    
    // Check parameter count
    if (static_cast<int>(params.size()) != stmt.nParams) {
        cerr << "Expected " << stmt.nParams << " parameters, got " << params.size() << endl;
        return 4;
    }

    // Free up the result variable
    if (result.result) {
        PQclear(result.result);
        result.result = nullptr;
    }

    //const char* formatedParams[s.nParams];
    std::vector<const char*> formatedParams;
    formatedParams.reserve(stmt.nParams);  // Preallocate memory to avoid reallocation


    for (const auto& param : params) {
        formatedParams.push_back(param.c_str());
    }

    result.result = PQexecPrepared(
        dbConn, 
        stmt.name.c_str(), 
        stmt.nParams, 
        formatedParams.data(), 
        nullptr, 
        nullptr, 
        0
    );

    // Check for errors
    if (!checkResult(result.result, stmt.type)) {
        string errMsg = PQerrorMessage(dbConn);
        cerr << "Execution error: " << errMsg << endl;
        if (result.result) {
            cerr << "Result status: " << PQresStatus(PQresultStatus(result.result)) << endl;
        }

        PQclear(result.result);
        result.result = nullptr;

        return 5;
    }
    
    return 0;
}


