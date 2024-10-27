#include "turtle-graphics.h"
#include "../neillsimplescreen.h"

int main(int argc, char **argv){
    test();

    Parser* c = calloc(1, sizeof(Parser));
    c->args = argc;
    turtle_init(c);
    stack_init(c);

    FILE* fp = fopen(argv[READFILE], "r");
    FILE* wp = fopen(argv[WRITEFILE], "w");
    on_error(c, fp, wp, argc); //check for any issues with allocating memory or locating files
    load_ins(c, fp); //copy instructions from TTL file into a 2d array
    fclose(fp);

    if (!prog(c)){
        fprintf(stderr, "Invalid grammar, failed to parse!");
        exit(EXIT_FAILURE);
    }

    if (argc == 3){
        print_grid(c, wp);
    }

    else {
        print_screen(c);
    }

    parser_free(c);
}

void load_ins(Parser* c, FILE* fp){
    int i = 0;

    while (fscanf(fp, "%s", c->instruction[i]) != EOF){ //copy all instructions in TTL file to parser
        i++;
    }

    c->instruction[i][0] = '\0';
}

void stack_init(Parser* c){
    c->stack = calloc(1, sizeof(Stack));
    if (c->stack == NULL){
        fprintf(stderr, "failed to allocate stack memory!");
        exit(EXIT_FAILURE);
    }
    c->stack->top = EMPTY_STACK; //initialise stack pointer to signal an empty stack
}

void turtle_init(Parser* c){
    c->turtle = calloc(1, sizeof(Turtle)); 
    if (c->turtle == NULL){
        fprintf(stderr, "failed to allocate turtle memory!");
        exit(EXIT_FAILURE);
    }
    c->turtle->y = (MAXHEIGHT/2);
    c->turtle->x = (MAXWIDTH/2); //set starting positions
    c->turtle->oldY = 0;
    c->turtle->oldX = 0;
    c->turtle->angle = FWDANGLE; //set starting angle
    c->turtle->colour = 'W'; //set starting colour

    for (int y = 0; y < MAXHEIGHT; y++){
        for (int x = 0; x < MAXWIDTH; x++){
            c->turtle->grid[y][x] = '\0'; //populate the turtle grid with null characters
        }
    }
}

void on_error(Parser* c, FILE* fp, FILE* wp, int argc){

    if ((argc <= 1) || (argc > 3)){
        fprintf(stderr, "invalid number of arguments.\nUsage: ./filename <TTLfile> <outputfile>");
        exit(EXIT_FAILURE);
    }

    if (c == NULL){
        fprintf(stderr, "failed to allocate memory!");
        exit(EXIT_FAILURE);
    }

    if (fp == NULL){
        fprintf(stderr, "failed to locate file!");
        exit(EXIT_FAILURE);
    }

    if (argc == 3){
        if (wp == NULL){ //if file to write to is unable to be found even though there are 3 arguments exit code 1
            fprintf(stderr, "failed to locate file to write to!");
            exit(EXIT_FAILURE);
        }
    }
}

void parser_free(Parser* c){ //free all dynamically allocated memory
    free(c->stack);
    free(c->turtle);
    free(c);
}

bool prog(Parser* c){

    if (samestr(c->instruction[c->cw], "START")){ //check that each file begins with a START command
        c->cw = c->cw + 1;
        if (inslst(c)){ 
            return true;
        }
    }
    return false;
}

bool inslst(Parser* c){

    if (samestr(c->instruction[c->cw], "END")){
        return true;
    }

    if (ins(c)){ //not the end - must be an instruction
        c->cw = c->cw + 1; //next letter
        return inslst(c);//recursion until END reached
    }
    return false;
}

bool ins(Parser* c){
    //check through the list of instructions until a match is found
    if (fwd(c)){
        return true;
    }

    if (rgt(c)){
        return true;
    }

    if (loop(c)){
        return true;
    }
    
    if (col(c)){
        return true;
    }

    if (set(c)){
        return true;
    }

    if (rectangle(c)){
        return true;
    }

    if (triangle(c)){
        return true;
    }

    return false;
}

