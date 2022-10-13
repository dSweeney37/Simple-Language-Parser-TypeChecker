#include "project4.h"

using namespace std;




int main() {
	Parser input;


	input.ParseProgram();
}



void Parser::CheckForDeclErrors() {
	bool declared = true;
	bool referenced = true;
	string name;


	for (int i = 0; i < declarations.size(); ++i) {
		for (int j = 0; j < declarations[i].size(); ++j) {
			for (int k = j + 1; k < declarations[i].size(); ++k) {
				if (declarations[i][j].name == declarations[i][k].name) { PrintErrorCode(1, declarations[i][j].name); }
			}
			if (declarations[i][j].referenced == false) {
				referenced = false;
				name = declarations[i][j].name;
			}
		}
	}
	for (int i = 0; i < statements.size(); ++i) {
		Statement* s = &statements[i];


		if ((*s).lhs.x == -2) {
			declared = false;
			name = (*s).lhs.name;
		}
		for (int j = 0; j < (*s).expressions.size(); ++j) {
			if ((*s).expressions[j].operand.x == -2) {
				declared = false;
				name = (*s).expressions[j].operand.name;
			}
		}
	}
	if (declared == false) { PrintErrorCode(2, name); }
	if (referenced == false) { PrintErrorCode(3, name); }
}



void Parser::CheckForTypeMismatch() {
	for (int i = 0; i < statements.size(); ++i) {
		Statement* s = &statements[i];
		TokenType RHS = EMPTY;


		RHS = (*s).expressions[(*s).expressions.size() - 1].operand.type;

		if ((*s).expressions.size() > 1) {
			for (int j = (*s).expressions.size() - 1; j > 0; --j) {
				RHS = CheckForTypeMismatchM2(&(*s).expressions[j - 1], RHS, (*s).lineNum);
			}
		}
		CheckForTypeMismatchM1(&(*s).lhs.type, &RHS, (*s).lineNum);
	}
}



void Parser::CheckForTypeMismatchM1(TokenType* lhs, TokenType* rhs, short lineNum) {
	if (*lhs == BOOLEAN && *rhs != BOOLEAN) { PrintTypeMismatch(lineNum, 1); }
	if (*lhs == STRING && *rhs != STRING) { PrintTypeMismatch(lineNum, 1); }
	if (*lhs == INT && *rhs != INT && *rhs != BOOLEAN) { PrintTypeMismatch(lineNum, 2); }
	if (*lhs == REAL && *rhs != INT && *rhs != REAL) { PrintTypeMismatch(lineNum, 3); }
}



TokenType Parser::CheckForTypeMismatchM2(Expression* e, TokenType tt2, short lineNum) {
	TokenType* tt1 = &(*e).operand.type;
	TokenType returnType = EMPTY;


	if ((*e).type == ARITHMETIC_OP ) {
		if (*tt1 != REAL && *tt1 != INT && *tt1 != STRING) { PrintTypeMismatch(lineNum, 4); }
		if (tt2 != REAL && tt2 != INT && tt2 != STRING) { PrintTypeMismatch(lineNum, 4); }
		if (*tt1 == STRING && tt2 != STRING) { PrintTypeMismatch(lineNum, 7); }
		if (tt2 == STRING) {
			if ((*e).sign == MULT && *tt1 != STRING && *tt1 != INT) { PrintTypeMismatch(lineNum, 8); }
			if ((*e).sign != MULT && *tt1 != STRING) { PrintTypeMismatch(lineNum, 8); }
		}

		if (*tt1 == STRING || tt2 == STRING) { returnType = STRING; }
		if (*tt1 == REAL || tt2 == REAL) { returnType = REAL; }
		if ((*e).sign != DIV && *tt1 == INT && tt2 == INT) { returnType = INT; }
		else if ((*e).sign == DIV && *tt1 == INT && tt2 == INT) { returnType = REAL; }
	}
	if ((*e).type == BOOL_OP) {
		if (*tt1 != BOOLEAN || tt2 != BOOLEAN) { PrintTypeMismatch(lineNum, 5); }
		else { returnType = BOOLEAN; }
	}
	if ((*e).type == RELATIONAL_OP) {
		if ((*tt1 == INT || *tt1 == REAL) && tt2 != INT && tt2 != REAL) { PrintTypeMismatch(lineNum, 9); }
		if ((tt2 == INT || tt2 == REAL) && *tt1 != INT && *tt1 != REAL) { PrintTypeMismatch(lineNum, 9); }
		if (*tt1 == STRING && tt2 != STRING) { PrintTypeMismatch(lineNum, 6); }
		if (tt2 == STRING && *tt1 != STRING) { PrintTypeMismatch(lineNum, 6); }
		if (*tt1 == BOOLEAN && tt2 != BOOLEAN) { PrintTypeMismatch(lineNum, 6); }
		if (tt2 == BOOLEAN && *tt1 != BOOLEAN) { PrintTypeMismatch(lineNum, 6); }
		returnType = BOOLEAN;
	}
	else { returnType = *tt1; }

	return returnType;
}



