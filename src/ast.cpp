#include "ast.h"
#include <iostream>
#include <cstdio>

std::unique_ptr<ExprAST> LogError(const char *str) {
    fprintf(stderr, "LogError: %s\n", str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *str) {
    LogError(str);
    return nullptr;
}
