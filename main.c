#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int Expression();   // prototyping

int check_if_exist(FILE *file, char* name)
{
    if (file == NULL)
    {
        printf("'%s' not found.\n", name);
        return 0;
    }
    else
    {
        printf("'%s' opened.\n", name);
        return 1;
    }
}

typedef struct{
    int type;       // type of token (there are 9)
    char str[100];  // value of token
    int length;     // length of token in number of bytes
}Token;

int ptr = 0;
int input_length = 0;
char* input_string;
enum type{TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_CHAR, TYPE_COMMENT,
    TYPE_TYPE, TYPE_RESERVED, TYPE_OPERATOR, TYPE_IDENTIFIER, TYPE_INVALID, TYPE_WHITESPACE,
    TYPE_UNARY};
FILE *input_file, *printout;


const char* getType(enum type t){
    switch (t)
    {
        case TYPE_INTEGER:
            return "TYPE_INTEGER";
        case TYPE_FLOAT:
            return "TYPE_FLOAT";
        case TYPE_STRING:
            return "TYPE_STRING";
        case TYPE_CHAR:
            return "TYPE_CHAR";
        case TYPE_COMMENT:
            return "TYPE_COMMENT";
        case TYPE_TYPE:
            return "TYPE_TYPE";
        case TYPE_RESERVED:
            return "TYPE_RESERVED";
        case TYPE_OPERATOR: return "TYPE_OPERATOR";
        case TYPE_IDENTIFIER: return "TYPE_IDENTIFIER";
        case TYPE_INVALID: return "TYPE_INVALID";
        case TYPE_WHITESPACE: return "TYPE_WHITESPACE";
        case TYPE_UNARY: return "TYPE_UNARY";
    }
}

// advances nexttoken
void next(){
    while(input_string[ptr] == ' ' || input_string[ptr] == '\t' || input_string[ptr] == '\n')
        ptr++;
}

// reached end of input
int doneWithInput(){
    //moves ptr past white spaces
    return 0;
}

int matchdatatype(char* id){
    if (strcmp(id, "void") == 0 || strcmp(id, "char") == 0 || strcmp(id, "int") == 0 || strcmp(id, "float") == 0){
        return 1;
    }
    return 0;
}

int matchreserved(char id[]){
    if (strcmp(id, "sizeof") == 0 || strcmp(id, "enum") == 0 || strcmp(id, "case") == 0 || strcmp(id, "default") == 0 ||
        strcmp(id, "if") == 0 || strcmp(id, "else") == 0 || strcmp(id, "switch") == 0 || strcmp(id, "while") == 0 ||
        strcmp(id, "do") == 0 || strcmp(id, "for") == 0 || strcmp(id, "goto") == 0 || strcmp(id, "continue") == 0 ||
        strcmp(id, "break") == 0 || strcmp(id, "return") == 0){
        return 1;
    }
    return 0;
}

int matchoperator(char op){
    if (op == '+'  || op == '-'|| op == '*' || op == '/' || op == '<' || op == '>' ||
        op == '='  || op == '!'|| op == '&' || op == '?' || op == '[' || op == ']' ||
        op == '{' || op == '}' || op == '|' || op == '(' || op == ')' || op == '%' ||
        op == '^' || op == ',' || op == ':' || op == ';' || op == '#' || op == '~' ||
        op == '\\'){
        return 1;
    }

    return 0;
}

int foundwhitespace(char space){
    if (space == ' ' || space == '\t' || space == '\n')
        return 1;
    else
        return 0;
}

