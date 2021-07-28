//
// Created by Rafid on 10/25/17.
//


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define STEPS_AB 100
#define STEPS_BC 5
#define STEPS_CD 150
#define STEPS_DD 300
#define EMPTY 0
#define FULL 1

int *speeds, *ids;
pthread_mutex_t testingBC;
pthread_mutex_t testingCB;
pthread_mutex_t testingBridge;
pthread_mutex_t mutex_BC_steps[STEPS_BC + 1];
pthread_mutex_t mutex_CB_steps[STEPS_BC + 1];
pthread_mutex_t mutex_CD_steps[STEPS_CD + 1];
int BC_states[STEPS_BC + 1];
int CB_states[STEPS_BC + 1];
int CD_states[STEPS_CD + 1];
pthread_mutex_t mutex_cd_wait_count;
int waiting_at_D = 0, towards_cd = 0;

int check_ahead_CD(int current_step, int steps_remaining, int speed) {
    int empty_step_ahead = 0;
    int i, available_steps;
    if(steps_remaining < speed) available_steps = steps_remaining;
    else available_steps = speed;
    for(i = current_step + 1; i <= current_step + available_steps; ++i) {
        if(CD_states[i] == EMPTY) {
            if(i == current_step + 1) pthread_mutex_unlock(&mutex_CD_steps[i]);
            empty_step_ahead++;
        } else {
            return empty_step_ahead;
        }
    }
    return empty_step_ahead;
}

int check_ahead_BC(int current_step, int steps_remaining, int speed) {
    int empty_step_ahead = 0;
    int i, available_steps;
    if(steps_remaining < speed) available_steps = steps_remaining;
    else available_steps = speed;
    for(i = current_step + 1; i <= current_step + available_steps; ++i) {
        if(BC_states[i] == EMPTY) {
            if(i == current_step + 1) pthread_mutex_unlock(&mutex_BC_steps[i]);
            empty_step_ahead++;
        } else {
            return empty_step_ahead;
        }
    }
    return empty_step_ahead;
}


int check_ahead_DC(int current_step, int steps_remaining, int speed) {
    int empty_step_ahead = 0;
    int i, available_steps;
    if(steps_remaining < speed) available_steps = steps_remaining;
    else available_steps = speed;
    for(i = current_step - 1; i >= current_step - available_steps; --i) {
        if(CD_states[i] == EMPTY) {
            if(i == current_step - 1) pthread_mutex_unlock(&mutex_CD_steps[i]);
            empty_step_ahead++;
        } else {
            return empty_step_ahead;
        }
    }
    return empty_step_ahead;
}


int check_ahead_CB(int current_step, int steps_remaining, int speed) {
    int empty_step_ahead = 0;
    int i, available_steps;
    if(steps_remaining < speed) available_steps = steps_remaining;
    else available_steps = speed;
    for(i = current_step + 1; i <= current_step + available_steps; ++i) {
        if(CB_states[i] == EMPTY) {
            if(i == current_step + 1) pthread_mutex_unlock(&mutex_CB_steps[i]);
            empty_step_ahead++;
        } else {
            return empty_step_ahead;
        }
    }
    return empty_step_ahead;
}



void walk_AB(int person) {
    int steps_remaining = STEPS_AB;
    int current_step = 0;
    int previous_step = 0;
    while(steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        if(steps_remaining < speeds[person]) {
            current_step = previous_step + steps_remaining;
            steps_remaining = 0;
        }
        else {
            current_step = previous_step + speeds[person];
            steps_remaining = STEPS_AB - current_step;
        }
        printf("Person %d moved along AB from %d to %d\n", person + 1, previous_step, current_step);
    }
}

void walk_BC(int person) {
    int steps_remaining = STEPS_BC;
    int current_step = 0;
    int previous_step = 0;
    int empty_steps_ahead;
    while(steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        pthread_mutex_lock(&mutex_BC_steps[current_step + 1]);
        pthread_mutex_lock(&testingBC);
        empty_steps_ahead = check_ahead_BC(current_step, steps_remaining, speeds[person]);
        BC_states[previous_step] = EMPTY;
        pthread_mutex_unlock(&mutex_BC_steps[previous_step]);
        BC_states[current_step += empty_steps_ahead] = FULL;
        pthread_mutex_lock(&mutex_BC_steps[current_step]);
        printf("Person %d moved along BC from %d to %d\n", person + 1, previous_step, current_step);
        pthread_mutex_unlock(&testingBC);
        steps_remaining -= empty_steps_ahead;
    }
}

