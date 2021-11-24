#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
FILE *input_file;
Token t;


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

void accept(Token *token, int tptr, enum type type1, char* line){
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

//    ptr = tptr;
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
//    Token t;
    identifyNextToken(&t);
//    GetToken(&t);
//    printf("%s: %s  [%d]\n", getType(t.type), t.str, ptr);
    if (strcmp(t.str, string) == 0 && t.type == type)
        return 1;

    return 0;
}

// find the type of the next token
int matchtype(int type){
//    Token t;
    identifyNextToken(&t);
//    printf("%s: %s  [%d]\n", getType(t.type), t.str, ptr);
    GetToken(&t);
    if (t.type == type)
        return 1;
    return 0;
}

void GetToken(Token *token){
    while (strcmp(getType(token->type), "TYPE_WHITESPACE") == 0 ||
           strcmp(getType(token->type), "TYPE_COMMENT") == 0){
        ptr++;
        identifyNextToken(token);
    }
}

float primaryval, retval;
int PrimaryExpression(){

    if (matchtype(TYPE_IDENTIFIER)){
        printf("parsing IDENTIFIER\n");
        retval = 0;
        next();
    }
    else if (matchtype(TYPE_INTEGER)){
        printf("parsing INT: %s\n", t.str);
        GetToken(&t);
        retval = atoi(t.str);
        next();
    }
    else if (matchtype(TYPE_FLOAT)){
        printf("parsing FLOAT\n");
        GetToken(&t);
        retval = atof(t.str);
        next();
    }
    else if (matchtype(TYPE_CHAR)){
        printf("parsing CHAR\n");
        GetToken(&t);
        retval = t.str[1];
        next();
    }
    else if (matchtype(TYPE_STRING)){
        printf("parsing STRING\n");
        retval = 0;
        next();
    }
    else{
        printf("SYNTAX ERROR IN PRIMARY EXPRESSION\n");
        //exit(0);
        return 0;
    }
    printf("\t%s: %s  [%d]\n", getType(t.type), t.str, ptr);

    return 1;
}

int PostFixExpression(){

    int firstvalue = PrimaryExpression();

    return firstvalue;
}

int UnaryExpression(){

    // 0 dont do anything, 1 means -, 2 means !
    int unaryop = 0;

    if (matchtype(TYPE_OPERATOR))
    {
        if(match("+",TYPE_OPERATOR))
        {
            printf("parse unary: +\n");
            next();
        }
        else if(match("-",TYPE_OPERATOR))
        {
            printf("parse unary: -\n");
            unaryop = 1;
            next();
        }
        else if(match("!",TYPE_OPERATOR))
        {
            printf("parse unary: !\n");
            unaryop = 2;
            next();
        }
        else {
            printf("NOT A VALID UNARY OPERATOR\n");
            return 0;
        }

    }

    if(!PostFixExpression())
        return 0;

    //retval now is the result of primary
    printf("do unary operator evaluation here\n");
    //set retval to be based on unary operator

    if (unaryop==1)
    {
        retval = -retval;
    }
    else if (unaryop==2)
    {
        if(retval==0)
            retval=1;
        else
            retval=0;
    }

    return 1;

}

int MultiplicativeExpression(){

    UnaryExpression();
    int saveptr = ptr;

    if (matchtype(TYPE_OPERATOR)){
        ptr = saveptr;
        next();
        printf("%c  [%d]\n", input_string[ptr], ptr);
        if (match("*", TYPE_OPERATOR)){
            next();
            Token nexttoken;
            identifyNextToken(&nexttoken);
            GetToken(&nexttoken);
            printf("%s  [%d]\n", nexttoken.str, ptr);

            if (UnaryExpression()){
                GetToken(&nexttoken);
                float temp = atof(nexttoken.str);
                printf("%s  [%d]__%f\n", nexttoken.str, ptr, temp);
            }
        }
    }


    return 0;
}

int AdditiveExpression(){

    int firstvalue = MultiplicativeExpression();


    return firstvalue;

}

int RelationalExpression(){

    int firstvalue = AdditiveExpression();

    return firstvalue;
}

int LogicalExpression(){

    int firstvalue = RelationalExpression();
    printf("\tanswer is: %f     [%d]\n", retval, ptr);
    return firstvalue;
}


int Expression(){

    int firstvalue = LogicalExpression();



    return firstvalue;
}

int main(int argc, char* argv[]) {

    // run provided file
    char* input = "save.txt";
    input_file = fopen(input, "r");

    if (argv[1] != '\0')    // if something is passed through terminal
        input = argv[1];
    else if (check_if_exist(input_file, input) == 0)     // not running in IDE or nothing passed in terminal, run itself
        input = "main.c";

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

//    Token token;

    Expression();


    return 0;
}