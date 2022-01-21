#include <stdio.h> // printf
#include <stdlib.h> // malloc

#include <conio.h> // getch

//========== DEFINES ===========

// DATATYPES
#define DATATYPE_NONE       0
#define DATATYPE_CHAR       1
#define DATATYPE_SHORT      2
#define DATATYPE_INT        3
#define DATATYPE_LONG       4
#define DATATYPE_FLOAT      5
#define DATATYPE_DOUBLE     6
#define DATATYPE_VOID       7

// TYPE
#define TYPE_NONE           0
/* TYPE_NONE:
    �� ���� ��� ������ ��������� ��� ��������� ������
*/
#define TYPE_NEW_VAR        1
#define TYPE_VAR            2
/* TYPE_VAR and TYPE_NEW_VAR:
    DATATYPE - ������ � TYPE_NEW_VAR
    *name - ��� ���������
*/
#define TYPE_FUNC_CALL      3  // �� �����������
/* TYPE_FUNC_CALL:
        *name - ���
        args - ���������� ����������
        *dopargs - ����� ���������� �� ������ � ���������� command
*/
#define TYPE_IF             4
/* TYPE_IF:
    *name = ��������� � ���� ������
    args - id

    ����� ��������� �������
 */
#define TYPE_ELSE           5  // �� �����������
/* TYPE_ELSE:
    args - id

    ����� ��������� �������
*/
#define TYPE_FOR            6  // �� �����������
/* TYPE_FOR:
    *name - �������
    *dopargs - ����� ���������� �� ������ � ����������� command

    ����� ��������� �������
*/
#define TYPE_WHILE          7  // �� �����������
/* TYPE_WHILE
    *name - �������

    ����� ��������� �������
*/
#define TYPE_START_DO_WHILE 8  // �� �����������
/* TYPE_START_DO_WHILE:
    args - id
    ����� ��������� �������
*/
#define TYPE_END_DO_WHILE   9  // �� �����������
/* TYPE_END_DO_WHILE:
    *name - �������
    args - id

    ����� ��������� �������
*/
#define TYPE_NEW_FUNC       10 // �� �����������
/* TYPE_NEW_FUNC:
        DATATYPE
        *name - ��� �������
        *dopargs - ��������� � ���� ������

        ����� ��������� �������
*/
#define TYPE_START_BRACE    11 // �� �����������
/* TYPE_START_BRACE:
    args - id
    ����� ��������� �������
*/
#define TYPE_END_BRACE      12 // �� �����������
/* TYPE_END_BRACE:
    args - id
*/

#define TYPE_STRING         13
#define TYPE_NUMBER         14
#define TYPE_VOID           15 // 50% - �� �������
#define TYPE_MATH           16 // �� �����������
#define TYPE_TRANSFORM      17 // �� �����������
/* TYPE_TRANSFORM:
        DATATYPE
        name - ����� �� ����� ��������
        args - ����� ��������
*/

#define TYPE_EQU            18
#define TYPE_POINT          19 // �� �����������

// uint, true, false, null, bool
#define uint    unsigned int
#define true    1
#define false   0
#define null    0
#define bool    char

// DEBUG

//#define DEBUG_PARSER

// ============= VAR ================

char *code = "    if  ( ( abc > 5 ) || ( abc < 10 ) ) printf(\"hi\");"; // �������� ���
char *EndCode; // ����� ����� ����

unsigned int lines = 0; // ������� ����� ��������

char **LocalMalloc_arr = 0; // ������ �� ����� ��� LocalMalloc
unsigned int LocalMalloc_arr_index = 0; // ������ ���������� �������� ������ LocalMalloc

unsigned int LAST_ID_IF = 0;
unsigned int LAST_ID_BRACE = 0;

// ============= FUNC ===============

