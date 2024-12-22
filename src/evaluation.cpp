#include <cmath>

#include "Def.h"
#include "value.h"
#include "expr.h"
#include "RE.h"
#include "syntax.h"
#include <string>
#include <vector>
#include <map>
#include<algorithm>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Value Let::eval(Assoc &env) {
    Assoc cur_env=env;
    std::vector<std::string>vars;
    // 检查是否有变量名重复
    for(auto c:bind) {
        // 检查变量命名是否规范
        if (c.first.size() == 0) {
            throw RuntimeError("Var Error");
        }
        // 命名规则必须满足
        if (isdigit(c.first[0]) || c.first[0] == '.' || c.first[0] == '@') {
            throw RuntimeError("Banned Name");
        }
        for (auto c: c.first) {
            if (c == '#' || c == '\'' || c == '\"' || c == '`') {
                throw RuntimeError("Banned Name");
            }
        }
        vars.push_back(c.first);
        cur_env=extend(c.first,c.second->eval(env),cur_env);
        if(std::count(vars.begin(),vars.end(),c.first)>1) {
            throw RuntimeError("Repeated Vars");
        }
    }

    // 在当前作用域下求值
    return body->eval(cur_env);
} // let expression

Value Lambda::eval(Assoc &env) {
    // 正如parser中注释所说，lambda应该有自己的作用域
    Assoc temp_env = env;
    return ClosureV(x, e, temp_env);
} // lambda expression

Value Apply::eval(Assoc &e) {
    Value rat_res = rator->eval(e); // 先执行函数体，根据函数体的情况执行后续操作
    if (rat_res->v_type == V_PROC) {
        Closure *clos_ptr = dynamic_cast<Closure *>(rat_res.get());

        // 检查参数数目：形参数量必须等于实参数量
        if (rand.size() != clos_ptr->parameters.size()) {
            throw RuntimeError("Argument Amount Error");
        }

        // 在当前作用域对expr*求值
        // std::vector<Value> vs;
        // for (auto &c: rand) {
        //     vs.push_back(c->eval(e));
        // }

        // 扩展出新作用域
        Assoc cur_e = clos_ptr->env;
        for (int i = 0; i < clos_ptr->parameters.size(); i++) {
            cur_e = extend(clos_ptr->parameters[i], rand[i]->eval(e), cur_e);
        }

        // 对函数体中的表达式在新作用域下求值
        return clos_ptr->e->eval(cur_e);
    }
    throw RuntimeError("Value Variable Wrong");
} // for function calling

Value Letrec::eval(Assoc &env) {

} // letrec expression

