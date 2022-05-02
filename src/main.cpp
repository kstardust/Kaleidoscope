#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum Token: int {
  TokenEOF = -1,
  TokenDef = -2,
  TokenExtern = -3,
  TokenIdentifier = -4,
  TokenNumber = -5
};

static std::string IdentifierStr;
static double NumVal;

int get_token() {
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
      return get_token();
  }

  if (LastChar == EOF)
    return TokenEOF;

  int thisChar = LastChar;
  LastChar = getchar();
  return thisChar;
}

void print_token(int token) {
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

int main() {
  int token;
  while ((token = get_token()) != TokenEOF) {
    print_token(token);
  }
}
