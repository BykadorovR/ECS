#include "InteractionActions.h"
#include <assert.h>
#include <regex>
#include "Common.h"

AssignAction::AssignAction() {
	_actionName = "AssignAction";
	_supportedOperations =
	{
		{ "AND", { 0, "left" } },
		{ "SET", { 1, "left" } },
		{ "+",   { 2, "left" } },
		{ "-",   { 2, "left" } },
		{ "/",   { 3, "left" } },
		{ "*",   { 3, "left" } },
		{ "^",   { 4, "right"} },
		{ "AT",  { 5, "left" } },
		{ "SIZE",{ 5, "left" } }
	};
	_expression = std::make_shared<Expression>(_supportedOperations);
}

//TODO: separate common parts from operations and actions
bool AssignAction::addArgument(std::shared_ptr<Entity> entity, std::string component, std::string name) {
	_arguments.push_back({ entity, component, name });
	return false;
}


bool AssignAction::initializeAction(std::string condition) {
	_expression->setCondition(condition);
	return _expression->prepareExpression(_postfix);
}

bool AssignAction::doAction() {
	std::vector<std::tuple<std::shared_ptr<OperationComponent>, std::string, int> > intermediate;
	for (auto word = _postfix.begin(); word < _postfix.end(); word++) {
		//word (token) is operator
		if (_supportedOperations.find(*word) != _supportedOperations.end()) {
			//operations with only 1 argument
			if (intermediate.size() > 0) {
				auto oneArgumentResult = _expression->oneArgumentOperation(intermediate.back(), *word);
				if (std::get<1>(oneArgumentResult)) {
					intermediate.pop_back();
					intermediate.push_back({ nullptr, std::get<0>(oneArgumentResult), -1 });
					continue;
				}
			}
			
			if (intermediate.size() > 1) {
				auto twoArgumentResult = _expression->twoArgumentOperation(intermediate[intermediate.size() - 1], intermediate[intermediate.size() - 2], *word);
				if (std::get<3>(twoArgumentResult)) {
					intermediate.pop_back();
					intermediate.pop_back();
					intermediate.push_back({ std::get<0>(twoArgumentResult), std::get<1>(twoArgumentResult), std::get<2>(twoArgumentResult) });
				}
				else {
					assert("Can't find operation");
				}
			}
			else
				assert("Incorrect implementation");

			//operand[0] - second operand, operand[1] - first operand
			//operandTuple vice versa
			//TODO: IMPL SET FOR ALL CASES!
			//if ${0} AT 2 SET 10
			//if ${0} SET ${1} AT 2
			//if ${0} AT 2 SET ${1} AT 2
		}
		//word (token) is operand
		else {
			// Extraction of a sub-match
			const std::regex varIndexRegex("\\$\\{(\\d+)\\}");
			std::smatch match;
			if (std::regex_search(*word, match, varIndexRegex)) {
				std::string varIndex = match[1].str();
				std::shared_ptr<Entity> entity = std::get<0>(_arguments[atoi(varIndex.c_str())]);
				std::string componentName = std::get<1>(_arguments[atoi(varIndex.c_str())]);
				std::shared_ptr<OperationComponent> component = nullptr;
				if (componentName != "")
					component = std::dynamic_pointer_cast<OperationComponent>(entity->getComponent(componentName));
				std::string varName = std::get<2>(_arguments[atoi(varIndex.c_str())]);
				intermediate.push_back({ component, varName, -1 });
			}
			else {
				intermediate.push_back({ nullptr, *word, -1 });
			}
		}
	}

	return false;
}