Value Var::eval(Assoc &e) {
    // 在当前作用域中尝试寻找变量x
    Value temp = find(x, e);
    // 名字显然不能为空，也确保后面可以下标访问
    if (x.size() == 0) {
        throw RuntimeError("Var Error");
    }
    // 命名规则必须满足
    if (isdigit(x[0]) || x[0] == '.' || x[0] == '@') {
        throw RuntimeError("Banned Name");
    }
    for (auto c: x) {
        if (c == '#' || c == '\'' || c == '\"' || c == '`') {
            throw RuntimeError("Banned Name");
        }
    }

    // 还要判断是不是数字

    if (temp.get()) {
        return temp;
    } else {
        if (primitives.find(x) == primitives.end()) {
            throw RuntimeError("Var Error");
        } else {
            Expr expr{};
            std::vector<std::string> parameters;
            switch (primitives[x]) {
                case E_MUL:
                    parameters = {"Mul_l", "Mul_r"};
                    expr = Expr(new Mult(Expr(new Var("Mul_l")), Expr(new Var("Mul_r"))));
                    break;

                case E_MINUS:
                    parameters = {"Minus_l", "Minus_r"};
                    expr = Expr(new Minus(Expr(new Var("Minus_l")), Expr(new Var("Minus_r"))));
                    break;

                case E_PLUS:
                    parameters = {"Plus_l", "Plus_r"};
                    expr = Expr(new Plus(Expr(new Var("Plus_l")), Expr(new Var("Plus_r"))));
                    break;

                case E_LT:
                    parameters = {"Lt_l", "Lt_r"};
                    expr = Expr(new Less(Expr(new Var("Lt_l")), Expr(new Var("Lt_r"))));
                    break;

                case E_LE:
                    parameters = {"Le_l", "Le_r"};
                    expr = Expr(new LessEq(Expr(new Var("Le_l")), Expr(new Var("Le_r"))));
                    break;

                case E_EQ:
                    parameters = {"Eq_l", "Eq_r"};
                    expr = Expr(new Equal(Expr(new Var("Eq_l")), Expr(new Var("Eq_r"))));
                    break;

                case E_GE:
                    parameters = {"Ge_l", "Ge_r"};
                    expr = Expr(new GreaterEq(Expr(new Var("Ge_l")), Expr(new Var("Ge_r"))));
                    break;

                case E_GT:
                    parameters = {"Gt_l", "Gt_r"};
                    expr = Expr(new Greater(Expr(new Var("Gt_l")), Expr(new Var("Gt_r"))));
                    break;

                case E_VOID:
                    expr = Expr(new MakeVoid());
                    break;

                case E_EQQ:
                    parameters = {"Eqq_l", "Eqq_r"};
                    expr = Expr(new IsEq(Expr(new Var("Eqq_l")), Expr(new Var("Eqq_r"))));
                    break;

                case E_BOOLQ:
                    parameters = {"Boolq"};
                    expr = Expr(new IsBoolean(Expr(new Var("Boolq"))));
                    break;

                case E_INTQ:
                    parameters = {"Intq"};
                    expr = Expr(new IsFixnum(Expr(new Var("Intq"))));
                    break;

                case E_NULLQ:
                    parameters = {"Nullq"};
                    expr = Expr(new IsNull(Expr(new Var("Nullq"))));
                    break;

                case E_PAIRQ:
                    parameters = {"Pairq"};
                    expr = Expr(new IsPair(Expr(new Var("Pairq"))));
                    break;

                case E_PROCQ:
                    parameters = {"Procq"};
                    expr = Expr(new IsProcedure(Expr(new Var("Procq"))));
                    break;

                case E_SYMBOLQ:
                    parameters = {"Symbolq"};
                    expr = Expr(new IsSymbol(Expr(new Var("Symbolq"))));
                    break;

                case E_CONS:
                    parameters = {"Cons_l", "Cons_r"};
                    expr = Expr(new Cons(Expr(new Var("Cons_l")), Expr(new Var("Cons_r"))));
                    break;

                case E_NOT:
                    parameters = {"Not"};
                    expr = Expr(new Not(Expr(new Var("Not"))));
                    break;

                case E_CAR:
                    parameters = {"Car"};
                    expr = Expr(new Car(Expr(new Var("Car"))));
                    break;

                case E_CDR:
                    parameters = {"Cdr"};
                    expr = Expr(new Cdr(Expr(new Var("Cdr"))));
                    break;

                case E_EXIT:
                    expr = Expr(new Exit());
                    break;

                default:
                    throw RuntimeError("No matched primitive");
                    break;
            }
            Assoc cur_e = e;
            // 此处返回的parameters里面其实是自己起名字的参数，在apply中会与对应的value值绑定
            return ClosureV(parameters, expr, cur_e);
        }
    }
} // evaluation of variable

Value Fixnum::eval(Assoc &e) {
    return IntegerV(n);
} // evaluation of a fixnum

Value If::eval(Assoc &e) {
    Value temp = cond->eval(e);
    if (temp->v_type == V_BOOL && dynamic_cast<Boolean *>(temp.get())->b == false) {
        return alter->eval(e);
    } else {
        return conseq->eval(e);
    }
} // if expression

Value True::eval(Assoc &e) {
    return BooleanV(true);
} // evaluation of #t

Value False::eval(Assoc &e) {
    return BooleanV(false);
} // evaluation of #f

Value Begin::eval(Assoc &e) {
    if (es.empty()) {
        return NullV();
    }
    for (int i = 0; i < es.size(); i++) {
        es[i]->eval(e);
        if (i == es.size() - 1) {
            return es[i]->eval(e);
        }
    }
} // begin expression