void printcolor(Token *token){

    if (strcmp(getType(token->type), "TYPE_TYPE") == 0){     //blue
        printf("\x1b[34m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_STRING") == 0 || strcmp(getType(token->type), "TYPE_CHAR") == 0){       //green
        printf("\x1b[32m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_INTEGER") == 0 || strcmp(getType(token->type), "TYPE_FLOAT") == 0){     //cyan
        printf("\x1b[36m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_RESERVED") == 0){      //purple
        printf("\x1b[35m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_COMMENT") == 0){       // hite (shaded)
        printf("\x1b[37m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_OPERATOR") == 0){      //yellow
        printf("\x1b[33m%s\x1b[0m", token->str);
    } else if (strcmp(getType(token->type), "TYPE_INVALID") == 0){       //bright red
        printf("\x1b[91m%s\x1b[0m", token->str);
    }
    else{   //regular
        printf("%s", token->str);
    }
}

Token accept(Token *token, int tptr, enum type type1, char* line){
    int len = tptr - ptr +1;
    token->type = type1;
    token->length = len;

    for (int i = 0; i < len; i++){
        token->str[i] = line[ptr +i];
    }
    token->str[len] = '\0';

    // if it's an identifier - check to reserved and type types
    if(matchreserved(token->str) == 1)
        token->type = TYPE_RESERVED;
    if (matchdatatype(token->str) == 1)
        token->type = TYPE_TYPE;

    ptr = tptr;


//    return token;
}