bool fwd(Parser* c){

    char answer[MAXTOKENSIZE];
    if (samestr(c->instruction[c->cw], "FORWARD")){
        c->cw = c->cw + 1;
        if (var(INSTRUCTION)){ //if variable follows forward, access the variable in the array and set the distance to var value
            int i = find_var(INSTRUCTION[1]);
            validVar(i);

            if (c->variable[i].in_use){
                c->turtle->distance = c->variable[i].value;
                draw_line(c);
                if (c->args == 2){ //if only two arguments have been specified 
                    print_screen(c);
                }
            }
            return true; //if a number has been found in the var function it must be a distance
        }
        
        if (num(INSTRUCTION)){
            c->turtle->distance = strtod(INSTRUCTION, NULL); //convert string to a double and draw a line
            draw_line(c);
            if (c->args == 2){
                print_screen(c);
            }
            return true;
        }

        else {
            c->cw = c->cw - 1; //go one step back because no value was added
            ask(INSTRUCTION, answer);
            c->turtle->distance = strtod(answer, NULL);
            draw_line(c);
            if (c->args == 2){
                print_screen(c);
            }
            return true;
        }
    }
    return false;
}

bool rgt(Parser* c){
    char answer[MAXTOKENSIZE];

    if (samestr(c->instruction[c->cw], "RIGHT")){
        c->cw = c->cw + 1;

        if (var(INSTRUCTION)){ //if var found, set angle to be current angle - variable.value
            int i = find_var(INSTRUCTION[1]);
            validVar(i);

            if (c->variable[i].in_use){
                c->turtle->angle -= c->variable[i].value; //subtract value stored at variable
                return true;
            }
        }

        if (num(INSTRUCTION)){
            c->turtle->angle -= strtod(INSTRUCTION, NULL); //convert string to double, take away from current angle
            return true;
        }

        else {
            c->cw = c->cw - 1; //go one step back because no value was added
            ask(INSTRUCTION, answer);
            c->turtle->angle -= strtod(answer, NULL);
            return true;
        }
    }
    return false;
}

bool col(Parser* c){
    char answer[MAXTOKENSIZE];
    if (samestr(INSTRUCTION, "COLOUR")){
        c->cw = c->cw + 1;

        if (var(INSTRUCTION)){
            int i = find_var(INSTRUCTION[1]);
            validVar(i);
            if (c->variable[i].in_use){
                if (c->variable[i].colour != '\0'){ //if colour isnt null assign the colour to the turtle
                    c->turtle->colour = c->variable[i].colour;
                }
            }
             return true; //return true because a valid variable was passed in 
        }

        if (word(INSTRUCTION)){
            if (validword(INSTRUCTION)){ //if a valid colour has been passed in, assign that colour to the turtle's pen
                c->turtle->colour = assign_col(INSTRUCTION);
            }
            return true; //return true because a valid word has been passed in 
        }

        else { 
            c->cw = c->cw - 1;
            ask(INSTRUCTION, answer);
            c->turtle->colour = assign_col(answer);
            return true;
        }
    }
    return false;
}

bool loop(Parser* c) {
    if (samestr(INSTRUCTION, "LOOP")) {
        c->cw = c->cw + 1;

        if (ltr(INSTRUCTION[0])) {
            int v = find_var(INSTRUCTION[0]);
            validVar(v);
            c->variable[v].in_use = true; //set the loop variable to be in use

            c->cw = c->cw + 1;

            if (samestr(INSTRUCTION, "OVER")) { 
                c->cw = c->cw + 1;
                int start = c->cw; //save location of instruction pointer

                if (!lst(c)) { //parse through the list to check every item in it can be assigned to the loop variable
                fprintf(stderr, "invalid loop items!\n");
                    exit(EXIT_FAILURE);
                }

                c->cw = start; //return instruction pointer to start of the list
                loopsetup(c, v);

                return true;
            }
        }
    }
    return false;
}