void Parser::CheckForUninitialized() {
	bool quit = false;


	for (int i = 0; i < statements.size(); ++i) {
		vector<Expression>* e = &statements[i].expressions;

		for (int j = 0; j < (*e).size(); ++j) {
			if ((*e)[j].operand.initialized == false) {
				PrintUninitialized((*e)[j].operand.name, statements[i].lineNum);
				quit = true;
			}
		}
	}
	if (quit == true) { exit(1); }
}



// This function gets a token and checks if it is of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated this function is particularly useful to match
// terminals in a right hand side of a rule. Written by Mohsen Zohrevandi
// Note: Borrowed from Project 1
Token Parser::Expect(TokenType expected_type) {
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;
	

	if (*tt != expected_type) { SyntaxError(); }
	return t;
}



void Parser::FindReference(Operand* o, Token* pToken) {
	bool rerun = true;
	int i = x;


	while (rerun == true && i >= 0) {
		int j = 0, max = declarations[i].size();


		while (rerun == true && j < max) {
			if (declarations[i][j].name == (*pToken).lexeme) {
				rerun = false;
				(*o).x = i;
				(*o).y = j;
				(*o).type = declarations[i][j].type;
				declarations[i][j].referenced = true;
			}
			else { ++j; }
		}
		--i;
	}

	if (rerun == true) { (*o).x = -2; }
}



void Parser::ParseProgram() {
	// -> scope


	ParseScope();
	Expect(END_OF_FILE);

	CheckForDeclErrors();
	CheckForTypeMismatch();
	CheckForUninitialized();
	PrintReferences();
}



void Parser::ParseScope() {
	// -> LBRACE & scope_list & RBRACE


	// Resizes the "variables" matrix to "lvl" + 1 number of columns.
	declarations.resize(++x + 1);

	Expect(LBRACE);
	ParseScopeList();
	Expect(RBRACE);
	--x;
}



void Parser::ParseScopeList() {
	// -> scope | scope & scope_list | stmt | stmt & scope_list | var_decl | var_decl & scope_list
	Token p1 = lexer.peek(1);
	Token p2 = lexer.peek(2);


	if (p1.token_type == LBRACE) { ParseScope(); }
	else if (p1.token_type == WHILE || p2.token_type == EQUAL) { ParseStmt(); }
	else if (p2.token_type == COLON || p2.token_type == COMMA) { ParseVarDecl(); }
	else { SyntaxError(); }

	// Checks whether or not to continue parcing this block.
	p1 = lexer.peek(1);

	if (p1.token_type != RBRACE) { ParseScopeList(); }
}



void Parser::ParseVarDecl() {
	// -> id_list & COLON & type_name & SEMICOLON


	ParseIdList();

	// Verifies and consumes "COLON".
	Expect(COLON);
	
	ParseTypeName();
	
	// Verifies and consumes "SEMICOLON".
	Expect(SEMICOLON);
}



