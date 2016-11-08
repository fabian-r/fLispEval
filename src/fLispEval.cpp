#include <iostream>
#include <stdio.h>
#include <sstream>
#include <deque>
#include <boost/multiprecision/gmp.hpp>

void require(bool cond, const char* msg)
{
    if (!cond) {
        std::cerr << "ERROR: " << msg << "!" << std::endl;
        exit(1);
    }
}

enum Op { LEQ, LT, GT, GEQ, EQ, ADD, SUB, MUL, DIV, AND, OR, NOT };

struct StackItem
{
    typedef boost::multiprecision::mpq_rational val_t;

    enum Kind { OP, VAL, BV } kind;
    Op op;
    val_t val;
    bool bv;

    StackItem(Op o) : kind(OP), op(o) {}
    StackItem(val_t v) : kind(VAL), val(v) {}
    StackItem(bool b) : kind(BV), bv(b) {}

    friend std::ostream& operator<< (std::ostream& stream, const StackItem& si);
};

std::ostream& operator<< (std::ostream& stream, const StackItem& si)
{
    switch (si.kind) {
        case StackItem::OP:
            stream << "op: " << si.op;
            break;
        case StackItem::VAL:
            stream << si.val;
            break;
        case StackItem::BV:
            stream << (si.bv ? "true" : "false");
            break;
    }
    return stream;
}

#define CHECK_BIN(type)\
            require(operands.size() == 2, "Invalid number of arguments for binary operator");\
            require(operands[0].kind == StackItem::type, "Invalid first argument type");\
            require(operands[1].kind == StackItem::type, "Invalid second argument type");

#define CHECK_UN(type)\
            require(operands.size() == 1, "Invalid number of arguments for unary operator");\
            require(operands[0].kind == StackItem::type, "Invalid argument type");

StackItem eval(Op o, const std::deque<StackItem>& operands)
{
    switch  (o) {
        case LEQ: CHECK_BIN(VAL);
            return StackItem(operands[0].val <= operands[1].val);
        case GEQ: CHECK_BIN(VAL);
            return StackItem(operands[0].val >= operands[1].val);
        case LT: CHECK_BIN(VAL);
            return StackItem(operands[0].val < operands[1].val);
        case GT: CHECK_BIN(VAL);
            return StackItem(operands[0].val > operands[1].val);
        case EQ: CHECK_BIN(VAL);
            return StackItem(operands[0].val == operands[1].val);
        case ADD: CHECK_BIN(VAL);
            return StackItem(operands[0].val + operands[1].val);
        case SUB:
            if (operands.size() == 1) {
                CHECK_UN(VAL);
                return StackItem(- operands[0].val);
            } else if (operands.size() == 2) {
                CHECK_BIN(VAL);
                return StackItem(operands[0].val - operands[1].val);
            } else {
                require(false, "Invalid number of arguments for operator '-'");
            }
        case MUL: CHECK_BIN(VAL);
            return StackItem(operands[0].val * operands[1].val);
        case DIV: CHECK_BIN(VAL);
            require(operands[1].val != 0, "Division by zero");
            return StackItem(operands[0].val / operands[1].val);
        case AND: CHECK_BIN(BV);
            return StackItem(operands[0].bv && operands[1].bv);
        case OR: CHECK_BIN(BV);
            return StackItem(operands[0].bv || operands[1].bv);
        case NOT: CHECK_UN(BV);
            return StackItem(!operands[0].bv);
        default:
            require(false, "Invalid operation!");
    }
}

#define EXPECT(x) require(std::getchar() == x, "Expected " #x);

int main(int argc, char *argv[])
{
    std::deque<StackItem> stack;
    char chr = std::getchar();
    while (chr != EOF) {
        switch (chr) {
            case '<':
                chr = std::getchar();
                if (chr == '=') {
                    stack.emplace_back(Op::LEQ);
                } else {
                    stack.emplace_back(Op::LT);
                    continue;
                }
                break;
            case '>':
                chr = std::getchar();
                if (chr == '=') {
                    stack.emplace_back(Op::GEQ);
                } else {
                    stack.emplace_back(Op::GT);
                    continue;
                }
                break;
            case '=':
                stack.emplace_back(Op::EQ);
                break;
            case 'a': EXPECT('n'); EXPECT('d');
                stack.emplace_back(Op::AND);
                break;
            case 'o': EXPECT('r');
                stack.emplace_back(Op::OR);
                break;
            case 'n': EXPECT('o'); EXPECT('t');
                stack.emplace_back(Op::NOT);
                break;
            case '+':
                stack.emplace_back(Op::ADD);
                break;
            case '-':
                stack.emplace_back(Op::SUB);
                break;
            case '*':
                stack.emplace_back(Op::MUL);
                break;
            case '/':
                stack.emplace_back(Op::DIV);
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                    std::stringstream sb;
                    sb << chr;
                    chr = getchar();
                    while (chr >= '0' && chr <= '9') {
                        sb << chr;
                        chr = getchar();
                    }
                    stack.emplace_back(StackItem::val_t(sb.str()));
                    continue;
                }
                break;
            case ')': {
                    std::deque<StackItem> v;
                    while (stack.back().kind != StackItem::OP) {
                        v.push_front(stack.back());
                        stack.pop_back();
                    }
                    auto o = stack.back().op;
                    stack.pop_back();
                    stack.push_back(eval(o, v));
                }
                break;
            case ' ': case '\n': case '\t': case '(':
                break;
            default:
                require(false, "Invalid input");
        }
        chr = getchar();
    }

    for (const auto& si : stack) {
        std::cout << "> " << si << std::endl;
    }

    return 0;
}
