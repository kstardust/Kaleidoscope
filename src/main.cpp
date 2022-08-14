#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ast.h"

enum Token : int {
  TokenEOF = -1,
  TokenDef = -2,
  TokenExtern = -3,
  TokenIdentifier = -4,
  TokenNumber = -5
};

static std::string IdentifierStr;
static double NumVal;
static int CurToken;
static std::map<char, int> BinOpPrecedence;

int GetToken();
void PrintToken(int token);
int GetTokenPrecedence();
std::unique_ptr<ExprAST> ParseExpression();
std::unique_ptr<ExprAST> ParseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);

void SetOpPrecedence(char op, int prec) {
    BinOpPrecedence[op] = prec;
}

int GetTokenPrecedence() {
    if (!isascii(CurToken)) {
        return -1;
    }
    int tokenPrec = BinOpPrecedence[CurToken];
    if (tokenPrec < 0)
        return -1;
    return tokenPrec;
}

int GetNextToken() {
    return CurToken = GetToken();
}

int GetToken() {
  static int LastChar = ' ';
  while (isspace(LastChar))
    LastChar = getchar();

  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getchar())) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "def") {
      return TokenDef;
    } else if (IdentifierStr == "extern") {
      return TokenExtern;
    }
    return TokenIdentifier;
  }

  if (isdigit(LastChar) || LastChar == '.') {
    std::string numStr;
    do {
      numStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(numStr.c_str(), 0);
    return TokenNumber;
  }

  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return GetToken();
  }

  if (LastChar == EOF)
    return TokenEOF;

  int thisChar = LastChar;
  LastChar = getchar();
  return thisChar;
}

void PrintToken(int token) {
  switch (token) {
  case TokenDef:
    printf("TokenDef\n");
    break;
  case TokenEOF:
    printf("TokenEOF\n");
    break;
  case TokenExtern:
    printf("TokenExtern\n");
    break;
  case TokenNumber:
    printf("TokenNumber: %f\n", NumVal);
    break;
  case TokenIdentifier:
    printf("TokenIdentifier: %s\n", IdentifierStr.c_str());
    break;
  default:
    printf("Other: %c\n", token);
    break;
  }
  return;
}

std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    GetNextToken();
    return Result;
}

std::unique_ptr<ExprAST> ParseParenExpr() {
    GetNextToken();
    auto V = ParseExpression();
    if (!V) 
        return nullptr;

    if (CurToken != ')')
        return LogError("expected ')'");

    GetNextToken();
    return V;
}

std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string idName = IdentifierStr;
    
    GetNextToken();
    if (CurToken != '(')
        return std::make_unique<VariableExprAST>(idName);

    GetNextToken();
    std::vector<std::unique_ptr<ExprAST>> args;
    
    if (CurToken != ')') {
        while (1) {
            if (auto arg = ParseExpression())
                args.push_back(std::move(arg));
            else
                return nullptr;
            
            if (CurToken == ')')
                break;

            if (CurToken != '.')
                return LogError("expected ')' or ',' in argument list");
            GetNextToken();
        }
    }
    GetNextToken();
    return std::make_unique<CallExprAST>(idName, std::move(args));
}

std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurToken) {
    case TokenIdentifier:
        return ParseIdentifierExpr();
    case TokenNumber:
        return ParseNumberExpr();
    case '(':
        return ParseParenExpr();
    default:
        return LogError("unknown token when expecting an expression");
    }
}

std::unique_ptr<ExprAST> ParseExpression() {
    auto lhs = ParsePrimary();
    if (!lhs)
        return nullptr;
    return ParseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> ParseBinOpRHS(int exprPrec,
                                       std::unique_ptr<ExprAST> lhs) {
    while (1) {
        int tokenPrec = GetTokenPrecedence();

        if (tokenPrec < exprPrec)
            return lhs;

        int binOp = CurToken;
        GetNextToken();

        auto rhs = ParsePrimary();

        if (GetTokenPrecedence() > tokenPrec) {
            rhs = ParseBinOpRHS(tokenPrec + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
    }
    
    return lhs;
}

std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurToken != TokenIdentifier)
        return LogErrorP("Expected function name in prototype");

    std::string funcName = IdentifierStr;
    GetNextToken();

    if (CurToken != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::string> argNames;
    while (GetNextToken() == TokenIdentifier)
        argNames.push_back(IdentifierStr);

    if (CurToken != ')')
        return LogErrorP("Expected ')' in prototype");

    GetNextToken();
    return std::make_unique<PrototypeAST>(funcName, std::move(argNames));
}

std::unique_ptr<FunctionAST> ParseDefinition() {
    GetNextToken();
    auto proto = ParsePrototype();
    if (!proto)
        return nullptr;
    auto e = ParseExpression();
    if (!e)
        return nullptr;
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
}

std::unique_ptr<PrototypeAST> ParseExtern() {
    GetNextToken();
    return ParsePrototype();
}

std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto e = ParseExpression()) {
        auto proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

void HandleTopLevelExpression() {
    if (ParseTopLevelExpr()) {
        printf("Parsed a top-level expr.\n");
    } else {
        GetNextToken();
    }
}

void HandleDefinition() {
    if (ParseDefinition()) {
        printf("Parsed a definition.\n");
    } else {
        GetNextToken();
    }    
}

void HandleExtern() {
    if (ParseExtern()) {
        printf("Parsed a extern.\n");
    } else {
        GetNextToken();
    }    
}

void MainLoop() {
    printf("ready>");
    GetNextToken();
    
    while (1) {
        printf("ready>");
        switch (CurToken) {
        case TokenEOF:
            return;
        case ';':
            GetNextToken();
            break;
        case TokenDef:
            HandleDefinition();
            break;
        case TokenExtern:
            HandleExtern();
            break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}

int main() {
    SetOpPrecedence('<', 10);
    SetOpPrecedence('+', 20);
    SetOpPrecedence('-', 20);
    SetOpPrecedence('*', 40);

    MainLoop();

}
