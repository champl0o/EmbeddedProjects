#include "safe.h"
#include "actuators.h"
#include <stdio.h>
#include <stdbool.h>

static const int code[CODE_LEN] = {1, 2, 3, 4};
static int entered[CODE_LEN] = {0};
static int current_index = 0;

int pos = 0;
int digit = 0;
int tries_used = 0;
int dir = 0;

safe_state_t current_state = ENTER;

void safe_init()
{
    current_state = ENTER;
    tries_used = 0;
    buzzer_init();

    new_attempt();
}
static void check_code()
{
    if (current_state == ENTER && pos == CODE_LEN)
    {
        bool correct = true;

        for (int i = 0; i < CODE_LEN; i++)
        {
            if (entered[i] != code[i])
            {
                correct = false;
            }
        }

        if (correct)
        {
            current_state = OPEN;
        }
        else
        {
            reset_code();
        }
    }
}

static void reset_code()
{
    if (tries_used + 1 < RETRIES)
    {
        tries_used++;
    }
    else
    {
        current_state = LOCKED;
    }

    new_attempt();
}

void new_attempt()
{
    if (current_state != LOCKED)
    {
        dir = 0;
        pos = 0;
        digit = 0;
        current_index = 0;

        for (int i = 0; i < CODE_LEN; i++)
        {
            entered[i] = 0;
        }

        printf("Attempt: %d\n", tries_used);
    }
    else
    {
        printf("Safe is locked. No new attempts allowed.\n");
    }
    fflush(stdout);
}
