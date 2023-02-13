// BismIllah
#include <map>
#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <stack>
#include <sstream>
#include <limits>
#include <iomanip>
using namespace std;

// source https://stackoverflow.com/a/16606128/16141581
template <typename T>
inline std::string to_string_with_precision(const T a_value, const int n = 32)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

#define M(N) ((unsigned long long int)1) << N
enum class TokenType : unsigned long long int {

	NUMBER = M(0),

	OP_OPERAND = M(1),

	OP_PLUS = M(2) | OP_OPERAND,
	OP_MINUS = M(3) | OP_OPERAND,
	OP_MUL = M(4) | OP_OPERAND,
	OP_DIV = M(5) | OP_OPERAND,
	OP_POW = M(6) | OP_OPERAND,

	OP_FUNCS = M(7),

	OP_SIN = M(8) | OP_FUNCS,
	OP_COS = M(9) | OP_FUNCS,
	OP_EXP = M(10) | OP_FUNCS,
	OP_LOGN = M(13) | OP_FUNCS,
	OP_SQRT = M(14) | OP_FUNCS,



	SYM_BRACE_O = M(11),
	SYM_BRACE_C = M(12)
};

inline TokenType operator|(TokenType a, TokenType b)
{
	return static_cast<TokenType>(static_cast<unsigned long long int>(a) | static_cast<unsigned long long int>(b));
}
inline TokenType operator&(TokenType a, TokenType b)
{
	return static_cast<TokenType>(static_cast<unsigned long long int>(a) & static_cast<unsigned long long int>(b));
}
struct Token {
	string raw;
	TokenType type;
	int priority;
	int sign;
};

map<string, TokenType> _MAPPING = {
	{"+", TokenType::OP_PLUS},
	{"-", TokenType::OP_MINUS},
	{"/", TokenType::OP_DIV},
	{"*", TokenType::OP_MUL},
	{"^", TokenType::OP_POW},

	{"sin", TokenType::OP_SIN},
	{"cos", TokenType::OP_COS},
	{"exp", TokenType::OP_EXP},
	{"ln", TokenType::OP_LOGN},
	{"sqrt", TokenType::OP_SQRT},

	{"(", TokenType::SYM_BRACE_O},
	{")", TokenType::SYM_BRACE_C},
};
map<TokenType, int> _PRIORITIES = {
	{TokenType::OP_PLUS, 0},
	{TokenType::OP_MINUS, 0},

	{TokenType::OP_MUL, 1},
	{TokenType::OP_DIV, 1},
	{TokenType::OP_SIN, 2},
	{TokenType::OP_COS, 2},
	{TokenType::OP_EXP, 2},
	{TokenType::OP_LOGN, 2},
	{TokenType::OP_SQRT, 2},

	{TokenType::OP_POW, 2},

	{TokenType::SYM_BRACE_O, INT_MIN},
	{TokenType::SYM_BRACE_C, INT_MIN}
};
unordered_set<TokenType> _SKIP = {
	{TokenType::SYM_BRACE_O},
	{TokenType::SYM_BRACE_C},
	{TokenType::NUMBER},
};

#define CONTAINS_C(SET, C) SET.find(string{C}) != SET.end()
#define CONTAINS_S(SET, S) SET.find(S) != SET.end()

void addToken(vector<Token>* tokens, string& totalAccumulator, string& charAccumulator, string& numberAccumulator) {
	if (totalAccumulator.length() > 0) {
		if (CONTAINS_S(_MAPPING, totalAccumulator)) {
			tokens->push_back({ totalAccumulator, _MAPPING[totalAccumulator], _PRIORITIES[_MAPPING[totalAccumulator]], 1 });

			charAccumulator = "";
			numberAccumulator = "";
		}
	}

	if (charAccumulator.length() > 0) {
		if (CONTAINS_S(_MAPPING, charAccumulator)) {
			tokens->push_back({ charAccumulator, _MAPPING[charAccumulator], _PRIORITIES[_MAPPING[charAccumulator]], 1 });
			charAccumulator = "";
		}
	}

	if (numberAccumulator.length() > 0
		&& charAccumulator.length() == 0
		&& numberAccumulator.length() == totalAccumulator.length()) {

		tokens->push_back({ numberAccumulator, TokenType::NUMBER, -1, 1 });
		numberAccumulator = "";
	}
	else if (numberAccumulator.length() > 0
		&& charAccumulator.length() == 0
		&& numberAccumulator.length() < totalAccumulator.length()) {
		tokens->push_back({ totalAccumulator, TokenType::NUMBER, -1, 1 });
		numberAccumulator = "";
	}
}
vector<Token>* tokenize(string expression) {

	vector<Token>* tokens = new vector<Token>();

	string totalAccumulator = "";
	string numberAccumulator = "";
	string charAccumulator = "";

	for (auto c : expression) {

		if (c == ' ' || c == '\r' || c == '\t') continue;

		if (CONTAINS_C(_MAPPING, c)) {
			addToken(tokens, totalAccumulator, charAccumulator, numberAccumulator);
			tokens->push_back({ string{c}, _MAPPING[string{c}], _PRIORITIES[_MAPPING[string{c}]], 1 });
			totalAccumulator = "";

			continue;

		}
		else if (isdigit(c))
			numberAccumulator += c;
		else if (isalpha(c)) charAccumulator += c;
		totalAccumulator += c;
	}

	addToken(tokens, totalAccumulator, charAccumulator, numberAccumulator);

	return tokens;
}

