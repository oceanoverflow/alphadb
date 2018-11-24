#include "sql.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include <stdio.h>
#include <string>

bool sql::tokenize(const std::string& sql, std::vector<int64_t>& tokens)
{
    yyscan_t scanner;
    if (yylex_init(&scanner)) {
        fprintf(stderr, "sql parser: error when initializing lexer!\n");
        return false;
    }

    YY_BUFFER_STATE state;
    state = yy_scan_string(sql.c_str(), scanner);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    int16_t token = yylex(&yylval, &yylloc, scanner);
    
    while(token){
        tokens.push_back(token);
        token = yylex(scanner);
    }

    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    
    return true;
}

bool sql::parse(const std::string& sql, parser_result& result)
{
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    if (yylex_init(&scanner)) {
        fprintf(stderr, "sql parser: error when initialzing lexer\n!");
        return false;
    }

    const char* str = sql.c_str();
    state = yy_scan_string(str, scanner);

    // yyparse
    int ret = yyparse(result, scanner);
    bool success = ret == 0;
    result.set_valid(success);

    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    return true;
}