#include <stdio.h> // printf
#include <conio.h> // getch
#include <windows.h> // LocalAlloc

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
#define TYPE_FUNC_CALL      3
/* TYPE_FUNC_CALL:
        *name - ���
        args - ���������� ����������
        *dopargs - ������ ���������� �� ( �� )
*/
#define TYPE_IF             4
/* TYPE_IF:
    *name = ��������� � ���� ������

    ����� ��������� �������
 */
#define TYPE_ELSE           5 // elseif - ����
/* TYPE_ELSE:
    ����� ��������� �������
*/
#define TYPE_FOR            6
/* TYPE_FOR:
    *name - �������
    *dopargs - ������ �� ( )

    ����� ��������� �������
*/
#define TYPE_WHILE          7
/* TYPE_WHILE
    *name - �������

    ����� ��������� �������
*/
#define TYPE_START_DO_WHILE 8
/* TYPE_START_DO_WHILE:
    ����� ��������� �������
*/
#define TYPE_NEW_FUNC       10
/* TYPE_NEW_FUNC:
        DATATYPE
        *name - ��� �������
        *dopargs - ��������� � ���� ������

        ����� ��������� �������
*/
#define TYPE_START_BRACE    11
#define TYPE_END_BRACE      12

#define TYPE_START_ROUND_BRACKETS 13
#define TYPE_END_ROUND_BRACKETS   14

#define TYPE_STRING         15
#define TYPE_NUMBER         16
#define TYPE_VOID           17 // 50% - �� �������
#define TYPE_MATH           18
/* TYPE_MATH:
    name - type ( + - * / )
*/
#define TYPE_TRANSFORM      19
/* TYPE_TRANSFORM:
        DATATYPE
        args - ����� ��������
        ����� ��������� �������
*/

#define TYPE_EQU            20
#define TYPE_POINT          21
/* TYPE_POINT:
    *name - name
*/
#define TYPE_ARRAY          22
/* TYPE_ARRAY:
    name - ��� ����� ������ [ ]
*/
#define TYPE_STRUCT_point   23
#define TYPE_STRUCT_arrow   24

// uint, true, false, null, bool
#define uint    unsigned int
#define true    1
#define false   0
#define null    0
#define bool    char

#define malloc(len) LocalAlloc(LPTR, len);
#define remalloc(adr, len) LocalReAlloc( LocalHandle(adr), len, LPTR);
#define free(adr) LocalFree( LocalHandle(adr) );

// DEBUG
//#define DEBUG_PARSER

// ============= VAR ================

char *code = "a[b]].\nc;"; // �������� ���

unsigned int lines = 1; // ������� ����� ��������

char **LocalMalloc_arr = 0; // ������ �� ����� ��� LocalMalloc
unsigned int LocalMalloc_arr_index = 0; // ������ ���������� �������� ������ LocalMalloc
unsigned int LocalMalloc_arr_index_max = 20;

unsigned int LAST_ID_IF = 0;
unsigned int LAST_ID_BRACE = 0;

// ============= FUNC ===============

// char* ( malloc )
char *LocalMalloc(unsigned int len){ // �������� ������ � ��������� ����� � �����
    if( LocalMalloc_arr_index >= LocalMalloc_arr_index_max ){
        LocalMalloc_arr_index_max += 20;
        remalloc( LocalMalloc_arr, sizeof( char* ) * LocalMalloc_arr_index_max );
    }

    if( LocalMalloc_arr == 0 ){ LocalMalloc_arr = malloc( sizeof( char* ) * LocalMalloc_arr_index_max ); }
    LocalMalloc_arr[LocalMalloc_arr_index] = malloc( len );
    LocalMalloc_arr_index++;
    return LocalMalloc_arr[ LocalMalloc_arr_index - 1 ];
}