Token clc_func(Token a, Token f) {
	long double a_f = a.sign * stod(a.raw);

	Token token;
	token.type = TokenType::NUMBER;
	token.priority = -5;
	token.sign = 1;

	switch (f.type) {
	case TokenType::OP_SIN:
		token.raw = to_string_with_precision(sin(a_f));
		break;
	case TokenType::OP_COS:
		token.raw = to_string_with_precision(cos(a_f));
		break;
	case TokenType::OP_EXP:
		token.raw = to_string_with_precision(exp(a_f));
		break;
	case TokenType::OP_LOGN:
		token.raw = to_string_with_precision(log(a_f));
		break;
	case TokenType::OP_SQRT:
		token.raw = to_string_with_precision(sqrtl(a_f));
		break;
	}


	return token;
}
Token clc_syms(Token a, Token b, Token operand) {

	long double a_f = a.sign * stod(a.raw);
	long double b_f = b.sign * stod(b.raw);

	Token token;
	token.type = TokenType::NUMBER;
	token.priority = -5;
	token.sign = 1;


	switch (operand.type) {

	case TokenType::OP_PLUS:
		token.raw = to_string_with_precision(a_f + b_f);
		break;
	case TokenType::OP_MINUS:
		token.raw = to_string_with_precision(a_f - b_f);
		break;
	case TokenType::OP_MUL:
		token.raw = to_string_with_precision(a_f * b_f);
		break;
	case TokenType::OP_DIV:
		token.raw = to_string_with_precision(a_f / b_f);
		break;
	case TokenType::OP_POW:
		token.raw = to_string_with_precision(pow(a_f, b_f));
		break;
	}

	return token;
}
Token handle_high_op(stack<Token>& t_stack) {
	Token b = t_stack.top();
	t_stack.pop();

	if (t_stack.size() == 0)
		return b;


	Token op = t_stack.top();
	t_stack.pop();

	if ((op.type & TokenType::OP_FUNCS) == TokenType::OP_FUNCS) {
		return clc_func(b, op);
	}
	else if (op.type == TokenType::SYM_BRACE_O) {
		t_stack.push(op);
		return b;
	}
	else if (op.type == TokenType::OP_MINUS && t_stack.top().type == TokenType::SYM_BRACE_O) {
		b.raw = to_string_with_precision(stod(b.raw) * (-1.00));
		return b;
	}


	Token a = t_stack.top();
	t_stack.pop();

	if (t_stack.size() != 0 && t_stack.top().type == TokenType::OP_MINUS) {
		t_stack.pop();
		t_stack.push({ string{'+'}, _MAPPING[string{'+'}], _PRIORITIES[_MAPPING[string{'+'}]], 1 });

		a.sign = -1;
	}
	return clc_syms(a, b, op);
}

long double eval(vector<Token>* tokens) {

	if (((*tokens->begin()).type & TokenType::OP_OPERAND) == TokenType::OP_OPERAND) {
		// source: https://stackoverflow.com/a/48251347/16141581
		tokens->push_back({ "0", TokenType::NUMBER, -1, 1 });
		std::rotate(tokens->rbegin(), tokens->rbegin() + 1, tokens->rend());
	}

	stack<Token> t_stack;
	int lastPriority = INT_MIN;

	for (auto t : *tokens) {

		if (t.type == TokenType::SYM_BRACE_O)
			lastPriority = INT_MIN;

		if (!_SKIP.count(t.type)) {

			if (t.priority <= lastPriority) {
				t_stack.push(handle_high_op(t_stack));
				lastPriority = t.priority;
			}
			else lastPriority = t.priority;
		}
		else if (t.type == TokenType::SYM_BRACE_C) {
			Token last;
			while (t_stack.size() > 0 && t_stack.top().type != TokenType::SYM_BRACE_O) {
				if (t_stack.top().type == TokenType::NUMBER) {
					last = handle_high_op(t_stack);
					if (t_stack.size() > 0 && (t_stack.top().type & TokenType::OP_OPERAND) == TokenType::OP_OPERAND)
						t_stack.push(last);
				}
			}
			t_stack.pop(); // pop )
			if (t_stack.size() > 0) {
				lastPriority = t_stack.top().priority;
			}
			else lastPriority = INT_MIN;


			if (t_stack.size() > 0 && (t_stack.top().type & TokenType::OP_FUNCS) == TokenType::OP_FUNCS)
			{
				t_stack.push(last);
				t_stack.push(handle_high_op(t_stack));
			}
			else {
				t_stack.push(last);
			}
			continue;
		}

		t_stack.push(t);
	}

	while (t_stack.size() > 1) {
		if (t_stack.top().type == TokenType::NUMBER) {
			t_stack.push(handle_high_op(t_stack));
		}
	}

	delete tokens;

	return (long double)stod(t_stack.top().raw);
}

int main(void) {

	auto tokens = tokenize("2*2*2+10*2");
	cout << setprecision(32) << eval(tokens);

	return 0;
}