// char* ( malloc )
char *LocalMalloc(unsigned int len){ // �������� ������ � ��������� ����� � �����
    if( LocalMalloc_arr == 0 ){ LocalMalloc_arr = malloc( sizeof( char* ) * 20 ); }
    LocalMalloc_arr[LocalMalloc_arr_index] = malloc( len );
    LocalMalloc_arr_index++;
    return LocalMalloc_arr[ LocalMalloc_arr_index - 1 ];
}
// bool
bool LocalMalloc_Clear(){ // ������� ��� ��������� ������ �������� � ������� LocalMalloc
    for(unsigned int i = 0; i<LocalMalloc_arr_index; i++){
        // ����������, ���������� �������
        // free( (char*)(LocalMalloc[i]) ); // �������� ������
    }
    return true;
}
// length string
unsigned int lenstr(char *str){ // ����� ������ ASCII (�� ������� 0)
    register unsigned int len=0;
    while(1){
        if(str[len]==0){ return len; }
        len++;
    }
}
// bool
bool cmpstr(char *str, char *find){ // �������� ������ ASCII (�� 0 � str ��� find)
    for(unsigned int i=0; i<lenstr(find); i++){
        if(str[i]==0){ return 0; }
        if(str[i]!=find[i]){ return 0; }
    }
}
// char* ( LocalMalloc )
char *copystr(char *start, unsigned int len){ // ����������� ������ � ��������� ������ (� ������� LocalMalloc, ASCII)
    if(len == 0){
        len = lenstr(start);
    }
    char *buf = LocalMalloc(len+1);
    for(unsigned int i=0; i<len; i++){
        buf[i]=start[i];
        buf[i+1]=0;
    }
    return buf;
}
// CLEAR STRUCT/ARRAY
void cleararr(char *arr, unsigned int len){ // ��������� ����� ������
    for(unsigned int i=0; i<len; i++){
        arr[i]=0;
    }
}

// PRINT ERROR
void error(char *code, char *errorText){ // ������� ������ �� ����� � ������������� ���������
    printf("ERROR: %s\n", errorText);
    code = copystr(code, 0);
    code[10]='\0';
    printf("%d | %s\n", lines, code);
    exit(1);
}

// IGNORE SPACE
unsigned int IgnoreSpace(char *code){ // ������������ �������� (ASCII)
    unsigned int i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( code[i] == '\n' ){ lines++; }
        if( code[i] == ' ' || code[i] < '!' ){ i++; } else { break; }
    }
    return i;
}

// =========== STRUCT ===============

struct command{ // �������� ��������� ����������� �������
        char type;
        /*
        TYPE:
            0  - NONE
            1  - NEW VAR (=)
            2  - VAR (=)
            3  - FUNC. CALL
            4  - IF ( args )
            5  - ELSE
            6  - FOR( com; args; com)
            7  - while( args )
            8  - do...
            9  - ...while( args )
            10 - NEW FUNC.
            11 - { (start)
            12 - (end) }

            13 - string
            14 - number
            15 - void
            16 - ariphmetic (a+b*c)
            17 - transform ( (int)(abc) )

            18 - = (equ)
            19 - point
        */

        bool UNSIGNED;  // bool
        bool REGISTER;  // bool

        char datatype;  // int
        /*
        TYPE:
            0 - NONE
            1 - CHAR
            2 - SHORT
            3 - INT
            4 - LONG
            5 - FLOAT
            6 - DOUBLE
            7 - VOID
        */

        char *name; // if "if" than this condition

        unsigned int args; // id ^^^

        char *dopArgs;
};

// ============= PARSER =============

struct command *parser(){
    // code - 'string' code
    // ����� ������� ������ ���������� ���, � ������ ��������� ����������� ���� �������

    struct command *tmpcom = 0; // �������� ��������� ��� ��������� �������
    if( tmpcom==0 ){ tmpcom = LocalMalloc( sizeof( struct command ) ); } // ��������� ������ ��� ���������
    cleararr( tmpcom, sizeof(tmpcom) ); // ��������� ��������� ������ (����������� ������ ��� ������)
    tmpcom->name = 0; // ����� �� ���������� ������, �� ���� �������� ���� 0 �� ��� ��������

    uint i = 0; // ��� ������ while