bool rectangle(Parser* c) {
    double height = 0;
    double width = 0;
    double prev_angle = c->turtle->angle;
    if (samestr(INSTRUCTION, "RECTANGLE")) {
        c->cw = c->cw + 1;

        if (rectangle_setup(c, &height, &width)){
            draw_rectangle(c, height, width);
            c->turtle->angle = prev_angle; //reset angle
            return true;
        }
    }
    return false;
}
 
bool rectangle_setup(Parser* c, double* h, double* w){
    char answer1[MAXTOKENSIZE];
    char answer2[MAXTOKENSIZE]; //declare the two answer strings

    if (samestr(INSTRUCTION, "HEIGHT")){
        c->cw = c->cw + 1;
        
        if (check_dimensions(c, h, answer1)){ //check that the number folllowing height is valid and pass it into height variable
            c->cw = c->cw + 1; 
            if (samestr(INSTRUCTION, "WIDTH")){
                c->cw = c->cw + 1; //check width is valid and pass into width variable
                if (check_dimensions(c, w, answer2)){
                    return true;
                }
            }
        }
    }
    return false;
}

bool check_dimensions(Parser* c, double* dimension, char* answer){
    if (num(INSTRUCTION)){
        *dimension = strtod(INSTRUCTION, NULL); //if number, set dimension to number
        return true; 
    }

    if (var(INSTRUCTION)){
        int v = find_var(INSTRUCTION[1]);
        validVar(v);
        if (c->variable[v].in_use){
            *dimension = c->variable[v].value; //if var, set dimension to var
            return true;
        }
    }

    c->cw = c->cw - 1; //go back a step to see what function we just came from
    ask(INSTRUCTION, answer); //obtain a response from the user
    *dimension = strtod(answer, NULL); //store user input into dimension
    return true;
}

void draw_rectangle(Parser* c, double height, double width){
    c->turtle->angle = FWDANGLE; //face turtle upright
    c->turtle->distance = height; //go up this distance
    draw_line(c);
    if (c->args == 2){
        print_screen(c); //output to screen if no output file specified
    }

    c->turtle->angle = RGTANGLE; //face turtle right;
    c->turtle->distance = width;
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }


    c->turtle->angle = DWNANGLE; //face turtle down
    c->turtle->distance = height;
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }


    c->turtle->angle = LFTANGLE; //face turtle left
    c->turtle->distance = width;
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }

}

bool triangle(Parser* c) {
    char answer[MAXTOKENSIZE]; //initialise answer string
    double prev_angle = c->turtle->angle; //save angle

    if (samestr(INSTRUCTION, "TRIANGLE")) {
        c->cw = c->cw + 1;

        if (num(INSTRUCTION)) {
            c->turtle->distance = strtod(INSTRUCTION, NULL);
            draw_triangle(c);
            return true;
        }

        if (var(INSTRUCTION)){
            int v = find_var(INSTRUCTION[1]);
            validVar(v);

            if (c->variable[v].in_use){
                c->turtle->distance = c->variable[v].value; //restore angle turtle was facing
                draw_triangle(c);
                return true;
            }
        }
        
        else{ //prompt user input for the size of the triangle we need to draw
            c->cw = c->cw - 1;
            (ask(INSTRUCTION, answer));
            c->turtle->distance = strtod(answer, NULL);
            draw_triangle(c);
            c->turtle->angle = prev_angle;
            return true;
        }
    }
    return false;
}

void draw_triangle(Parser* c) {
    c->turtle->angle = FWDANGLE; //set angle to face forward
    c->turtle->angle += (FWDANGLE / 2);//go up 45 degrees
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }


    c->turtle->angle += FWDANGLE; //make a 90 degree turn to come back down
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }


    c->turtle->distance++; //extend distance to go back to starting point.
    c->turtle->angle += FWDANGLE + (FWDANGLE/2); //go back across to the start 
    draw_line(c);
    if (c->args == 2){
        print_screen(c);
    }


    c->turtle->distance--;
}
 
