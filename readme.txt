Huy Le
COMP525 Compilers
Syntactic Parser for C Expressions


This program is a slightly modified version of the lexical analyzer with the addition of a Syntactic Parser
for C expressions. The program runs a partial Backus-Naur Form (BNF) in a top-down recursive form. It is able to
handle only a single lined expression within a file. The BNF is as follows:

<expression-statement> ::= <expression> ;

<expression> ::= { <identifier> <assignment-operator> }? <logical-expression>

<assignment-operator> ::= = | += | -=

<logical-expression> ::= <relational-expression>
			{ <logical-operator> <relational-expression> }*

<relational-expression> ::= <additive-expression>
			{ <relational-operator> <additive-expression> }*

<logical-operator> ::= && | ||

<relational-operator> ::= < | <= | > | >= | == | !=

<additive-expression> ::= <multiplicative-expression>
			{ <additive-operator> <multiplicative-expression> }*

<additive-operator> ::= + | -

<multiplicative-expression> ::= <unary-expression>
				{ <multiplicative-operator> <unary-expression> }*

<multiplicative-operator> ::= * | / | %

<unary-expression> ::= { <unary_operator> }? <postfix-expression>

<unary-operator> ::= + | - | & | * | !


<postfix-expression> ::= <primary-expression> {
			[ <expression> ]
			|	( <expression> { , <expression> }* )
			|	++
			|	--
			}*

<primary-expression> ::= <identifier> | <string> | <integer> | <char> | <float>
			|  ( <expression> )

To run the program, pass a single .c or .txt file as an argument when calling the program otherwise it will run
the provided save.txt file by default.

notes:
    A completed postfix-expression was outside of the scope of the current assignment and only primary expressions
    are completed for it. Additionally, only !, -, and + unary operators were required for this assignment as well.