void LocalMalloc_Clear(){ // ������� ��� ��������� ������ �������� � ������� LocalMalloc
    for(unsigned int i = 0; i<LocalMalloc_arr_index; i++){
        free( LocalMalloc_arr[i] ); // �������� ������
    }
    LocalMalloc_arr = 0;
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
// PRINT ERROR
void error(char *code, char *errorText){ // ������� ������ �� ����� � ������������� ���������
    printf("ERROR: %s\n", errorText);
    code = copystr(code, 0);
    code[10]='\0';
    printf("%d | \"", lines);
    for(uint i=0; i<=10; i++){
        if( code[i]==0 || code[i]=='\n' ){ break; }
        printf("%c", code[i]);
    }
    printf("\"\n");
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

        bool UNSIGNED;
        bool REGISTER;

        char datatype;

        char *name;

        unsigned int args;

        char *dopArgs;
};

// ============= PARSER =============

struct command *parser(){
    // code - 'string' code
    // ����� ������� ������ ���������� ���, � ������ ��������� ����������� ���� �������

    struct command *tmpcom = 0; // �������� ��������� ��� ��������� �������
    if( tmpcom==0 ){ tmpcom = LocalMalloc( sizeof( struct command ) ); } // ��������� ������ ��� ���������

    tmpcom->datatype = 0;
    tmpcom->REGISTER = 0;
    tmpcom->UNSIGNED = 0;
    tmpcom->type = 0;
    tmpcom->name = 0;
    tmpcom->args = 0;
    tmpcom->dopArgs = 0;

    uint i = 0; // ��� ������ while
    uint count = 0;
    char *EndCode = code + lenstr(code);

    // ========= CODE =============================================================================
    Start:

    if( code[0]==0 ){ tmpcom = 0; goto ExitParser; }

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i > EndCode ){ return 0; }
    code+=i;

    if( code[0]==';' ){ // ���� �� ����� ; �� ������� �� ����� ��������� � ����������� �����
        code++;
        goto Start;
    }

    // MATH =========================================================================================
    if( code[0]=='+' ){
        tmpcom->type = TYPE_MATH;
        tmpcom->name = "+";
        code++;
        goto ExitParser;
    }

    if( code[0]=='-' ){
        tmpcom->type = TYPE_MATH;
        tmpcom->name = "-";
        code++;
        goto ExitParser;
    }

    if( code[0]=='*' ){
        tmpcom->type = TYPE_MATH;
        tmpcom->name = "*";
        code++;
        goto ExitParser;
    }

    if( code[0]=='/' ){
        tmpcom->type = TYPE_MATH;
        tmpcom->name = "/";
        code++;
        goto ExitParser;
    }
    // END MATH ========================================================================================

    if( code[0]=='.' ){
        tmpcom->name = ".";
        tmpcom->type = TYPE_STRUCT_point;
        code++;
        goto ExitParser;
    }

    if( code[0]=='-' && code[1]=='>' ){
        tmpcom->name = "->";
        tmpcom->type = TYPE_STRUCT_arrow;
        code+=2;
        goto ExitParser;
    }

    if( code[0]=='[' ){
        code++;
        tmpcom->type = TYPE_ARRAY;

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
        code+=i;

        if( code[0]==']' ){ goto ExitParser; }
        count = 1;
        i = 0;
        while(1){
            if( count <= 0 ){ i--; break; }
            if( code+i > EndCode ){ error(code+i, "I'm waiting [ ]"); }
            if( code[i] == '[' ){ count++; }
            if( code[i] == ']' ){ count--; }
            i++;
        }
        tmpcom->name = copystr(code, i);
        code+=i+1;
        goto ExitParser;
    }

    if( code[0]=='{' ){
        tmpcom->type = TYPE_START_BRACE;
        code++;
        tmpcom->name = "{";
        goto ExitParser;
    }
    if( code[0]=='}' ){
        tmpcom->type = TYPE_END_BRACE;
        code++;
        tmpcom->name = "}";
        goto ExitParser;
    }

    if( code[0]=='(' ){
        code++;
        while(1){
            i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
            if( code+i>EndCode ){ return 0; }
            code+=i;

            if( code > EndCode ){ return 0; }

            if( cmpstr( code, "unsigned" ) ){ tmpcom->UNSIGNED  = true; code += 8; } else
            if( cmpstr( code, "register" ) ){ tmpcom->REGISTER  = true; code += 8; } else {
                break;
            }
            if( code[0]!=' ' && code[0]!=')' ){
                if( tmpcom->UNSIGNED == true ){ code-=8; tmpcom->UNSIGNED = false; }else
                if( tmpcom->REGISTER == true ){ code-=8; tmpcom->REGISTER = false; }
                break;
            }
        }

        while(1){
            i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
            if( code+i>EndCode ){ return 0; }
            code+=i;

            if( code > EndCode ){ return 0; }

            if( cmpstr( code, "char"  ) ){ tmpcom->datatype = DATATYPE_CHAR;   code += 4; } else
            if( cmpstr( code, "short" ) ){ tmpcom->datatype = DATATYPE_SHORT;  code += 5; } else
            if( cmpstr( code, "int"   ) ){ tmpcom->datatype = DATATYPE_INT;    code += 3; } else
            if( cmpstr( code, "long"  ) ){ tmpcom->datatype = DATATYPE_LONG;   code += 4; } else
            if( cmpstr( code, "float" ) ){ tmpcom->datatype = DATATYPE_FLOAT;  code += 5; } else
            if( cmpstr( code, "double") ){ tmpcom->datatype = DATATYPE_DOUBLE; code += 6; } else
            if( cmpstr( code, "void"  ) ){ tmpcom->datatype = DATATYPE_VOID;   code += 4; } else {
                if(tmpcom->datatype == 0){ tmpcom->datatype = DATATYPE_NONE; }
                break;
            }
            if( code[0]!=' ' && code[0]!=')' ){
                if( tmpcom->datatype == DATATYPE_CHAR ){ code-=4; }else
                if( tmpcom->datatype == DATATYPE_SHORT ){ code-=5; }else
                if( tmpcom->datatype == DATATYPE_INT ){ code-=3; }else
                if( tmpcom->datatype == DATATYPE_LONG ){ code-=4; }else
                if( tmpcom->datatype == DATATYPE_FLOAT ){ code-=5; }else
                if( tmpcom->datatype == DATATYPE_DOUBLE ){ code-=6; }else
                if( tmpcom->datatype == DATATYPE_VOID ){ code-=4; }
                tmpcom->datatype = DATATYPE_NONE;
                break;
            }
        }

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
        code+=i;
        if( code[0]==')' && ( tmpcom->datatype || tmpcom->UNSIGNED || tmpcom->REGISTER ) ){ code++; }

        if( tmpcom->datatype || tmpcom->REGISTER || tmpcom->UNSIGNED ){
            tmpcom->type = TYPE_TRANSFORM;
        } else {
            tmpcom->type = TYPE_START_ROUND_BRACKETS;
            tmpcom->name = "(";
        }
        goto ExitParser;
    }
    if( code[0]==')' ){
        tmpcom->type = TYPE_END_ROUND_BRACKETS;
        code++;
        tmpcom->name = ")";
        goto ExitParser;
    }

    if( code[0]=='=' ){
        code++;
        tmpcom->type = TYPE_EQU;
        tmpcom->name = "="; // ��� ����������
        goto ExitParser;
    }

    if( ('0'<=code[0] && code[0]<='9') || code[0] == '\'' ){ // IS NUMBER? =================================================

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

        } else if( code[0] == '\'' ){ // CHAR =========================================================================

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

        } else { // SINGLE NUMBER ============================================================================

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
        if( code+i>EndCode ){ return 0; }
        code+=i;

        if( code > EndCode ){ return 0; }

        if( cmpstr( code, "unsigned " ) ){ tmpcom->UNSIGNED  = true; code += 9; } else
        if( cmpstr( code, "register " ) ){ tmpcom->REGISTER  = true; code += 9; } else {
            break;
        }
    }

    while(1){
        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
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
    if( code+i>EndCode ){ return 0; }
    code+=i;

    if( code > EndCode ){ return 0; } // ������ ����� ������� ��� �� �� ������� � ������

    // NAME ========================================================================================================================
    i = 0; // ��� ������ ���� ��� ���������� �����
    while(1){ // ���������� ����� � �����, �� �� ������ ������� ������� �� �����
        if( code[i] == 0 ){ break; }
        if( code+i > EndCode ){ return 0; } // ����� �������� �� ������
        if( ('0'<=code[i] && code[i]<='9') || ('A'<=code[i] && code[i]<='Z')
           || ('a'<=code[i] && code[i]<='z') || code[i]=='_' || code[i]=='*' || code[i]=='&' ){ i++; } else { break; }
    }

    tmpcom->name = copystr(code, i); // �������� ��� � ���������
    code+=i; // ������������� ���

    if( code[0]==':' ){ // POINT =============================================================================================
        tmpcom->type = TYPE_POINT;
        code++;
        goto ExitParser;
    }

    // �������� ���������������� ����
    if( cmpstr(tmpcom->name, "if") ){ // IF ==========================================================================================
        tmpcom->type = TYPE_IF; // ��� IF ))

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
        code+=i;

        if( code[0]=='(' ){ // ������� ������������, ��� ����� if ������ ��� ������� ������
            code++; // ������������� (
            if( code[0]==')' ){ code++; tmpcom->name = 0; goto ExitParser; }
            i = 0; // ��� ������ ���� ��� ���������� �����
            count=1; // ���������� ������
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
        goto ExitParser;

    } else if( cmpstr(tmpcom->name, "else") ){ // ELSE ===================================================================
        if( code[0]==' ' || code[0]=='{' ){
            tmpcom->type = TYPE_ELSE;
            goto ExitParser;
        }
    }
    if( cmpstr(tmpcom->name, "elseif") ){ // ELSEIF ===================================================================
        if( code[0]==' ' || code[0]=='(' ){
            tmpcom->type = TYPE_ELSE;
            code -= 2;
            goto ExitParser;
        }
    }

    if( cmpstr(tmpcom->name, "for") ){ // FOR ==========================================================================================
        tmpcom->type = TYPE_FOR;

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
        code+=i;

        if( code[0]=='(' ){
            code++; // ������������� (
            if( code[0]==')' ){ code++; tmpcom->name = 0; goto ExitParser; }
            i = 0; // ��� ������ ���� ��� ���������� �����
            count=1; // ���������� ������
            while(1){
                if( code[i] == '(' ){ count++; }
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
        goto ExitParser;

    } else if( cmpstr(tmpcom->name, "while") ){ // WHILE ==========================================================================================
        tmpcom->type = TYPE_WHILE;

        i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
        if( code+i>EndCode ){ return 0; }
        code+=i;

        if( code[0]=='(' ){
            code++; // ������������� (
            if( code[0]==')' ){ code++; tmpcom->name = 0; goto ExitParser; }
            i = 0; // ��� ������ ���� ��� ���������� �����
            count=1; // ���������� ������
            while(1){
                if( code[i] == '(' ){ count++; }
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
        goto ExitParser;

    } else if( cmpstr(code-2, "do ") ){ // DO ==========================================================================================
        tmpcom->type = TYPE_START_DO_WHILE;
        code++;
        goto ExitParser;

    }

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i>EndCode ){ return 0; }
    if( code[i] == '(' && tmpcom->datatype == 0 ){ // CALL FUNC =====================================================================
        code += i+1;
        tmpcom->type = TYPE_FUNC_CALL;
        i = 0; // ��� ������ ���� ��� ���������� �����
            count = 1; // ���������� ������
            while(1){
                if( code[i] == '(' ){ count++; }
                if( code[i] == ')' ){ count--; }
                if( code[i] == ')' && count <= 0 ){ i++; break; } // ^^^
                if( code[i] == '\n' ){ lines++; } // ���� ����� ������ �� ��������� ���������� ��������� ����� � ��������� lines
                if( code[i] == 0 ){ break; } // ���� ����� ������ �� �������
                if( code+i > EndCode ){ return 0; } // ���������� ��� ��� � �������
                i++;
            }
       if( count <= 0 ){ tmpcom->dopArgs = 0; }else{ tmpcom->dopArgs = copystr(code, i-1); }
       code+=i;
       goto ExitParser;
    }

    if( code > EndCode ){ return 0; } // ����� �������� ��� ��� � �������

    #ifdef DEBUG_PARSER
    printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\n", (int)(tmpcom->UNSIGNED), (int)(tmpcom->REGISTER) );
    printf("\t\tDATATYPE: %d\n\t\tNAME: \'%s\'\n", tmpcom->datatype, tmpcom->name);
    #endif // DEBUG_PARSER

    i = IgnoreSpace(code); // ��� ��� ������� ������ ������������� �������� ( space, \n )
    if( code+i>EndCode ){ return 0; }
    code+=i;            // ^^^

    if( code[0] == '(' ){ // NEW FUNC ======================================================================================
        tmpcom->type = TYPE_NEW_FUNC;
        code++;
        i = 0; // ��� ������ ���� ��� ���������� �����
            count = 1; // ���������� ������
            while(1){
                if( code[i] == '(' ){ count++; }
                if( code[i] == ')' ){ count--; }
                if( code[i] == ')' && count <= 0 ){ i++; break; } // ^^^
                if( code[i] == '\n' ){ lines++; } // ���� ����� ������ �� ��������� ���������� ��������� ����� � ��������� lines
                if( code[i] == 0 ){ break; } // ���� ����� ������ �� �������
                if( code+i > EndCode ){ return 0; } // ���������� ��� ��� � �������
                i++;
            }
        tmpcom->dopArgs = copystr(code, i);
        code+=i;
        goto ExitParser;
    }

    if( code[0] == '=' || code[0] == ';' || code[0] == 0 || code[0]==' ' ||
        code[0]==')' || code[0]=='+' || code[0]=='-' || code[0]=='*' || code[0]=='/' || code[0]=='[' ){ // ���� ��� ��������� �� ����� ������
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
    */
}

struct command **Loop1(unsigned int *len){
    uint max = 20;
    uint index = 0;
    char *LastCode = code;
    uint LastCodeIndex = 0;
    struct command **com = LocalMalloc( sizeof(char*)*max );
    while(1){
        if( index >= max ){ max+=20; remalloc(com, sizeof(char*)*max); }
        com[index] = parser();
        if( com[index]==0 || code[0]==0 ){ break; }
        if( code == LastCode ){ LastCodeIndex++; }
        if( LastCodeIndex>=3 ){ error(code, "..."); }
        LastCode = code;
        index++;
    }
    *len = index;
    return com;
}
int main(){

    uint len;
    struct command **com = Loop1(&len);

    printf("len: %d\n\n", len);
    for(uint i=0; i<len; i++){
        struct command *tmpcom = com[i];
        printf("Unsigned: %d\nRegister: %d\nDataType: %d\nTYPE: %d\nName: %s\nDopArgs: %s\n\n", tmpcom->UNSIGNED, tmpcom->REGISTER, tmpcom->datatype, tmpcom->type, tmpcom->name, tmpcom->dopArgs);
    }
    LocalMalloc_Clear();

    return 0;
}