void loopsetup(Parser* c, int v){
    int cnt = 0;
    c->cw = c->cw + 1;

    if (samestr(INSTRUCTION, "}")){ //if the first item after a closing brace is the opening brace return immediately
        return; //no need to execute this loop
    }

    while (!(samestr(INSTRUCTION, "}"))){
        strcpy(c->variable[v].loop.loop_ins[cnt], INSTRUCTION); //store item list into the loop struct for this variable
        c->cw = c->cw + 1;
        cnt++; //count how many items there are in the list
    }
    c->cw = c->cw + 1; //advance past the closing brace

    c->variable[v].loop.ins_start = c->cw; //set start of loop to instruction following closing lst brace
    c->variable[v].loop.instructions = cnt; //cnt is the amount of items in the loop list

    int j = 1;//set j to 1 since we're starting on an instruction
    
    while (!samestr(c->instruction[(c->cw + j)], "END")){
        j++; //count how many instructions there are until loop ends
    }
    c->variable[v].loop.ins_end = (c->variable[v].loop.ins_start + j); //end of the loop
    //now we need to carry out the instructions from ins_start to ins_end
    //while setting variable [i] to loop->instructions[j] while j >= cnt
    //this could be done with a for loop and an exec_loop function
    exec_loop(c, v);
}

void exec_loop(Parser* c, int v){
    //go through all loop instructions
    for (CURRENT_LOOP_INS = 0; CURRENT_LOOP_INS < c->variable[v].loop.instructions; CURRENT_LOOP_INS++){
        if (assign_loop_var(c, v)){ // assign the value of the current item in the loop to the loop variable
            c->cw = c->variable[v].loop.ins_start; //set the instruction pointer to the starting instruction of the loop
            inslst(c);
        }
    }
    c->cw = c->variable[v].loop.ins_end; //at the end of the loop, set the instruction pointer to the 
}

bool assign_loop_var(Parser* c, int v){
    if (var(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS])){ //if current item is a variable
        int item_var = find_var(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS][1]);
        validVar(item_var);
        if (c->variable[item_var].in_use){
            if(c->variable[item_var].colour != '\0'){
                c->variable[v].colour = c->variable[item_var].colour;
                c->variable[v].value = 0; //if loop var is a colour it cannot contain a value as well
                return true;
            }
            else {
                c->variable[v].value = c->variable[item_var].value;
                c->variable[v].colour = '\0'; //if loop var is a number it cannot also contain a colour
                return true;
            }
        }
    }

    if (validword(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS])){ //check if list item is a valid colour 
        c->variable[v].colour = assign_col(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS]); //if so, assign this colour to the loop variable
        c->variable[v].value = 0; //loop var is a colour, cannot contain a value
        return true;
    }

    if (num(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS])){
        c->variable[v].value = strtod(c->variable[v].loop.loop_ins[CURRENT_LOOP_INS], NULL); //set loop var to value of num in item
        c->variable[v].colour = '\0'; //loop var is a number, cannot be a colour 
        return true;
    }

    return false;
}

bool validword(char* w){
    const char* validWords[] = {"\"BLACK\"", "\"RED\"", "\"GREEN\"", 
                                "\"BLUE\"", "\"YELLOW\"", "\"CYAN\"", 
                                "\"MAGENTA\"", "\"WHITE\"", NULL};

    for (int i = 0; validWords[i] != NULL; i++) {
        if (samestr(w, validWords[i])) { //check if word passed into function was one of the valid colours specified in grammar
            return true;
        }
    }
    return false;
}

ColourCode assign_col(char* colour){ //converts a valid colour into a character to be printed on the turtle grid
    if (samestr(colour, "\"BLACK\"")) {
        return 'K';
    } 
    if (samestr(colour, "\"RED\"")) {
        return 'R';
    }
    if (samestr(colour, "\"GREEN\"")) {
        return 'G';
    }
    if (samestr(colour, "\"BLUE\"")) {
        return 'B';
    }
    if (samestr(colour, "\"YELLOW\"")) {
        return 'Y';
    }
    if (samestr(colour, "\"CYAN\"")) {
        return 'C';
    }
    if (samestr(colour, "\"MAGENTA\"")) {
        return 'M';
    }
    if (samestr(colour, "\"WHITE\"")) {
        return 'W';
    }
    else {
        return '\0';
    }
}

