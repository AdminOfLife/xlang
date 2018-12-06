Training interpreter for x language  
===========================================
Грамматика:
-----------------------------------

**Типы лексем**:
 
1) Зарезервированные слова
var, int, float, void, begin, end, if, then, else, while, do, write, return, read, waserror, exit

2) Идентификаторы
Начинаются с буквы или “_”

3) Операции почти как в плюсах:
=, ||, &&, +, -, *, /, %, ^ (возведение в степень), !, ==, !=, <, <=, >, >=

4) Разделители:
; , ( ) EOF

5) Литерал численный:
число либо число с точкой.

6) Литерал строковый:
набор символов между “”, \n – новая строка, \t - табуляция

{} – комментарии.

Переменные могут быть глобальными и локальными. Блок “begin”…”end” 
не локализует переменные, описанные в нем. 
Локальные переменные скрывают глобальные.

Выполнение начинается с функции int main(…), она всегда должна присутствовать.

Если описание функции заканчивается “;” а не блоком “begin”…”end” – это упреждающее описание, тело этой функции должно быть объявлено где-то дальше по тексту программы.

“else” относится к ближайшему “if”.

Нетерминалы – слова без кавычек,
Терминалы – слова в кавычках.
{“a”} – повторение “a”  0 или более раз,
[a|b|c] – альтернатива между a,b или c
[a]? – a встречается 0 или 1 раз

I – идентификатор,
L – численный литерал,
Ls – строковый литерал

*program* -> {[VarDef|Func]}EOF  
*var_def* -> “var” [“int”, ”float” ]  I {“,” I}  
*func* -> [“void”, “int”, “float” ] I “(“ [ [“int”, “float”] I {“;” [“int”, “float”] I } ]? “)” [“;”| BL]  
*BL* -> “begin” ST {“;” ST} “end”  
*ST* -> [  
    I “=” E | VarDef |    
    “if” E “then” ST [“else” ST]? |  
    “while” E “do” ST |  
    BL |  
   “return” E |  
    “write” “(“  [E|Ls] {“,” [E|Ls] } “)”  |  
    “read” I |  
    “exit” E |  
    I “(“ [ E {“,” E} ]? “)”  
    ]  
*e* -> e1 { “||” e1 }  
*e1* -> e2 { “&&” e2 }  
*e2*  -> e7 [ [“==”, ”!=”]  e7 ]?  
*e7* -> e3 [  [“>”, ”>=”, ”<”, ”<=”] e3 ]?  
*e3* -> e4 { [“+”, ”-”] e4}  
*e4* -> e5 { [“*”, ”/”, ”%”]  e5 }  
*e5* -> e6 [ “^” e6 ]?  
*e6* ->   “waserror” |  
    L |  
    !e6 |  
    -e6 |  
    +e6|  
    (e) |  
    I |  
    I “(“ [ e {“,” e} ]? “)”


Сборка
------

cd /path/to/xlang/
cmake -T \<toolset-name\> -S \<path-to-source\> -B \<path-to-build\>  
\<path-to-build> - build directory  
\<toolset-name> - toolset name (visual studio, makefile e.t.c)  
\<path-to-source> - location of CMakeList.txt  

