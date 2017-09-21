#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "circ.h"

// Population / Circuit size
#define NUMBER_ORGANISMS        150
#define NUMBER_GENES            60
#define ALLELES                 5

#define MAXIMUM_FITNESS 130
#define FALSE   0
#define TRUE    1

// Crossover settings 
// 0. One-point, 1. Uniform
#define XO_NO_DISRUPT 0
#define XO_MODE 0

// Logging
#define LOG_RATE 5

#define ANNEALING_PERIOD 5
#define ANNEALING_COOLDOWN 100

#define NUMBER_TESTS 26

// Global variables
unsigned char **currentgeneration, **nextgeneration;
unsigned char *modelorganism;

int *organismsfitnesses;

int total_fitness;

float mutation_rate         = 0.001;
int annealing_cooldown      = 0;
int annealing_period        = 0;

int global_best = 0;

int *in_arr, *out_arr;

short inputarrs[NUMBER_TESTS][5] = {};
short outputarrs[NUMBER_TESTS][5] = {};

// function declarations
void allocate_memory(void);
int run(void);
void initialize_organisms(void);
int evaluate_organisms(int);
void produce_next_generation(void);
int select_organism(void);


int main(){
    in_arr = read_file("in.txt", 16, 26);
    out_arr = read_file("out.txt", 16, 26);

    int generations;
    int i, j;

     // insert input into arrays
    for(i = 0; i < NUMBER_TESTS; i++) {
        for(j = 4; j >= 0; j--) {
            inputarrs[i][j] = (in_arr[i] >> j) & 1;
            outputarrs[i][j] = (out_arr[i] >> j) & 1;
        }
    }

    allocate_memory();
    generations = run(); //Number of generations taken
    printf("the final generation was: %d\n", generations);
}


void allocate_memory(void){
    int organism;

    currentgeneration = (unsigned char**) malloc(sizeof(unsigned char*) * NUMBER_ORGANISMS);
    nextgeneration = (unsigned char**) malloc(sizeof(unsigned char*) * NUMBER_ORGANISMS);
    modelorganism = (unsigned char*) malloc(sizeof(unsigned char) * NUMBER_GENES);
    organismsfitnesses = (int*) malloc(sizeof(int) * NUMBER_ORGANISMS);

    for(organism=0; organism<NUMBER_ORGANISMS; ++organism){
        currentgeneration[organism] = (unsigned char*)malloc(sizeof(unsigned char) * NUMBER_GENES);
        nextgeneration[organism] = (unsigned char*)malloc(sizeof(unsigned char) * NUMBER_GENES);
    }
}


int run(void){
    int generations = 1;
    int is_done = FALSE;

    initialize_organisms();

    while(TRUE){
        is_done = evaluate_organisms(generations);
        if( is_done == TRUE ) return generations;
        produce_next_generation();
        generations++;
    }
}


void initialize_organisms(void){
    int organism;
    int gene;

    // initialize the normal organisms
    for(organism=0; organism<NUMBER_ORGANISMS; organism++){
        for(gene=0; gene<NUMBER_GENES; gene++){
            currentgeneration[organism][gene] = rand()%ALLELES;
        }
    }
}