bool set(Parser* c){
    if (samestr(INSTRUCTION, "SET")){
        c->cw = c->cw + 1;

        if (ltr(INSTRUCTION[0])){
            int v = find_var(INSTRUCTION[0]);
            validVar(v);
            c->variable[v].in_use = true;
            c->cw = c->cw + 1;

            if (samestr(INSTRUCTION, "(")){
                c->cw = c->cw + 1;
                int previous = c->cw; //record instruction just before parsing the pfix expressions

                if (!pfix(c)){ 
                    fprintf(stderr, "invalid postfix items!\n");
                    exit(EXIT_FAILURE); //parse the set function to check its all valid
                }
                c->cw = previous; //set instruction pointer to start of expression
                set_interp(c, v);
                return true;
            }
        }
    }
    return false;
}

//process the set expression
void set_interp(Parser* c, int v){
    while (!samestr(INSTRUCTION, ")")){

        if (var(INSTRUCTION)){ //if item is a variable
            int i = find_var(INSTRUCTION[1]);
            validVar(i);
            if (c->variable[i].in_use){
                if (c->variable[i].colour != '\0'){
                    c->variable[v].colour = c->variable[i].colour; //assign colour straightaway because they should not go on the stack
                }
                else {
                    push(c, c->variable[i].value); //push variable value onto the stack
                }
            }
        }

        if (num(INSTRUCTION)){ //if postfix item is a number
            double operand = strtod(INSTRUCTION, NULL); //convert string to double
            push(c, operand); //push the number onto the stack
        }

        if (op(INSTRUCTION[0])){
            char operator = INSTRUCTION[0];
            double operand2 = pop(c); //pop top two values of stack
            double operand1 = pop(c);
            double item = apply_operation(operand1, operand2, operator); //apply the operation
            push (c, item); //push result of operation back onto the stack
        }
        c->cw = c->cw + 1; //advance instruction pointer;
    }
    if (!is_stack_empty(c)) { //if stack isnt empty
        c->variable[v].value = pop(c); //pop top of stack into value of variable;
    }
}

// Push an item onto the stack
void push(Parser* c, double item) {
    if (c->stack->top < MAX_STACK_SIZE - 1) {
        c->stack->items[++c->stack->top] = item; //increment stack pointer first, then set top of stack to new item passed in
    } 
    else {
        //stack overflow
        fprintf(stderr, "Stack overflow!\n");
        exit(EXIT_FAILURE);
    }
}

// Pop an item from the stack
double pop(Parser* c) {
    if (!is_stack_empty(c)) {
        return c->stack->items[c->stack->top--]; //return item at top of stack and decrement stack pointer
    } 
    else {
        //handle stack underflow
        fprintf(stderr, "Stack underflow!\n");
        exit(EXIT_FAILURE);
    }
}

bool is_stack_empty(Parser* c){
    return (c->stack->top == EMPTY_STACK); //check if stack pointer is = to empty stack
}

double apply_operation(double operand1, double operand2, char operator){
    switch (operator) { //perform operation based on operand
        case '+':
            return operand1 + operand2;
        case '-':
            return operand1 - operand2;
        case '*':
            return operand1 * operand2;
        case '/':
            if ((int)operand2 != 0) { //
                return operand1 / operand2;
            } 
            else {
                //if they try to divide by zero
                fprintf(stderr, "Division by zero!\n");
                exit(EXIT_FAILURE);
            }
    }
    return 0; //if invalid operator return 0 (shouldnt ever need to do this because valid operands are checked for twice)
}

bool num(char* number){
    char* endptr = NULL;
    strtod(number, &endptr); //process the instruction 

    if (*endptr != '\0'){ //end pointer should be at the end of the string, if not then an invalid number was inputted
        return false;
    }

    return true;
}