void Parser::ParseIdList() {
	// -> ID | ID & COMMA & id_list
	Token t;
	TokenType* tt;


	// Verifies and consumes "ID".
	t = Expect(ID);

	// Adds a new variable to the current scope's list of variables.
	declarations[x].push_back(Declaration(t.lexeme, t.line_no));

	// Checks whether or not to continue parcing this block.
	t = lexer.GetToken();
	tt = &t.token_type;
	if (*tt == COMMA) { ParseIdList(); }
	else { lexer.UngetToken(1); }
}



void Parser::ParseTypeName() {
	// -> REAL | INT | BOOLEAN | STRING
	Token t =  lexer.GetToken();
	TokenType* tt = &t.token_type;
	
	
	if (*tt != REAL && *tt != INT && *tt != BOOLEAN && *tt != STRING) { SyntaxError(); }

	// Appends the "token_type" info to each of the variable(s) that exist on the same line as the type info.
	for (int i = 0; i < declarations[x].size(); ++i) {
		if (declarations[x][i].lineNum == t.line_no) { declarations[x][i].type = *tt; }
	}
}



void Parser::ParseStmtList() {
	// -> stmt | stmt & stmt_list
	Token p;
	TokenType* tt;


	ParseStmt();

	p = lexer.peek(2);
	tt = &p.token_type;
	if (*tt == EQUAL || *tt == LPAREN) { ParseStmtList(); }
}



void Parser::ParseStmt() {
	// -> assign_stmt | while_stmt
	Token p = lexer.peek(2);
	TokenType* tt = &p.token_type;

	
	// Creates a new "Statement" object and pushes it onto the "statements" vector.
	statements.push_back(Statement());
	statements[statements.size() - 1].lineNum = p.line_no;

	if (*tt == EQUAL) { ParseAssignStmt(); }
	else if (*tt == LPAREN) { ParseWhileStmt(); }
	else { SyntaxError(); }
}



void Parser::ParseAssignStmt() {
	// -> ID & EQUAL & expr & SEMICOLON
	Operand* o = &statements[statements.size() - 1].lhs;
	Token t;


	// Verifies and consumes "ID".
	t = Expect(ID);
	FindReference(o, &t);
	(*o).name = t.lexeme;
	(*o).initialized = true;
	if ((*o).x != -2) {
		declarations[(*o).x][(*o).y].initialized = true;
		(*o).type = declarations[(*o).x][(*o).y].type;
	}

	// Verifies and consumes "EQUAL".
	Expect(EQUAL);
	
	ParseExpr();

	// Verifies and consumes "SEMICOLON".
	Expect(SEMICOLON);
}



void Parser::ParseWhileStmt() {
	// -> WHILE & LPAREN & condition & RPAREN & stmt | WHILE & LPAREN & condition & RPAREN & LBRACE & stmt_list & RBRACE
	Token t;
	TokenType* tt;


	// Verifies and consumes "WHILE".
	Expect(WHILE);
	// Verifies and consumes "LPAREN".
	Expect(LPAREN);

	ParseCondition();

	// Verifies and consumes "RPAREN".
	Expect(RPAREN);

	// Checks which block to parse.
	t = lexer.GetToken();
	tt = &t.token_type;
	if (*tt == LBRACE) {
		ParseStmtList();

		// Verifies and consumes "RBRACE".
		Expect(RBRACE);
	}
	else {
		lexer.UngetToken(1);
		ParseStmt();
	}
}



void Parser::ParseExpr() {
	// -> primary | arithmetic_expr | bool_expr
	Statement* s = &statements[statements.size() - 1];
	Token p = lexer.peek(1);
	TokenType* tt = &p.token_type;


	if ((*s).expressions.size() == 0 || (*s).expressions[(*s).expressions.size() - 1].operand.type != EMPTY) {
		(*s).expressions.push_back(Expression());
	}
	
	if (*tt == PLUS || *tt == MINUS || *tt == MULT || *tt == DIV) { ParseArithmeticExpr(); }
	else if (*tt == AND || *tt == OR || *tt == XOR || *tt == GREATER || *tt == GTEQ || *tt == LESS
		|| *tt == NOTEQUAL || *tt == LTEQ || *tt == NOT) { ParseBoolExpr(); }
	else { ParsePrimary(); }
}