// fill empty token with next token
void identifyNextToken(Token *token){

    int tptr = ptr;
    int state = 0;

    // pass through white spaces
    if(input_string[ptr] == ' ' || input_string[ptr] == '\n' || input_string[ptr] == '\t'){
        token->str[0] = input_string[ptr];
        token->str[1] = '\0';
        token->type = TYPE_WHITESPACE;
//        return token;
    }
        // check for operators, comments, includes and defines
    else if (matchoperator(input_string[ptr]) == 1){
        if (input_string[ptr] == '/' && input_string[tptr+1] == '/'){
            while (input_string[tptr] != '\n'){
                tptr++;
            }
            accept(token,tptr, TYPE_COMMENT, input_string);
        }
        else if (input_string[tptr] == '/' && input_string[tptr+1] == '*') {
            tptr += 3;
            while (input_string[tptr] != '*' && input_string[tptr + 1] != '/') {
                tptr++;
            }
            tptr +=1; // capture the second operator for closing comment
            accept(token, tptr, TYPE_COMMENT, input_string);
        }
        else if (input_string[ptr] == '#' && input_string[ptr+1] == 'i') {    //handle library inclusion (state 2)
            int count = 0;
            char word[10];

            while(input_string[tptr] != ' '){
                tptr++;
                word[count] = input_string[tptr];
                count++;
            }
            word[count-1] = '\0';
            while(input_string[tptr] != '\n'){
                tptr++;
            }

            if(strcmp(word, "include") == 0 || strcmp(word, "define") == 0)    // handle include libraries - type invalid for now
                accept(token, tptr, TYPE_COMMENT, input_string);
            else {
                accept(token, tptr, TYPE_INVALID, input_string);
                printf("1___%d\n", tptr);
            }
        }
        else {

            if (input_string[tptr] == '+' && (input_string[tptr +1] == '+' || input_string[tptr +1] == '=')){
                tptr++;
                accept(token, tptr, TYPE_OPERATOR, input_string);
            }
            else if (input_string[tptr] == '-' && (input_string[tptr +1] == '-' || input_string[tptr +1] == '=')){
                tptr++;
                accept(token, tptr, TYPE_OPERATOR, input_string);
            }
            else if ((input_string[tptr] == '=' || input_string[tptr] == '<' || input_string[tptr] == '>' || input_string[tptr] == '!')
                     && input_string[tptr +1] == '='){
                tptr++;
                accept(token, tptr, TYPE_OPERATOR, input_string);
            }
            else if (input_string[tptr] == '&' && input_string[tptr +1] == '&'){
                tptr++;
                accept(token, tptr, TYPE_OPERATOR, input_string);
            }
            else if (input_string[tptr] == '|' && input_string[tptr +1] == '|'){
                tptr++;
                accept(token, tptr, TYPE_OPERATOR, input_string);
            }
            else
                accept(token, tptr, TYPE_OPERATOR, input_string);
        }

    }
        // check for char
    else if (input_string[ptr] == '\''){
        // make sure there is appropriate spacing for char
        if (input_string[ptr+1] == '\\')    // handle backslash chars
            tptr = ptr+3;
        else
            tptr = ptr + 2;  // move to end of char (closing ')

        if (input_string[tptr] == '\'')
            accept(token, tptr, TYPE_CHAR, input_string);
        else {  // if there is no closing apostrophe, syntax error
            {      //syntax error, char has too many inputs
                while (input_string[tptr] != '\'')
                {
                    tptr++;
                    if (input_string[tptr] == '\0')
                        break;
                }
                accept(token, tptr, TYPE_INVALID, input_string);
            }
        }
    }
        // check for string
    else if (input_string[ptr] == '"'){
        tptr++;
        while (input_string[tptr] != '"'){
            tptr++;
            if (input_string[tptr] == '\0') // no ending quotes, syntax error
                break;
        }
        if (input_string[ptr] == input_string[tptr])
            accept(token, tptr, TYPE_STRING, input_string);
        else {
            accept(token, tptr, TYPE_INVALID, input_string);
            printf("2___%d\n", tptr);
        }
    }
        // check for integers   -- need to check for hexidecimals
    else if (input_string[ptr] >= '0' && input_string[ptr] <= '9'){
        state = 1;
        while (input_string[tptr] >= '0' && input_string[tptr] <= '9'){
            tptr++;
            // check for float (state 2)
            if (input_string[tptr] == '.'){
                tptr++; //move past decimal
                while (input_string[tptr] >= '0' && input_string[tptr] <= '9') {
                    tptr++;
                }
                state = 2;
            }
        }
        tptr--; // decrement due to while loop
        if (state == 2)
            accept(token, tptr, TYPE_FLOAT, input_string);
        else
            accept(token, tptr, TYPE_INTEGER, input_string);
    }
        //check for identifiers (state 1)
    else if ((input_string[ptr] >= 65 && input_string[ptr] <= 90) || (input_string[ptr] >= 97 && input_string[ptr] <= 122)){
        while ((input_string[tptr] >= 65 && input_string[tptr] <= 90) || (input_string[tptr] >= 97 && input_string[tptr] <= 122) ||
               (input_string[tptr] >= 48 && input_string[tptr] <= 57) || input_string[tptr] == '_' || input_string[tptr] == '.' ||
               input_string[tptr] == '\n' || input_string[tptr] == '\t'){
            tptr++;
        }
        tptr--; // decrement due to while loop
        // make sure no operator following is being added to string
        if (input_string[tptr] == '=' || input_string[tptr] == '(' || input_string[tptr] == '[' || input_string[tptr] == ' ' ||
            matchoperator(input_string[tptr]) == 1){
            tptr--;
        }
        accept(token, tptr, TYPE_IDENTIFIER, input_string);
    }
    else{
        accept(token, tptr, TYPE_INVALID, input_string);
        printf("3___%d\n", tptr);
    }

}

int match(char* string, int type){
    Token t;
    identifyNextToken(&t);
    if (strcmp(t.str, string) == 0 && t.type == type)
        return 1;

    return 0;
}

// find the type of the next token
int matchtype(int type){

    Token t;
    identifyNextToken(&t);
    if (t.type == type)
        return 1;
    return 0;
}

void GetToken(Token *t){
    while (strcmp(getType(t->type), "TYPE_WHITESPACE") == 0 ||
           strcmp(getType(t->type), "TYPE_COMMENT") == 0){
        ptr++;
        identifyNextToken(t);
    }
}