    // ========= CODE =========

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i >= EndCode ){ return 0; }
    code+=i;

    if( code[0]==';' ){ return 0; } // ���� �� ����� ; �� ������� 0

    if( code[0]=='=' ){
        code++;
        tmpcom->type = TYPE_EQU;
        tmpcom->name = "="; // ��� ����������
        goto ExitParser;
    }

    if( ('0'<=code[0] && code[0]<='9') || code[0] == '\'' ){ // IS NUMBER?

        if( code[0]=='0' && code[1]=='x' ){ // HEX

            tmpcom->type = TYPE_NUMBER;
            i = 2;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == 0 ){ break; }
                if( ('0'<=code[i] && code[i]<='9') || (code[i] == 'A' || code[i] == 'B' || code[i] == 'C' || code[i] == 'D' || code[i] == 'E' || code[i] == 'F'
                   || code[i] == 'a' || code[i] == 'b' || code[i] == 'c' || code[i] == 'd' || code[i] == 'e' || code[i] == 'f') ){ i++; } else { break; }
            }
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;

        } else if( code[0] == '\'' ){ // CHAR

            tmpcom->type = TYPE_NUMBER;
            i = 1;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == '\\' && code[i+1] == '\'' ){ i+=2; }
                if( code[i] == 0 ){ error(code, "where \' ?"); }
                if( code[i] == '\'' ){ break; }
                i++;
            }
            i++;
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;

        } else { // SINGLE NUMBER

            tmpcom->type = TYPE_NUMBER;
            i = 0;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == 0 ){ break; }
                if( ('0'<=code[i] && code[i]<='9') ){ i++; } else { break; }
            }
            if( code[i] == 'h' ){ i++; } // or HEX
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;
        }
    }

    if( code[0]=='\"' ){
        tmpcom->type = TYPE_STRING;
        i=1;
        while(1){
            if( code+i > EndCode ){ return 0; }
            if( code[i] == 0 || code[i]=='\"' ){ break; }
            i++;
        }
        tmpcom->name = copystr(code, ++i);
        code+=i;
        goto ExitParser;
    }

    // DATATYPE
    while(1){
        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>=EndCode ){ return 0; }
        code+=i;

        if( code > EndCode ){ return 0; }

        if( cmpstr( code, "unsigned " ) ){ tmpcom->UNSIGNED  = true; code += 9; } else
        if( cmpstr( code, "register " ) ){ tmpcom->REGISTER  = true; code += 9; } else {
            break;
        }
    }

    while(1){
        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>=EndCode ){ return 0; }
        code+=i;

        if( code > EndCode ){ return 0; }

        if( cmpstr( code, "char "  ) ){ tmpcom->datatype = DATATYPE_CHAR;   code += 5; } else
        if( cmpstr( code, "short " ) ){ tmpcom->datatype = DATATYPE_SHORT;  code += 6; } else
        if( cmpstr( code, "int "   ) ){ tmpcom->datatype = DATATYPE_INT;    code += 4; } else
        if( cmpstr( code, "long "  ) ){ tmpcom->datatype = DATATYPE_LONG;   code += 5; } else
        if( cmpstr( code, "float " ) ){ tmpcom->datatype = DATATYPE_FLOAT;  code += 6; } else
        if( cmpstr( code, "double ") ){ tmpcom->datatype = DATATYPE_DOUBLE; code += 7; } else
        if( cmpstr( code, "void "  ) ){ tmpcom->datatype = DATATYPE_VOID;   code += 5; } else {
            if(tmpcom->datatype == 0){
                tmpcom->datatype = DATATYPE_NONE;
            }
            break;
        }
    }

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i>=EndCode ){ return 0; }
    code+=i;

    if( code > EndCode ){ return 0; } // ������ ����� ������� ��� �� �� ������� � ������

    // NAME
    i = 0; // ��� ������ ���� ��� ���������� �����
    while(1){ // ���������� ����� � �����, �� �� ������ ������� ������� �� �����
        if( code[i] == 0 ){ break; }
        if( code+i > EndCode ){ return 0; } // ����� �������� �� ������
        if( ('0'<=code[i] && code[i]<='9') || ('A'<=code[i] && code[i]<='Z')
           || ('a'<=code[i] && code[i]<='z') || code[i]=='_' || code[i]=='*' || code[i]=='&' ){ i++; } else { break; }
    }

    tmpcom->name = copystr(code, i); // �������� ��� � ���������
    code+=i; // ������������� ���

    if( cmpstr(tmpcom->name, "if") ){ // IF - ���������
        tmpcom->type = TYPE_IF; // ��� IF ))

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>=EndCode ){ return 0; }
        code+=i;                // ^^^

        if( code[0]=='(' ){ // ������� ������������, ��� ����� if ������ ��� ������� ������
            code++; // ������������� (
            i = 0; // ��� ������ ���� ��� ���������� �����
            unsigned int count=1; // ���������� ������
            while(1){
                if( code[i] == '(' ){ count++; } // ��� ��� ��� ����������� ������ ��������� ������� � ���� if
                if( code[i] == ')' ){ count--; }
                if( code[i] == ')' && count <= 0 ){ break; } // ^^^
                if( code[i] == '\n' ){ lines++; } // ���� ����� ������ �� ��������� ���������� ��������� ����� � ��������� lines
                if( code[i] == 0 ){ break; } // ���� ����� ������ �� �������
                if( code+i > EndCode ){ return 0; } // ���������� ��� ��� � �������
                i++;
            }
            tmpcom->name = copystr(code, i);
            code+=i+1;
        } else { error(code, "I'm waiting ( )"); }
        LAST_ID_IF++;
        tmpcom->args = LAST_ID_IF;
    }

    if( code > EndCode ){ return 0; } // ����� �������� ��� ��� � �������

    #ifdef DEBUG_PARSER
    printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\n", (int)(tmpcom->UNSIGNED), (int)(tmpcom->REGISTER) );
    printf("\t\tDATATYPE: %d\n\t\tNAME: \'%s\'\n", tmpcom->datatype, tmpcom->name);
    #endif // DEBUG_PARSER

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i>=EndCode ){ return 0; }
    code+=i;            // ^^^

    if( code[0] == '=' || code[0] == ';' ){ // ���� ��� ��������� �� ����� ������
        if( tmpcom->REGISTER || tmpcom->UNSIGNED || tmpcom->datatype ){
            tmpcom->type = TYPE_NEW_VAR;
        }else{
            tmpcom->type = TYPE_VAR;
        }
    }

    #ifdef DEBUG_PARSER
        printf("\t\tTYPE: %d\n", tmpcom->type);
    #endif // DEBUG_PARSER

    ExitParser: // ============ EXIT PARSER ===============
    return tmpcom;

    /*
        ����� ��� ������������ ���� �� ����� �� ����� ���� ���������.

        ������� ��� ������� TYPE_NEW_VAR, TYPE_VAR.
        ����� ����������\����������� ��� � ����� ����

        ������� �����:
        TYPE_NEW_VAR | TYPE_VAR (����� �������, �������� ��� ����� if)
        TYPE_NUMBER
        TYPE_STRING
        TYPE_IF
    */
}

int main(){
    EndCode = code + lenstr(code);

    //printf( "%d\n", (int)( parser(code) ) );

    for(uint i=0; i<4; i++){
        struct command *tmpcom = parser( code );

        if( tmpcom == 0 ){
            printf("\t\t(null)\n");
        } else {
            printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\n", (int)(tmpcom->UNSIGNED), (int)(tmpcom->REGISTER) );
            printf("\t\tDATATYPE: %d\n\t\tNAME: ", tmpcom->datatype);
            if( tmpcom->name == 0 ){
                printf("(null)\n");
            } else {
                printf("\"%s\"\n", tmpcom->name);
            }
            printf("\t\tTYPE: %d\n\n", tmpcom->type);
            printf("\t\tARGS: %d\n\n", tmpcom->args);
        }
    }

    return 0;
}