bool var(char* variable){

    if ((variable[0] == '$') && (strlen(variable) == 2)){ //check that the only thing in this string is the $ and a letter
        if (ltr(variable[1])){ 
            return true;
        }
    }

    return false;
}

bool ltr(char c){
    return (isupper(c));
}

bool varnum(char* instruction){
    return (var(instruction) || num(instruction)); //pass string into both var and num functions
}

bool word(char* instruction) {

    int length = strlen(instruction);

    if (instruction[0] == '\"'){
        if (instruction[length - 1] == '\"'){ //if the word starts and ends with double quotations it is a valid word under the grammar
            return true;
        }
    }

    return false;
}

bool pfix(Parser* c){
    if (samestr(INSTRUCTION, ")")){ //base case is the ending closing brace
        return true;
    }

    if (op(INSTRUCTION[0])){
        c->cw = c->cw + 1;
        if (!pfix(c)){ //advance instruction pointer and recursively go through the function 
            return false;
        }
    }

    if (varnum(INSTRUCTION)){
        c->cw = c->cw + 1;
        if (!pfix(c)){
            return false; 
        }
    }

    return true;
}

bool op(char op){ //check that a valid operator is present
    if (op == '+'){
        return true;
    }

     if (op == '-'){
        return true;
    }

     if (op == '/'){
        return true;
    }

     if (op == '*'){
        return true;
    }

    return false;
}

bool lst(Parser* c) {
    if (samestr(c->instruction[c->cw], "{")) {
        c->cw = c->cw + 1; //advance past closing brace

        if (items(c)) { //items returns true as soon as a closing brace is found
            c->cw = c->cw + 1; //advance past the closing brace
            return true;
        }
    }
    return false;
}

bool items(Parser* c){
    if (samestr(c->instruction[c->cw], "}")) { //base case = if a closing brace for the items list is found
        return true; 
    }

    if (!item(c)){
        return false;
    }

    c->cw = c->cw + 1;
    return items(c); // Keep caling items recursively
}

bool item(Parser* c){
    return ((varnum(INSTRUCTION)) || (word(INSTRUCTION))); //check that an list item is a valid word, number, or variable
}

void calc_position(Parser* c){
    double angleRadians = c->turtle->angle * (M_PI / 180.0); //convert angle to radians to use cos and sin functions

    int new_x = c->turtle->x + ((int) (cos(angleRadians) * (c->turtle->distance)));
    int new_y = c->turtle->y - ((int) (sin(angleRadians) * (c->turtle->distance))); 
    //subtract for y because screen coordinates increase as you go down

    //update positional variables
    c->turtle->oldX = c->turtle->x;
    c->turtle->oldY = c->turtle->y; //store the old values of x and y for the line drawing function
    c->turtle->x = new_x;
    c->turtle->y = new_y; //assign new position of the turtle
}

void draw_line(Parser* c){
    (calc_position(c));
    double dx = c->turtle->x - c->turtle->oldX;
    double dy = c->turtle->y - c->turtle->oldY; //calculate difference between the new and old coordinates

    double steps = calc_steps(dy, dx); //calculate the number of steps between the new and old coordinates
    double xIncrement = dx / steps;
    double yIncrement = dy / steps; //calculate how much to increment x and y values 

    double x = c->turtle->oldX;
    double y = c->turtle->oldY; //set starting positions 

    for (int i = 0; i < (int)steps; i++){ //cast to int to compare steps to i
        if (in_grid(x, y)){ //check that the values trying to be drawn to are within the grid
            c->turtle->grid[(int)y][(int)x] = c->turtle->colour; //cast to int to plot on grid
        }
        x += xIncrement;
        y += yIncrement; //increment x and y values to draw the next point on the line
    }
}

bool in_grid(int x, int y){
    return (check_x(x) && check_y(y));
}

bool check_x(int x){
    if ((x >= 0) && (x < MAXWIDTH)){
        return true;
    }
    return false;
}

bool check_y(int y){
    if ((y >= 0) && (y < MAXHEIGHT)){
        return true;
    }
    return false;
}

