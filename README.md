# Scheme-Interpreter
My big task4

1.第一次提交，subtask4开始出现错误

2.第二次提交，subtask4开始出现错误。此次提交修改了parser.cpp中Let情况Expr和extend所在语句的顺序

3.第三次提交，subtask4开始出现错误。此次提交parser.cpp中Let情况最后返回值时，将last_expr去掉，直接写进返回值表达式中

4.第四次提交，subtask4开始出现错误。此次提交将之前的修改改回去了，并且parser.cpp中Let情况返回值时从.get()->parse改成了.parse

5.第五次提交，subtask5开始出现错误。此次提交将parser.cpp中lambda和let情况里Value()默认构造函数改为了NullV(),通过了subtask4

6.第六次提交，subtask5最后一个点错误。此次提交将evaluation.cpp中cur_env2的处理从extend修改为modify

7.第七次提交。此次提交修改了evaluation.cpp中letrec对应的函数，将在cur_env1下求值的过程独立出来，不再放到最后修改作用域的循环中。