void Parser::ParseArithmeticExpr() {
	// -> arithmetic_primary | arithmetic_operator & arithmetic_expr & arithmetic_expr
	Statement* s = &statements[statements.size() - 1];
	Token p = lexer.peek(1);
	TokenType* tt = &p.token_type;


	if (*tt == PLUS || *tt == MINUS || *tt == MULT || *tt == DIV) {
		ParseArithmeticOp();
		ParseArithmeticExpr();
		(*s).expressions.push_back(Expression());
		ParseArithmeticExpr();
	}
	else { ParseArithmeticPrimary(); }
}



void Parser::ParseBoolExpr() {
	// -> bool_primary | NOT & bool_expr | relational_operator & expr & expr | binary_bool_operator & bool_expr & bool_expr
	Statement* s = &statements[statements.size() - 1];
	Token p = lexer.peek(1);
	TokenType* tt = &p.token_type;


	if (*tt == ID || *tt == TRUE || *tt == FALSE) { ParseBoolPrimary(); }
	else if (*tt == NOT) {
		Expect(NOT);
		ParseBoolExpr();
	}
	else if (*tt == GREATER || *tt == GTEQ || *tt == LESS || *tt == NOTEQUAL || *tt == LTEQ) {
		ParseRelationalOp();
		ParseExpr();
		ParseExpr();
	}
	else if (*tt == AND || *tt == OR || *tt == XOR) {
		ParseBinaryBoolOp();
		ParseBoolExpr();
		(*s).expressions.push_back(Expression());
		ParseBoolExpr();
	}
}



void Parser::ParseArithmeticOp() {
	// -> PLUS | MINUS | MULT | DIV
	Statement* s = &statements[statements.size() - 1];
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];


	if (*tt != PLUS && *tt != MINUS && *tt != MULT && *tt != DIV) { SyntaxError(); }
	(*e).sign = *tt;
	(*e).type = ARITHMETIC_OP;
}



void Parser::ParseBinaryBoolOp() {
	// -> AND | OR | XOR
	Statement* s = &statements[statements.size() - 1];
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;


	if (*tt != AND && *tt != OR && *tt != XOR) { SyntaxError(); }
	(*e).sign = *tt;
	(*e).type = BOOL_OP;
}



void Parser::ParseRelationalOp() {
	// -> GREATER | GTEQ | LESS | NOTEQUAL | LTEQ
	Statement* s = &statements[statements.size() - 1];
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;


	if (*tt != GREATER && *tt != GTEQ && *tt != LESS && *tt != NOTEQUAL && *tt != LTEQ) { SyntaxError(); }
	(*e).sign = *tt;
	(*e).type = RELATIONAL_OP;
}



void Parser::ParsePrimary() {
	// -> ID | NUM | REALNUM | STRING_CONSTANT | bool_const
	Statement* s = &statements[statements.size() - 1];
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];
	Operand* o = &(*e).operand;
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;
	

	if (*tt == TRUE || *tt == FALSE) {
		(*o).initialized = true;
		(*o).type = BOOLEAN;
		lexer.UngetToken(1);
		ParseBoolConst();
	}
	else if (*tt != ID && *tt != NUM && *tt != REALNUM && *tt != STRING_CONSTANT) { SyntaxError(); }
	else if (*tt == ID) {
		FindReference(o, &t);

		if ((*o).x != -2) { (*o).initialized = declarations[(*o).x][(*o).y].initialized; }
		(*o).name = t.lexeme;
	}
	else if (*tt == NUM) {
		(*o).initialized = true;
		(*o).type = INT;
	}
	else if (*tt == REALNUM) {
		(*o).initialized = true;
		(*o).type = REAL;
	}
	else if (*tt == STRING_CONSTANT) {
		(*o).initialized = true;
		(*o).type = STRING;
	}
}