int evaluate_organisms(int generation){
    int organism;
    int gene;

    int b_fitness = 0;
    float average_fitness;

    int letter_count, input1, input2, success_count;
    int location;

    unsigned char val[5];

    total_fitness = 0;

    int organism_fitness;
    for(organism=0; organism<NUMBER_ORGANISMS; organism++){
        // calculate fitness
        success_count = 0;

        for(letter_count= 0; letter_count< NUMBER_TESTS; letter_count++) {
            //initialize 0th layer values
            val[0] = inputarrs[letter_count][0];
            val[1] = inputarrs[letter_count][1];
            val[2] = inputarrs[letter_count][2];
            val[3] = inputarrs[letter_count][3];
            val[4] = inputarrs[letter_count][4];

            for(gene=0; gene<NUMBER_GENES; gene=gene+3) {
                 //if first layer
                input1 = val[currentgeneration[organism][gene]];
                input2 = val[currentgeneration[organism][gene+1]];

                location = (gene % 15) / 3;

                switch(currentgeneration[organism][gene+2]) {
                    case 0: val[location] = input1; break;
                    case 1: val[location] = ~input1; break;
                    case 2: val[location] = input1 | input2; break;
                    case 3: val[location] = input1 & input2; break;
                    case 4: val[location] = input1 ^ input2; break;
                }
            }
            for(int i = 0; i < 5; i++) {
                if(val[i] == outputarrs[letter_count][i]) {
                    success_count++;
                }
            }
        }

        organism_fitness = success_count;

        if( organism_fitness > b_fitness)
            b_fitness = organism_fitness;

        // save the tally in the fitnesses data structure
        // and add its fitness to the generation's total
        organismsfitnesses[organism] = organism_fitness;
        total_fitness += organism_fitness;
        average_fitness = (float) total_fitness / NUMBER_ORGANISMS;

        // check if we have a perfect generation
        if( organism_fitness == MAXIMUM_FITNESS){
            return TRUE;
        }
    }
    if(annealing_cooldown){
        mutation_rate = 0.001;
        annealing_cooldown--;
    } else if (annealing_period) {
        mutation_rate = 0.08;
        annealing_period--;
    } else {
        if(b_fitness - average_fitness <= 2) {
            annealing_cooldown = ANNEALING_COOLDOWN;
            annealing_period = ANNEALING_PERIOD;
        } else {
            mutation_rate = 0.001;
        }    
    }
    if (b_fitness > global_best ) {
        global_best = b_fitness;
    }

   if(generation % LOG_RATE == 0) {
        printf("generation %10d: record = %4d, best = %4d, average = %10.3f, mutation= %5.3f\n", generation, global_best, b_fitness, average_fitness, mutation_rate);
    }
    return FALSE;
}


void produce_next_generation(){
    int organism;
    int gene;
    int crossover_point;
    int do_mutation;
    int parent_one, parent_two;

    // fill the nextgeneration data structure with the children
    for(organism=0; organism<NUMBER_ORGANISMS; organism++) { 
        parent_one = select_organism();
        parent_two = select_organism();

        crossover_point = rand() % NUMBER_GENES;

        for(gene=0; gene<NUMBER_GENES; ++gene) {
            do_mutation = rand() % (int)(1.0 / mutation_rate);

            if(do_mutation == 0){
                nextgeneration[organism][gene] = rand() % ALLELES;
            } else if(XO_MODE == 0) { // One-point crossover
                if(XO_NO_DISRUPT) crossover_point = crossover_point - (crossover_point % 3);

                if (gene < crossover_point){
                    nextgeneration[organism][gene] = currentgeneration[parent_one][gene];
                } else {
                    nextgeneration[organism][gene] = currentgeneration[parent_two][gene];
                }
            } else if(XO_MODE == 1) { // Uniform crossover
                if(rand() % 2) {
                    nextgeneration[organism][gene] = currentgeneration[parent_one][gene];
                } else {
                    nextgeneration[organism][gene] = currentgeneration[parent_two][gene];
                }
            }
        }
    }
    // copy the children in nextgeneration into currentgeneration
    for(organism=0; organism<NUMBER_ORGANISMS; organism++){
        for(gene=0; gene<NUMBER_GENES; gene++){
            currentgeneration[organism][gene] = nextgeneration[organism][gene];
        }
    }
}

// Fitness proportionate selection
int select_organism(void){
    int organism;
    int runningTotal;
    int randomSelectPoint;

    runningTotal = 0;
    randomSelectPoint = rand() % (total_fitness + 1);

    for(organism=0; organism<NUMBER_ORGANISMS; organism++){
        runningTotal += organismsfitnesses[organism];
        if(runningTotal >= randomSelectPoint) return organism;
    }
    return 0;
}