void walk_CD(int person) {
    int steps_remaining = STEPS_CD;
    int current_step = 0;
    int previous_step = 0;
    int empty_steps_ahead;
    while (steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        if(current_step == 0) {
            pthread_mutex_lock(&mutex_cd_wait_count);
            if(waiting_at_D > 0) {
                pthread_mutex_unlock(&mutex_cd_wait_count);
                continue;
            }
            else {
                pthread_mutex_lock(&mutex_CD_steps[current_step + 1]);
                pthread_mutex_lock(&testingBridge);
                empty_steps_ahead = check_ahead_CD(current_step, steps_remaining, speeds[person]);
                CD_states[previous_step] = EMPTY;
                current_step += empty_steps_ahead;
                CD_states[current_step] = FULL;
                pthread_mutex_unlock(&mutex_CD_steps[previous_step]);
                pthread_mutex_unlock(&mutex_CD_steps[current_step]);
                pthread_mutex_lock(&testingBC);
                BC_states[STEPS_BC] = EMPTY;
                pthread_mutex_unlock(&mutex_BC_steps[STEPS_BC]);
                pthread_mutex_unlock(&testingBC);
                printf("Person %d moved along CD from %d to %d\n", person + 1, previous_step, current_step);
                pthread_mutex_unlock(&testingBridge);
                towards_cd++;
                pthread_mutex_unlock(&mutex_cd_wait_count);
                steps_remaining -= empty_steps_ahead;
            }
        }
        else {
            pthread_mutex_lock(&mutex_CD_steps[current_step + 1]);
            pthread_mutex_lock(&testingBridge);
            empty_steps_ahead = check_ahead_CD(current_step, steps_remaining, speeds[person]);
            CD_states[previous_step] = EMPTY;
            current_step += empty_steps_ahead;
            CD_states[current_step] = FULL;
            pthread_mutex_unlock(&mutex_CD_steps[previous_step]);
            pthread_mutex_unlock(&mutex_CD_steps[current_step]);
            printf("Person %d moved along CD from %d to %d\n", person + 1, previous_step, current_step);
            pthread_mutex_unlock(&testingBridge);
            steps_remaining -= empty_steps_ahead;
        }
    }
}

void walk_DD(int person) {
    int steps_remaining = STEPS_DD;
    int current_step = 0;
    int previous_step = 0;
    pthread_mutex_lock(&mutex_cd_wait_count);
    towards_cd--;
    pthread_mutex_unlock(&mutex_cd_wait_count);
    pthread_mutex_lock(&testingBridge);
    CD_states[STEPS_CD] = EMPTY;
    pthread_mutex_unlock(&mutex_CD_steps[STEPS_CD]);
    pthread_mutex_unlock(&testingBridge);
    while(steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        if(steps_remaining < speeds[person]) {
            current_step = previous_step + steps_remaining;
            steps_remaining = 0;
        }
        else {
            current_step = previous_step + speeds[person];
            steps_remaining = STEPS_DD - current_step;
        }
        printf("Person %d moved along DD from %d to %d\n", person + 1, previous_step, current_step);
    }
}

void walk_DC(int person) {
    int steps_remaining = STEPS_CD;
    int current_step = STEPS_CD;
    int previous_step = STEPS_CD;
    int empty_steps_ahead;
    pthread_mutex_lock(&mutex_cd_wait_count);
    waiting_at_D++;
    pthread_mutex_unlock(&mutex_cd_wait_count);
    while (steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        if(current_step == STEPS_CD) {
            pthread_mutex_lock(&mutex_cd_wait_count);
            if(towards_cd > 0) {
                pthread_mutex_unlock(&mutex_cd_wait_count);
                continue;
            }
            else {
                pthread_mutex_lock(&mutex_CD_steps[current_step - 1]);
                pthread_mutex_lock(&testingBridge);
                empty_steps_ahead = check_ahead_DC(current_step, steps_remaining, speeds[person]);
                CD_states[previous_step] = EMPTY;
                current_step -= empty_steps_ahead;
                CD_states[current_step] = FULL;
                pthread_mutex_unlock(&mutex_CD_steps[previous_step]);
                pthread_mutex_lock(&mutex_CD_steps[current_step]);
                printf("Person %d moved along DC from %d to %d\n", person + 1, previous_step, current_step);
                pthread_mutex_unlock(&testingBridge);
                pthread_mutex_unlock(&mutex_cd_wait_count);
                steps_remaining -= empty_steps_ahead;
            }
        } else{
            pthread_mutex_lock(&mutex_CD_steps[current_step - 1]);
            pthread_mutex_lock(&testingBridge);
            empty_steps_ahead = check_ahead_DC(current_step, steps_remaining, speeds[person]);
            CD_states[previous_step] = EMPTY;
            current_step -= empty_steps_ahead;
            CD_states[current_step] = FULL;
            pthread_mutex_unlock(&mutex_CD_steps[previous_step]);
            pthread_mutex_lock(&mutex_CD_steps[current_step]);
            printf("Person %d moved along DC from %d to %d\n", person + 1, previous_step, current_step);
            pthread_mutex_unlock(&testingBridge);
            steps_remaining -= empty_steps_ahead;
        }
    }
}

