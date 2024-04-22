#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 10
#define EXPR_COUNT 10

// Expression struct
typedef struct { 
    int operand1;
    int operand2;
    char operator;
    float result;
    int producer_id;
} Expression;

// Buffer struct
typedef struct {
    Expression expressions[BUFFER_SIZE]; //array of expressions
    int count;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} Buffer;

//Initialize functions

void *producer(void *arg);
void *consumer(void *arg);

void add_expression(Expression expr);
Expression get_expression();
Expression make_expression(int prod);

float result_expression(Expression expr);
char rand_op(); // random operator

void initialize_buffer(Buffer *buffer);
void cleanup_buffer(Buffer *buffer);

// Global variables

Buffer buffer;

int main(int argc, char* argv[]) {
    // Declare all threads
    pthread_t p1, p2, c1, c2;

    //Get buffer ready
    initialize_buffer(&buffer);

    //Thread numbers 
    int t1 = 1;
    int t2 = 2;

    // Get a random seed
    srand(time(NULL));

    // Create 2 Producers
    pthread_create(&p1, NULL, &producer, (void * ) &t1);
    pthread_create(&p2, NULL, &producer, (void * ) &t2);

    // Create 2 Consumers
    pthread_create(&c1, NULL, &consumer, (void * ) &t1);
    pthread_create(&c2, NULL, &consumer, (void * ) &t2);

    // Wait here for all threads to finish
    pthread_join(p1, NULL);
    pthread_join(c1, NULL);
    pthread_join(p2, NULL);
    pthread_join(c2, NULL);

    // Destroy the memory buffer
    cleanup_buffer(&buffer);

    return 0;
}

// -- Producer / Consumer methods --

void *producer(void *arg) {
    int prod_id = *(int *)arg; // Pass in the thread number as a void ptr

    // Each thread makes 10 expressions
    for (int i = 0; i < EXPR_COUNT; i++) {
        //Make a random expression
        Expression expr = make_expression(prod_id); 

        //Add it to the buffer
        add_expression(expr);
    }

    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg; // Pass in thread number as a void ptr

    // Each thread consumes 10 expressions
    for (int i = 0; i < EXPR_COUNT; i++) {  
        //Grab an expression from the buffer
        Expression expr = get_expression(); 

        // Print the expression and result
        printf("Producer %d -> %d %c %d = %f -> Consumed by %d\n", expr.producer_id, expr.operand1, expr.operator, expr.operand2, expr.result, id);
    }
    return NULL;
}


// -- Expression Methods --

Expression get_expression() {
    Expression expr;

    pthread_mutex_lock(&buffer.mutex); //Lock the buffer

    while (buffer.count <= 0) { //If the buffer is empty
        pthread_cond_wait(&buffer.not_empty, &buffer.mutex); //Wait for the not empty signal
    }

    expr = buffer.expressions[buffer.out]; //get the expression
    buffer.out = (buffer.out + 1) % BUFFER_SIZE; // increment the 'out' index
    buffer.count--; // Increment the amount of items in the buffer

    pthread_cond_signal(&buffer.not_full); // Send a signal that the buffer is not full

    pthread_mutex_unlock(&buffer.mutex); // Unlock
    
    return expr;

}

void add_expression(Expression expr) {
    pthread_mutex_lock(&buffer.mutex); // Lock buffer

    while (buffer.count >= BUFFER_SIZE) { // If the buffer is full
        pthread_cond_wait(&buffer.not_full, &buffer.mutex); //Wait for the not full signal
    }

    buffer.expressions[buffer.in] = expr; // Add expression to the array
    buffer.in = (buffer.in + 1) % BUFFER_SIZE; // Increment the 'in' index 
    buffer.count++; // Increment the amount of expressions in buffer

    pthread_cond_signal(&buffer.not_empty); // Send a signal that the buffer is not empty

    pthread_mutex_unlock(&buffer.mutex); // Unlock
}

//Makes a random expression
Expression make_expression(int prod) { 
    Expression expr;
    expr.operand1 = (rand() % 50) + 1; //set to be a random number from 1-50
    expr.operand2 = (rand() % 50) + 1;
    expr.operator = rand_op();
    expr.result = result_expression(expr);
    expr.producer_id = prod; // Will be set by the producer
    return expr;
}

//Gives the answer to an expression
float result_expression(Expression expr) {
    switch (expr.operator) { 
        case '+':
            return expr.operand1 + expr.operand2;
        case '-':
            return expr.operand1 - expr.operand2;
        case '*':
            return expr.operand1 * expr.operand2;
        case '/':
            return (float) expr.operand1 / (float) expr.operand2;
        default:
            printf("Error: Bad operator\n");
            return 0;
    }
}

// Choose a random operator for the expression
char rand_op() {
    char operators[] = {'+', '-', '*', '/'};
    int index = rand() % 4;
    return operators[index];
}

// -- Buffer methods --

void initialize_buffer(Buffer *buffer) {
    buffer->count = 0; // Init everything
    buffer->in = 0;
    buffer->out = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
}

void cleanup_buffer(Buffer *buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);
}