void Parser::ParseArithmeticPrimary() {
	// -> ID | NUM | REALNUM | STRING_CONSTANT
	Statement* s = &statements[statements.size() - 1];
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];
	Operand* o = &(*e).operand;


	if (*tt != ID && *tt != NUM && *tt != REALNUM && *tt != STRING_CONSTANT) { SyntaxError(); }
	else if (*tt == ID) {
		FindReference(o, &t);

		if ((*o).x != -2) {
			(*o).initialized = declarations[(*o).x][(*o).y].initialized;
			(*o).type = declarations[(*o).x][(*o).y].type;
		}
		(*o).name = t.lexeme;
	}
	else if (*tt == NUM) {
		(*o).initialized = true;
		(*o).type = INT;
	}
	else if (*tt == REALNUM) {
		(*o).initialized = true;
		(*o).type = REAL;
	}
	else if (*tt == STRING_CONSTANT) {
		(*o).initialized = true;
		(*o).type = STRING;
	}
}



void Parser::ParseBoolPrimary() {
	// -> ID | bool_const
	Statement* s = &statements[statements.size() - 1];
	Expression* e = &(*s).expressions[(*s).expressions.size() - 1];
	Operand* o = &(*e).operand;
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;


	if (*tt == TRUE || *tt == FALSE) {
		(*o).initialized = true;
		(*o).type = BOOLEAN;
		lexer.UngetToken(1);
		ParseBoolConst();
	}
	else if (*tt != ID) { SyntaxError(); }
	else {
		FindReference(o, &t);

		if ((*o).x != -2) { (*o).initialized = declarations[(*o).x][(*o).y].initialized; }
		(*o).name = t.lexeme;
	}
}



void Parser::ParseBoolConst() {
	// -> TRUE | FALSE
	Token t = lexer.GetToken();
	TokenType* tt = &t.token_type;


	if (*tt != TRUE && *tt != FALSE) { SyntaxError(); }
}



void Parser::ParseCondition() {
	// -> boolean_expr
	Statement* s = &statements[statements.size() - 1];

	
	(*s).expressions.push_back(Expression());
	ParseBoolExpr();
}



void Parser::PrintErrorCode(short pCode, string pVar) {
	cout << "ERROR CODE 1." << pCode << " " << pVar << endl;
	exit(1);
}



void Parser::PrintReferences() {
	for (int i = 0; i < statements.size(); ++i) {
		Statement* s = &statements[i];

		if ((*s).lhs.x != -1) {
			cout << (*s).lhs.name << " " << (*s).lineNum << " " << declarations[(*s).lhs.x][(*s).lhs.y].lineNum << endl;
		}

		for (int j = 0; j < (*s).expressions.size(); ++j) {
			if ((*s).expressions[j].operand.x != -1) {
				cout << (*s).expressions[j].operand.name << " " << (*s).lineNum << " "
					<< declarations[(*s).expressions[j].operand.x][(*s).expressions[j].operand.y].lineNum << endl;
			}
		}
	}
}



void Parser::PrintTypeMismatch(short pLine, short pCode) {
	cout << "TYPE MISMATCH " << pLine << " C" << pCode << endl;
	exit(1);
}



void Parser::PrintUninitialized(string pVar, short pLineNum) {
	cout << "UNINITIALIZED " << pVar << " " << pLineNum << endl;
}



void Parser::SyntaxError() {
	cout << "Syntax Error &!#@" << endl;
	exit(1);
}



Declaration::Declaration(string pName, short pLineNum) {
	initialized = false;
	lineNum = pLineNum;
	name = pName;
	referenced = false;
	type = EMPTY;
}



Expression::Expression() {
	operand = Operand();
	sign = EMPTY;
	type = EMPTY;
}



Operand::Operand() {
	initialized = false;
	name = "";
	type = EMPTY;
	x = -1;
	y = -1;
}



Statement::Statement() {
	lhs = Operand();
	lineNum = -1;
}
