#include <iostream>
#include "lexer.h"




class Declaration {
	public:
		bool initialized;
		bool referenced;
		short lineNum;
		std::string name;
		TokenType type;

		Declaration(std::string, short);
};



class Operand {
	public:
		bool initialized;
		short x;
		short y;
		std::string name;
		TokenType type;

		Operand();
};



class Expression {
	public:
		Operand operand;
		TokenType sign;
		TokenType type;

		Expression();
};



class Statement {
	public:
		Operand lhs;
		std::vector<Expression> expressions;
		short lineNum;

		Statement();
};



class Parser {
	public:
		void ParseProgram();


	private:
		short x = -1;
		LexicalAnalyzer lexer;
		std::vector<std::vector<Declaration>> declarations;
		std::vector<Statement> statements;

		void CheckForDeclErrors();
		void CheckForTypeMismatch();
		void CheckForTypeMismatchM1(TokenType*, TokenType*, short);
		TokenType CheckForTypeMismatchM2(Expression*, TokenType, short);
		void CheckForUninitialized();
		Token Expect(TokenType);
		void FindReference(Operand*, Token*);
		void ParseScope();
		void ParseScopeList();
		void ParseVarDecl();
		void ParseIdList();
		void ParseTypeName();
		void ParseStmtList();
		void ParseStmt();
		void ParseAssignStmt();
		void ParseWhileStmt();
		void ParseExpr();
		void ParseArithmeticExpr();
		void ParseBoolExpr();
		void ParseArithmeticOp();
		void ParseBinaryBoolOp();
		void ParseRelationalOp();
		void ParsePrimary();
		void ParseArithmeticPrimary();
		void ParseBoolPrimary();
		void ParseBoolConst();
		void ParseCondition();
		void PrintErrorCode(short, std::string);
		void PrintReferences();
		void PrintTypeMismatch(short, short);
		void PrintUninitialized(std::string, short);
		void SyntaxError();
};
