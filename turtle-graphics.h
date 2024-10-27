#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define READFILE 1
#define WRITEFILE 2
#define WAIT_TIME 1
#define MAX_TOKENS 1000
#define MAXTOKENSIZE 100
#define MAXWIDTH 51
#define MAXHEIGHT 33
#define FWDANGLE 90
#define DWNANGLE 270
#define RGTANGLE 0
#define LFTANGLE 180
#define M_PI 3.14159265
#define MAX_STACK_SIZE 100
#define EMPTY_STACK -1
#define VARIABLE_LIST 26
#define INVALID_VAR -1
#define samestr(A,B) (strcmp(A, B) == 0)
#define INSTRUCTION c->instruction[c->cw]
#define PRINT_INS printf("current instruction is: %s \n", INSTRUCTION);
#define CURRENT_LOOP_INS c->variable[v].loop.current

typedef char ColourCode;

typedef struct Loop { 
    int ins_start; //starting instruction
    int ins_end; //ending instruction
    int current; //instruction pointer
    int instructions; //number of instructions in list
    char loop_ins[MAXTOKENSIZE][MAXTOKENSIZE]; //instructions
} Loop;

typedef struct Var {
    bool in_use; 
    double value; //use this if variable is a number
    ColourCode colour; //use this if variable is a colour
    Loop loop; //use if assigned to a loop
} Var;

typedef struct Stack { 
    double items[MAX_STACK_SIZE];
    int top; //pointer to top of stack
} Stack;

// typedef struct Postfix_Stack;

typedef struct Turtle {
    double y; // position of the tutle on the grid
    double x;
    double oldY; //save previous position for drawing function
    double oldX;
    double distance; //distance the turtle has to travel
    double angle; //direction turtle is facing
    ColourCode colour; //colour pen the turtle is holding
    char grid[MAXHEIGHT][MAXWIDTH]; //grid the turtle is on
} Turtle;

typedef struct Parser {
   char instruction[MAX_TOKENS][MAXTOKENSIZE];
   int cw;
   int args;
   Turtle* turtle;
   Var variable[VARIABLE_LIST];
   Stack* stack;
} Parser;

void on_error(Parser* c, FILE* fp, FILE* wp, int argc);

void load_ins(Parser* c, FILE* fp);

void stack_init(Parser* c);

void turtle_init(Parser* c);

void print_grid(Parser* c, FILE* wp);

void print_screen(Parser* c);

void parser_free(Parser* c);

bool prog(Parser* c);

bool inslst(Parser* c);

bool ins(Parser* c);

bool fwd(Parser* c);

bool rgt(Parser* c);

bool col(Parser* c);

bool loop(Parser* c);

bool rectangle(Parser* c);

bool rectangle_setup(Parser* c, double* h, double* w);

bool check_dimensions(Parser* c, double* dimension, char* answer);

void draw_rectangle(Parser* c, double height, double width);

bool triangle(Parser* c);

void draw_triangle(Parser* c);

void ask(char* instruction, char* answer);

void loopsetup(Parser* c, int v);

void exec_loop(Parser* c, int v);

bool assign_loop_var(Parser* c, int v);

bool set(Parser* c);

void set_interp(Parser* c, int v);

double apply_operation(double operand1, double operand2, char operator);

void push(Parser* c, double item);

double pop(Parser* c);

bool is_stack_empty(Parser* c);

bool varnum(char* instruction);

bool var(char* variable);

bool num(char* number);

bool ltr(char c);

bool word(char* instruction);

bool lst(Parser* c);

bool items(Parser* c);

bool item(Parser* c);

void validVar(int i);

bool pfix(Parser* c);

bool op(char op);

void calc_position(Parser* c);

void draw_line(Parser* c);

int calc_steps(int dy, int dx);

bool in_grid(int x, int y);

bool check_x(int x);

bool check_y(int y);

int find_var(char var);

bool loop(Parser* c);

bool validword(char* w);

char assign_col(char* colour);

void test();