int PrimaryExpression(){

    // get a token that is not a whitespace or comment
    Token t;
    identifyNextToken(&t);
    GetToken(&t);

    GetToken(&t);
    printf("got %s: %s    [%d]\n", getType(t.type), t.str, ptr);

    if (strcmp(getType(t.type), "TYPE_INTEGER") == 0){
        // got integer so move ptr;
        ptr++;
        return atoi(t.str);
    }
    else if (strcmp(getType(t.type), "TYPE_FLOAT") == 0){
        // floats will be rounded to the nearest whole digit and turned into an int
        int i = 0;
        while (t.str[i] != '.'){
            i++;
        }
        int val;
        if (t.str[i+1] >= '5')
            val = atoi(t.str) +1;
        else
            val = atoi(t.str);

        ptr++;
        return val;
    }
    else if (strcmp(getType(t.type), "TYPE_STRING") == 0){
        ptr++;
        return 0;
    }
    else if (strcmp(getType(t.type), "TYPE_CHAR") == 0){
        ptr++;
        // return decimal value here
        if (t.str[1] == '\\')
            return t.str[2];
        else
            return t.str[1];
    }
    else if (strcmp(getType(t.type), "TYPE_IDENTIFIER") == 0){
        ptr++;
        return 0;
    }
    else if (strcmp(t.str, "(") == 0){
        ptr++;
        int val = Expression();
        return val;
    }

    return -99;
}

int PostFixExpression(){

    int firstvalue = PrimaryExpression();

    return firstvalue;
}

int UnaryExpression(){

    // if !0, return 0 and if !# then return the #

    Token prevtoken;
    identifyNextToken(&prevtoken);
    GetToken(&prevtoken);

    int firstvalue = PostFixExpression();

    if (matchtype(TYPE_OPERATOR) && (strcmp(getType(prevtoken.type), "TYPE_OPERATOR") == 0 || ptr == 0)){
        Token utoken;
        identifyNextToken(&utoken);
        GetToken(&utoken);

//        printf("(u) %s  %s\n", prevtoken.str, utoken.str);

        if (match("-", TYPE_OPERATOR)) {
            printf("    unary token: %s\n", utoken.str);
            ptr++;
            int nextvalue = PostFixExpression();
            return -nextvalue;
        }
//        else if (match("+", TYPE_OPERATOR)){
//            printf("    unary token: %s\n", utoken.str);
//            ptr++;
//            int nextvalue = PostFixExpression();
//            return nextvalue;
//        }
        else if (match("!", TYPE_OPERATOR)){
            ptr++;
            int nextvalue = UnaryExpression();  // need to call unary for - token to check for negative numbers
            if (nextvalue == 0)
                return 1;
            else
                return 0;
        }
    }

    if (firstvalue == -99){
        Token t;
        identifyNextToken(&t);
        GetToken(&t);
        printf("__either expression sums to -99 or you got an incorrect entry for primary expression__\n"
               "%s\n", t.str);
    }

    return firstvalue;

}

int MultiplicativeExpression(){

    int firstvalue = UnaryExpression();

    while(1){

        Token optoken;
        identifyNextToken(&optoken);
        GetToken(&optoken);

        if (match("*", TYPE_OPERATOR) || match("/", TYPE_OPERATOR) || match("%", TYPE_OPERATOR)){
            printf("    multi token: %s       [%d]\n", optoken.str, ptr);

            // move ptr to next token
            ptr++;
            int nextvalue = UnaryExpression();

//            Token nexttoken;
//            identifyNextToken(&nexttoken);
//            GetToken(&nexttoken);

            if (strcmp(optoken.str, "*") == 0){
                firstvalue *= nextvalue;
            }
            else if (strcmp(optoken.str, "/") == 0){
                firstvalue /= nextvalue;
            }
            else
                firstvalue %= nextvalue;

        }
        else break;
    }

    return firstvalue;
}