void walk_CB(int person) {
    int steps_remaining = STEPS_BC;
    int current_step = 0;
    int previous_step = 0;
    int empty_steps_ahead;
    while(steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        pthread_mutex_lock(&mutex_CB_steps[current_step + 1]);
        pthread_mutex_lock(&testingCB);
        empty_steps_ahead = check_ahead_CB(current_step, steps_remaining, speeds[person]);
        if(current_step == 0) {
            pthread_mutex_lock(&mutex_cd_wait_count);
            waiting_at_D--;
            pthread_mutex_unlock(&mutex_cd_wait_count);
            pthread_mutex_lock(&testingBridge);
            CD_states[0] = EMPTY;
            pthread_mutex_unlock(&mutex_CD_steps[0]);
            pthread_mutex_unlock(&testingBridge);
        }
        CB_states[previous_step] = EMPTY;
        pthread_mutex_unlock(&mutex_CB_steps[previous_step]);
        CB_states[current_step += empty_steps_ahead] = FULL;
        pthread_mutex_lock(&mutex_CB_steps[current_step]);
        printf("Person %d moved along BC from %d to %d\n", person + 1, STEPS_BC -
                previous_step, STEPS_BC - current_step);
        pthread_mutex_unlock(&testingCB);
        steps_remaining -= empty_steps_ahead;
    }
}

void walk_BA(int person) {
    int steps_remaining = STEPS_AB;
    int current_step = STEPS_AB;
    int previous_step = STEPS_AB;
    pthread_mutex_lock(&testingCB);
    CB_states[STEPS_BC] = EMPTY;
    pthread_mutex_unlock(&mutex_CB_steps[STEPS_BC]);
    pthread_mutex_unlock(&testingCB);
    while(steps_remaining > 0) {
        sleep(1);
        previous_step = current_step;
        if(steps_remaining < speeds[person]) {
            current_step = previous_step - steps_remaining;
            steps_remaining = 0;
        }
        else {
            current_step = previous_step - speeds[person];
            steps_remaining = current_step;
        }
        printf("Person %d moved along BA from %d to %d\n", person + 1, previous_step, current_step);
    }
}







void* walk_thread(void *person) {
    int *NID = (int*)person;
    walk_AB(*NID);
    walk_BC(*NID);
    walk_CD(*NID);
    walk_DD(*NID);
    walk_DC(*NID);
    walk_CB(*NID);
    walk_BA(*NID);
    printf("Person %d has completed his walk\n", (*NID) + 1);
}



int main() {
    FILE *fin;
    int N, i, res;
    pthread_t *threads;
    int *thread_result;
    if((fin = fopen("in.txt", "r")) == NULL) {
        printf("Cannot open file.\n");
        exit(1);
    }
    fscanf(fin, "%d", &N);
    speeds = (int*)malloc(N* sizeof(int));
    threads = (pthread_t*)malloc(N * sizeof(pthread_t));
    ids = (int*)malloc(N* sizeof(int));
    for(i = 0; i < N; ++i) {
        fscanf(fin, "%d", &speeds[i]);
    }
    for(i = 1; i <= STEPS_BC; ++i) {
        res = pthread_mutex_init(&mutex_BC_steps[i], NULL);
        if (res != 0) {
            perror("Mutex initialization failed");
            exit(EXIT_FAILURE);
        }
        res = pthread_mutex_init(&mutex_CB_steps[i], NULL);
        if (res != 0) {
            perror("Mutex initialization failed");
            exit(EXIT_FAILURE);
        }
    }
    for(i = 1; i <= STEPS_CD; ++i) {
        res = pthread_mutex_init(&mutex_CD_steps[i], NULL);
        if (res != 0) {
            perror("Mutex initialization failed");
            exit(EXIT_FAILURE);
        }
    }
    res = pthread_mutex_init(&mutex_cd_wait_count, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_mutex_init(&testingBC, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_mutex_init(&testingCB, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_mutex_init(&testingBridge, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < N; ++i) {
        ids[i] = i;
        res = pthread_create(&threads[i], NULL, walk_thread, (void*)&ids[i]);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    for(i = 0; i < N; ++i) {
        res = pthread_join(threads[i], (void**)&thread_result);
        if (res != 0) {
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
    }
    free(speeds);
    free(ids);
    free(threads);
    return 0;
}