int calc_steps(int dx, int dy){
    int steps;

    if (abs(dx) > abs(dy)) {
        steps = abs(dx);
    } 

    else {
        steps = abs(dy);
    }

    return steps;
}

void print_grid(Parser* c, FILE* wp){
    for (int row = 0; row < MAXHEIGHT; row++){
        for (int col = 0; col < MAXWIDTH; col++){
            if (c->turtle->grid[row][col] == '\0'){
                fprintf(wp, " ");
            }
            else {
                fprintf(wp, "%c", c->turtle->grid[row][col]);
            }
        }
        fprintf(wp, "\n");
    }
}

int find_var(char var){
    int i = var - 'A';
    if ((i >= 0) && (i < 26)){ //check that variable is an uppercase letter
        return i;
    }
    return INVALID_VAR;
}

void print_screen(Parser* c){
    neillclrscrn(); ///clear screen
    // Iterate through the grid
    for (int j = 0; j < MAXHEIGHT; j++) {
        for (int i = 0; i < MAXWIDTH; i++) {
            if (c->turtle->grid[j][i] == '\0'){
                // Set the background colour
                neillbgcol(BACKGROUND);
                putchar(' '); //if null, print an emptyspace
            }
            else { //if a colour has been found
                neillcol colour = find_neillcol(c->turtle->grid[j][i]);
                neillfgcol(colour); //set colour
                putchar(c->turtle->grid[j][i]);//print the value stored at turtle grid
            }
            neillreset();
        }
	printf("\n");
    }
    neillbusywait(WAIT_TIME); //wait for a second after drawing a line
}
 

neillcol find_neillcol(char col){
    if (col == 'K') {
        return black; //return the appropriate neillcol for the character that was passed in
    }
    if (col == 'R') {
        return red;
    }
    if (col == 'G') {
        return green;
    }
    if (col == 'B') {
        return blue;
    }
    if (col == 'Y') {
        return yellow;
    }
    if (col == 'C') {
        return cyan;
    }
    if (col == 'M') {
        return magenta;
    }
    if (col == 'W') {
        return white;
    } 
    else{
        return white;
    }
}

void validVar(int i){
    if (i == INVALID_VAR){ //check that an invalid variable hasnt been passed in 
        fprintf(stderr, "invalid variable, please use an uppercase letter from A-Z\n");
        exit(EXIT_FAILURE);
    }
    return;
}

void ask(char* instruction, char* answer){

    //colour is a special case because we want a valid colour and not a double value
    if (samestr(instruction, "COLOUR")){
        printf("Turtle doesn't know which colour pen to use!\n Please tell them which colour to use (remember to use quotation marks around tthe colour): ");

        if (scanf("%s", answer) != 1){
            fprintf(stderr, "scanf failed!");
            exit(EXIT_FAILURE);
        }

        if (!validword(answer)){
            ask(instruction, answer); //if a valid word is not given recursively ask for a colour
        }
        return;
    }

    if (samestr(instruction, "FORWARD")){
        printf("Turtle doesn't know how far to travel!\n Please tell them how far to go: ");
    }
    if (samestr(instruction, "RIGHT")){
        printf("Turtle doesn't know which direction to turn!\n Please tell them where to face: ");
    }
    if (samestr(instruction, "TRIANGLE")){
        printf("Turtle doesn't know what size your triangle should be!\n Please tell them what size to draw: ");
    }
    if (samestr(instruction, "HEIGHT")){
        printf("Turtle doesn't know how high your rectangle should be!\n Please tell them how high to go: ");
    }
    if (samestr(instruction, "WIDTH")){
        printf("Turtle doesn't know how wide your rectangle should be!\n Please tell them how wide to go: ");
    }

    if (scanf("%s", answer) != 1){
        fprintf(stderr, "scanf failed!");
        exit(EXIT_FAILURE);
    }
    
    if (!num(answer)){
        ask(instruction, answer); //if a valid number hasn't been given, recursivly ask for one
    }
    return;
}


