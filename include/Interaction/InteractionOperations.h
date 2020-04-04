#pragma once
#include "Expression.h"
#include "Operation.h"
#include <algorithm>

class ExpressionOperation : public Operation {
private:
	std::shared_ptr<Expression> _expression;

	std::vector<std::string> _postfix;
	std::map<std::string, std::tuple<int, std::string> > _supportedOperations;
	std::vector<std::tuple<std::shared_ptr<Component>, std::string> > _arguments;
public:
	ExpressionOperation();
	bool addArgument(std::shared_ptr<Component> argument, std::string name);
	
	bool initializeOperation(std::string condition);
	bool checkOperation();
};