Value Quote::eval(Assoc &e) {
    if (dynamic_cast<Number *>(s.get())) {
        return IntegerV(dynamic_cast<Number *>(s.get())->n);
    } else if (dynamic_cast<TrueSyntax *>(s.get())) {
        return BooleanV(true);
    } else if (dynamic_cast<FalseSyntax *>(s.get())) {
        return BooleanV(false);
    } else if (dynamic_cast<Identifier *>(s.get())) {
        return SymbolV(dynamic_cast<Identifier *>(s.get())->s);
    } else if (dynamic_cast<List *>(s.get())) {
        // 处理pair类型
        if (dynamic_cast<List *>(s.get())->stxs.empty()) {
            return NullV();
        } else {
            Value cur_pair = NullV();
            std::vector<Syntax> stxs2 = dynamic_cast<List *>(s.get())->stxs;
            // for (int i = stxs2.size() - 1; i >= 0; i--) {
            //     cur_pair = PairV(stxs2[i]->parse(e)->eval(e), cur_pair);
            // }
            int cnt = 0; // 记录点的总数
            std::vector<int> poses; // 记录每个点的位置
            for (int i = 0; i < stxs2.size(); i++) {
                if (dynamic_cast<Identifier *>(stxs2[i].get())) {
                    std::string temp = dynamic_cast<Identifier *>(stxs2[i].get())->s;
                    if (temp == ".") {
                        cnt++;
                        poses.push_back(i);
                    }
                }
            }
            if (cnt == 0) {
                if (stxs2.size() == 1) {
                    return PairV(Quote(stxs2[0]).eval(e), NullV());
                } else {
                    List *rest = new List;
                    rest->stxs = std::vector<Syntax>(stxs2.begin() + 1, stxs2.end());
                    return PairV(Quote(stxs2[0]).eval(e), Quote(Syntax(rest)).eval(e));
                }
            }
            // 按照修改后的意思，应该最多出现一个".",而且只能在倒数第二个位置
            else if (cnt == 1) {
                if (poses[0] != stxs2.size() - 2 || poses[0] == 0) {
                    throw RuntimeError("Pair Error");
                }

                if (stxs2.size() == 3) {
                    return PairV(Quote(stxs2[0]).eval(e), Quote(stxs2[2]).eval(e));
                }

                List *rest = new List;
                rest->stxs = std::vector<Syntax>(stxs2.begin() + 1, stxs2.end());
                return PairV(Quote(stxs2[0]).eval(e), Quote(Syntax(rest)).eval(e));
            } else {
                // pair结构只能有一个"."
                throw RuntimeError("Pair Error");
            }
            // return cur_pair;
        }
    }

    throw RuntimeError("No Matched type");
} // quote expression

Value MakeVoid::eval(Assoc &e) {
    return VoidV();
} // (void)

Value Exit::eval(Assoc &e) {
    return TerminateV();
} // (exit)

Value Binary::eval(Assoc &e) {
    return evalRator(rand1->eval(e), rand2->eval(e));
} // evaluation of two-operators primitive

Value Unary::eval(Assoc &e) {
    return evalRator(rand->eval(e));
} // evaluation of single-operator primitive

Value Mult::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return IntegerV(dynamic_cast<Integer *>(rand1.get())->n * dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // *

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        // 基类指针转换为派生类指针,以调用需要的成员
        return IntegerV(dynamic_cast<Integer *>(rand1.get())->n + dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // +

Value Minus::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return IntegerV(dynamic_cast<Integer *>(rand1.get())->n - dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // -

Value Less::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n < dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // <

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n <= dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // <=

Value Equal::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n == dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // =

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n >= dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n > dynamic_cast<Integer *>(rand2.get())->n);
    }

    throw RuntimeError("Type Error");
} // >

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV(dynamic_cast<Integer *>(rand1.get())->n == dynamic_cast<Integer *>(rand2.get())->n);
    } else if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL) {
        return BooleanV(dynamic_cast<Boolean *>(rand1.get())->b == dynamic_cast<Boolean *>(rand2.get())->b);
    } else if (rand1->v_type == V_SYM && rand2->v_type == V_SYM) {
        return BooleanV(dynamic_cast<Symbol *>(rand1.get())->s == dynamic_cast<Symbol *>(rand2.get())->s);
    } else if ((rand1->v_type == V_NULL && rand2->v_type == V_NULL) || (
                   rand1->v_type == V_VOID && rand2->v_type == V_VOID)) {
        return BooleanV(true);
    } else {
        return BooleanV(rand1.get() == rand2.get()); // 其余情况比较内存位置
    }
} // eq?

Value Cons::evalRator(const Value &rand1, const Value &rand2) {
    return PairV(rand1, rand2);
} // cons  构造pair

Value IsBoolean::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_BOOL);
} // boolean?

Value IsFixnum::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_INT);
} // fixnum?

Value IsSymbol::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_SYM);
} // symbol?

Value IsNull::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_NULL);
} // null?

Value IsPair::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_PAIR);
} // pair?

Value IsProcedure::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_PROC);
} // procedure?

Value Not::evalRator(const Value &rand) {
    return BooleanV(rand->v_type == V_BOOL && dynamic_cast<Boolean *>(rand.get())->b == false); // 似乎只有这一种情况下取反是true
} // not

Value Car::evalRator(const Value &rand) {
    if (rand->v_type == V_PAIR) {
        return dynamic_cast<Pair *>(rand.get())->car;
    }

    throw RuntimeError("Type Error");
} // car

Value Cdr::evalRator(const Value &rand) {
    if (rand->v_type == V_PAIR) {
        return dynamic_cast<Pair *>(rand.get())->cdr;
    }

    throw RuntimeError("Type Error");
} // cdr
