#include "InteractionActions.h"
#include <assert.h>
#include <regex>
#include "Common.h"

AssignAction::AssignAction() {
	_actionName = "AssignAction";
	_supportedOperations =
	{
		{ "SET", { 0, "left" } },
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

bool AssignAction::addArgument(std::shared_ptr<OperationComponent> argument, std::string name) {
	_arguments.push_back({ argument, name });
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
			//TODO: move to entity operation section, because it's also operation with 1 argument or maybe merge with entity operation
			if (*word == "SIZE") {
				//in size we don't care about type
				auto operand = intermediate.back();
				intermediate.pop_back();
				auto vectorType = std::get<0>(operand)->getVariableType(std::get<1>(operand));
				if (vectorType == VariableType::varFloatVector) {
					int vectorSize = std::get<0>(std::get<0>(operand)->getMemberVectorFloat(std::get<1>(operand)))->size();
					intermediate.push_back({ nullptr, std::to_string(vectorSize), -1 });
				}
				else if (vectorType == VariableType::varStringVector) {
					int vectorSize = std::get<0>(std::get<0>(operand)->getMemberVectorString(std::get<1>(operand)))->size();
					intermediate.push_back({ nullptr, std::to_string(vectorSize), -1 });
				}
				continue;
			}

			std::tuple<std::shared_ptr<OperationComponent>, std::string, int> operandTuple[2];
			VariableType operandType[2] = { VariableType::varUnknown, VariableType::varUnknown };
			bool operandConst[2] = { false, false };

			for (int i = 0; i < 2; i++) {
				operandTuple[i] = intermediate.back();
				intermediate.pop_back();
				if (std::get<0>(operandTuple[i]) == nullptr) {
					operandConst[i] = true;
					if (isNumber(std::get<1>(operandTuple[i])))
						operandType[i] = VariableType::varFloat;
					else
						operandType[i] = VariableType::varString;
				}
				else {
					operandType[i] = std::get<0>(operandTuple[i])->getVariableType(std::get<1>(operandTuple[i]));
				}
			}

			std::tuple<float, bool> operandFloat[2] = { {0, false}, {0, false} };
			std::tuple<std::string, bool> operandString[2] = { {"", false}, {"", false} };
			for (int i = 0; i < 2; i++) {
				if (operandConst[i]) {
					if (operandType[i] == VariableType::varFloat)
						operandFloat[i] = { stof(std::get<1>(operandTuple[i])), true };
					else if (operandType[i] == VariableType::varString)
						operandString[i] = { std::get<1>(operandTuple[i]), true };
				}
				else if (operandType[i] == VariableType::varFloatVector) {
					auto operandValue = std::get<0>(operandTuple[i])->getMemberVectorFloat(std::get<1>(operandTuple[i]));
					if (std::get<1>(operandValue))
						operandFloat[i] = { std::get<0>(operandValue)->at(std::get<2>(operandTuple[i])), true };
					else
						return true;
				}
				else if (operandType[i] == VariableType::varStringVector) {
					auto operandValue = std::get<0>(operandTuple[i])->getMemberVectorString(std::get<1>(operandTuple[i]));
					if (std::get<1>(operandValue)) {
						int index = std::get<2>(operandTuple[i]);
						if (index >= 0)
							operandString[i] = { std::get<0>(operandValue)->at(index), true };
					}
					else
						return true;
				}
				else if (operandType[i] == VariableType::varFloat) {
					auto operandValue = std::get<0>(operandTuple[i])->getMemberFloat(std::get<1>(operandTuple[i]));
					if (std::get<1>(operandValue))
						operandFloat[i] = { *std::get<0>(operandValue), true };
					else
						return true;
				}
				else if (operandType[i] == VariableType::varString) {
					auto operandValue = std::get<0>(operandTuple[i])->getMemberString(std::get<1>(operandTuple[i]));
					if (std::get<1>(operandValue))
						operandString[i] = { *std::get<0>(operandValue), true };
					else
						return true;
				}
			}
			if (*word == "AT") {
				if (std::get<1>(operandFloat[0]))
					//push vector with correct index, index can only be float
					intermediate.push_back({ std::get<0>(operandTuple[1]), std::get<1>(operandTuple[1]), std::get<0>(operandFloat[0]) });
			} else if (*word == "SET") {
				if (std::get<1>(operandFloat[0]))
					std::get<0>(operandTuple[1])->setMember(std::get<1>(operandTuple[1]), std::get<0>(operandFloat[0]), std::get<2>(operandTuple[1]));
				else if (std::get<1>(operandString[0])) {
					std::get<0>(operandTuple[1])->setMember(std::get<1>(operandTuple[1]), std::get<0>(operandString[0]), std::get<2>(operandTuple[1]));
				}
			} else {
				if (std::get<1>(operandFloat[0]) && std::get<1>(operandFloat[1])) {
					float operand[2] = { std::get<0>(operandFloat[0]), std::get<0>(operandFloat[1]) };
					_expression->arithmeticOperationFloat(intermediate, operand, *word);
				} else if (std::get<1>(operandString[0]) && std::get<1>(operandString[1])) {
					std::string operand[2] = { std::get<0>(operandString[0]), std::get<0>(operandString[1]) };
					_expression->arithmeticOperationString(intermediate, operand, *word);
				}
			}

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
				std::shared_ptr<OperationComponent> object = std::get<0>(_arguments[atoi(varIndex.c_str())]);
				std::string varName = std::get<1>(_arguments[atoi(varIndex.c_str())]);
				intermediate.push_back({ object, varName, -1 });
			}
			else {
				intermediate.push_back({ nullptr, *word, -1 });
			}
		}
	}

	return false;
}