int AdditiveExpression(){

    int firstvalue = MultiplicativeExpression();

    while(1){

        Token optoken;
        identifyNextToken(&optoken);
        GetToken(&optoken);

//        if  (match(")", TYPE_OPERATOR))     // move past ) when we get it
//            ptr++;

        if (match("+", TYPE_OPERATOR) || match("-", TYPE_OPERATOR)){
            printf("    add token: %s       [%d]\n", optoken.str, ptr);

            // move ptr to next token and get its value
            ptr++;
            int nextvalue = MultiplicativeExpression();

//            Token nexttoken;
//            identifyNextToken(&nexttoken);
//            GetToken(&nexttoken);

            if (strcmp(optoken.str, "+") == 0){
                firstvalue += nextvalue;
            }
            else
                firstvalue -= nextvalue;

        }
        else break;
    }

    return firstvalue;

}

int RelationalExpression(){

    int firstvalue = AdditiveExpression();


    while(1){

        if (match(")", TYPE_OPERATOR))     // move past ) when we get it
            ptr++;
        else
            ptr--;

        Token reltoken;
        identifyNextToken(&reltoken);
        GetToken(&reltoken);

        if (strcmp(reltoken.str, "<") == 0 || strcmp(reltoken.str, "<=") == 0 || strcmp(reltoken.str, "==") == 0
            || strcmp(reltoken.str, "!=") == 0 || strcmp(reltoken.str, ">") == 0 || strcmp(reltoken.str, ">=") == 0){
            printf("    relational operator: %s     [%d]\n", reltoken.str, ptr);
            ptr++;
            int nextvalue = AdditiveExpression();

            if (strcmp(reltoken.str, "<") == 0)
                firstvalue = firstvalue < nextvalue;
            else if (match("<=", TYPE_OPERATOR))
                firstvalue = firstvalue <= nextvalue;
            else if (match(">", TYPE_OPERATOR))
                firstvalue = firstvalue > nextvalue;
            else if (match(">=", TYPE_OPERATOR))
                firstvalue = firstvalue >= nextvalue;
            else if (strcmp(reltoken.str, "!=") == 0)
                firstvalue = firstvalue != nextvalue;
            else
                firstvalue = firstvalue == nextvalue;

        }
        else break;
    }



    return firstvalue;
}

int LogicalExpression(){

    int firstvalue = RelationalExpression();

    while(1){

        if (match(")", TYPE_OPERATOR)) {     // move past ) when we get it
            ptr++;
            printf("    found closing ')' token\n");
        }
        else
            ptr--;

        Token logtoken;
        identifyNextToken(&logtoken);
        GetToken(&logtoken);
//        printf("%s  [%d]\n", logtoken.str, ptr);

        if (strcmp(logtoken.str, "&&") == 0 || strcmp(logtoken.str, "||") == 0){
            printf("    logical operator: %s     [%d]\n", logtoken.str, ptr);
            ptr++;
            int nextvalue = RelationalExpression();
//            printf("%d  %d\n",firstvalue, nextvalue);

            if (strcmp(logtoken.str, "&&") == 0)
                if (firstvalue == 1 && nextvalue == 1)
                    return 1;
                else
                    return 0;
            else {
                if (firstvalue == 1 || nextvalue == 1)
                    return 1;
                else
                    return 0;
            }

        }
        else break;
    }
    return firstvalue;
}


int Expression(){

    int firstvalue = LogicalExpression();
    printf("\tanswer is: %d     [%d]\n", firstvalue, ptr);


    return firstvalue;
}

int main(int argc, char* argv[]) {

    // add a random comment to push project
    // run provided file
    char* input = "save.txt";
    input_file = fopen(input, "r");

    if (argv[1] != '\0')    // if something is passed through terminal
        input = argv[1];

    input_file = fopen(input, "r");
    if (check_if_exist(input_file, input) == 0)         // if nothing can open, close program
        exit(0);

    char read;
    read = fgetc(input_file);
    // get length of file for string building
    while (read != EOF) {
        input_length++;
        read = fgetc(input_file);
    }
    printf("\n\n");

    // put the file into a string
    input_string=(char*)malloc(input_length);
    rewind(input_file);
    for(int i=0; i<input_length; i++){
        input_string[i]=fgetc(input_file);
    }
    input_string[input_length] = 0;
    fclose(input_file);


    Expression();


    return 0;
}