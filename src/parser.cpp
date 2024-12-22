#include "value.h"
#ifndef PARSER
#define PARSER

// parser of myscheme

#include "RE.h"
#include "Def.h"
#include "syntax.h"
#include "expr.h"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Expr Syntax::parse(Assoc &env) {
    return ptr->parse(env);
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr Identifier::parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    // initPrimitives();
    // initReservedWords();
    // 先检查是否为空，保证后续可以下标访问
    if (stxs.empty()) {
        throw RuntimeError("No matched operation");
    }

    auto first_ptr = stxs[0].get();
    // 若为identifiers类型
    if (dynamic_cast<Identifier *>(first_ptr)) {
        string first_word = dynamic_cast<Identifier *>(first_ptr)->s;
        Value if_find = find(first_word, env);
        // 先检查是否重定义
        if (if_find.get()) {
            vector<Expr> new_rand;
            for (int i = 1; i < stxs.size(); i++) {
                new_rand.push_back(stxs[i].get()->parse(env));
            }
            return Expr(new Apply(first_ptr->parse(env), new_rand));
        }

        if (reserved_words.find(first_word) != reserved_words.end()) {
            switch (reserved_words[first_word]) {
                case E_QUOTE: {
                    if (stxs.size() == 2) {
                        return Expr(new Quote(stxs[1]));
                    }
                    throw RuntimeError("Variable Amount Error");
                    break;
                }


                case E_BEGIN: {
                    std::vector<Expr> temp;
                    for (int i = 1; i <= stxs.size() - 1; i++) {
                        temp.push_back(stxs[i]->parse(env));
                    }
                    return Expr(new Begin(temp));
                    break;
                }


                case E_IF: {
                    if (stxs.size() == 4) {
                        return Expr(new If(stxs[1]->parse(env), stxs[2]->parse(env), stxs[3]->parse(env)));
                    } else {
                        throw RuntimeError("Variable Amount Error");
                    }
                    break;
                }


                case E_LAMBDA: {
                    if (stxs.size() == 3) {
                        std::vector<std::string> x;
                        // lambda有自己的作用域
                        Assoc cur_env = env;
                        // stxs[1]对应参数列表
                        if (dynamic_cast<List *>(stxs[1].get())) {
                            List *temp_ptr = dynamic_cast<List *>(stxs[1].get());
                            for (const auto &c: temp_ptr->stxs) {
                                // 参数是var,变量
                                if (dynamic_cast<Var *>(c->parse(env).get())) {
                                    Var *v_ptr = dynamic_cast<Var *>(c->parse(env).get());
                                    x.push_back(v_ptr->x);
                                    // lambda表达式最初创建时，x、y未赋值，只是个形参
                                    // （具体值应该是在closure后面的expr*zh绑定的吧，需要测试能否正常运行）
                                    cur_env = extend(v_ptr->x, NullV(), cur_env);
                                } else {
                                    throw RuntimeError("Not A Var");
                                }
                            }
                            // 注意求值应该是在新作用域下，否则找不到对应变量
                            Expr temp_expr = stxs[2]->parse(cur_env);
                            return Expr(new Lambda(x, temp_expr));
                        } else
                            throw RuntimeError("Variable Error");
                    } else
                        throw RuntimeError("Variable Amount Error");
                    break;
                }

                case E_LET: {
                    Assoc cur_env = env;
                    std::vector<std::pair<std::string, Expr> > bind;
                    if (stxs.size() == 3) {
                        if (dynamic_cast<List *>(stxs[1].get())) {
                            List *lst_ptr = dynamic_cast<List *>(stxs[1].get());
                            for (const auto &stx: lst_ptr->stxs) {
                                if (dynamic_cast<List *>(stx.get())) {
                                    List *temp_lst = dynamic_cast<List *>(stx.get());
                                    if (temp_lst->stxs.size() == 2) {
                                        if (dynamic_cast<Identifier *>(temp_lst->stxs[0].get())) {
                                            Identifier *id_ptr = dynamic_cast<Identifier *>(temp_lst->stxs[0].get());
                                            string var_name = id_ptr->s;
                                            Expr expr = temp_lst->stxs[1].get()->parse(env);
                                            cur_env = extend(var_name, NullV(), cur_env);
                                            bind.push_back(std::make_pair(var_name, expr));
                                            // 第一次绑定，变量无法使用
                                        } else
                                            throw RuntimeError("Var Error");
                                    } else
                                        throw RuntimeError("Variable Amount Error");
                                } else
                                    throw RuntimeError("Variable Amount Error");
                            }
                            Expr last_expr = stxs[2].parse(cur_env);
                            return Expr(new Let(bind, last_expr));
                        } else
                            throw RuntimeError("Variable Amount Error");
                    } else
                        throw RuntimeError("Variable Amount Error");
                    break;
                }

                case E_LETREC: {
                    Assoc cur_env1 = env;
                    Assoc cur_env2 = env;
                    vector<pair<string, Expr> > bind;
                    if (stxs.size() == 3) {
                        if (dynamic_cast<List *>(stxs[1].get())) {
                            List *lst_ptr = dynamic_cast<List *>(stxs[1].get());
                            for (const auto stx: lst_ptr->stxs) {
                                if (dynamic_cast<List *>(stx.get())) {
                                    List *temp_lst = dynamic_cast<List *>(stx.get());
                                    if (temp_lst->stxs.size() == 2) {
                                        if (dynamic_cast<Identifier *>(temp_lst->stxs[0].get())) {
                                            // 第一个新作用域
                                            Identifier *id_ptr = dynamic_cast<Identifier *>(temp_lst->stxs[0].get());
                                            string var_name = id_ptr->s;
                                            // Expr expr = temp_lst->stxs[1].get()->parse(env);
                                            // bind.push_back(std::make_pair(var_name, expr));
                                            cur_env1 = extend(var_name, Value(), cur_env1);
                                        } else
                                            throw RuntimeError("Var Error");
                                    } else
                                        throw RuntimeError("Var and Expr Error");
                                } else
                                    throw RuntimeError("List Error");
                            }

                            // for (const auto stx: lst_ptr->stxs) {
                            //     // 条件之前已经判断过了
                            //     List *temp_lst = dynamic_cast<List *>(stx.get());
                            //     Identifier *id_ptr = dynamic_cast<Identifier *>(temp_lst->stxs[0].get());
                            //     // 在env1下对expr*求值
                            //     Expr expr = temp_lst->stxs[1].get()->parse(cur_env1);
                            //     // 绑定到env2
                            //     cur_env2 = extend(id_ptr->s, expr->eval(cur_env1), cur_env2);
                            // }
                            for (const auto stx: lst_ptr->stxs) {
                                List *temp_lst = dynamic_cast<List *>(stx.get());
                                Identifier *id_ptr = dynamic_cast<Identifier *>(temp_lst->stxs[0].get());
                                Expr expr = temp_lst->stxs[1]->parse(cur_env1);
                                bind.push_back(std::make_pair(id_ptr->s, expr));
                            }
                            Expr last_expr = stxs[2]->parse(cur_env1);
                            return Expr(new Letrec(bind, last_expr));
                        } else
                            throw RuntimeError("List Error");
                    } else
                        throw RuntimeError("Variable Amount Error");
                    break;
                }

                default:
                    throw RuntimeError("No Matched Reserved Word");
                    break;
            }
        }

        // 若为primitives
        // 根据subtask4，它也应该被视为函数调用,按照函数调用写即可
        if (primitives.find(first_word) != primitives.end()) {
            // const Expr rator(first_ptr->parse(env));
            vector<Expr> new_rand;
            for (int i = 1; i < stxs.size(); i++) {
                new_rand.push_back(stxs[i].get()->parse(env));
            }
            // primitives是全局作用域的函数
            return Expr(new Apply(first_ptr->parse(env), new_rand));
        }

        // 剩余情况
        vector<Expr> new_rand;
        for (int i = 1; i < stxs.size(); i++) {
            new_rand.push_back(stxs[i].get()->parse(env));
        }
        return Expr(new Apply(first_ptr->parse(env), new_rand));
        // throw RuntimeError("Not defined");
    }

    // 若为list类型
    if (dynamic_cast<List *>(first_ptr)) {
        vector<Expr> new_rand;
        for (int i = 1; i < stxs.size(); i++) {
            new_rand.push_back(stxs[i].get()->parse(env));
        }
        return Expr(new Apply(dynamic_cast<List *>(first_ptr)->parse(env), new_rand));
    }

    vector<Expr> new_rand;
    for (int i = 1; i < stxs.size(); i++) {
        new_rand.push_back(stxs[1]->parse(env));
    }
    return Expr(new Apply(first_ptr->parse(env), new_rand));
    // throw RuntimeError("Grammar Error");
}

#endif
