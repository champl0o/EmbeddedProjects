#include "safe.h"
#include "actuators.h"
#include "display.h"
#include <stdio.h>
#include <stdbool.h>

static const int code[CODE_LEN] = {1, 2, 3, 4};
static int entered[CODE_LEN] = {0};

static int pos = 0;
static int digit = 0;
static int tries_used = 0;
static int dir = 0;

static safe_state_t current_state = ENTER;

static void reset_code(void);

void safe_init(void)
{
    current_state = ENTER;
    tries_used = 0;
    buzzer_init();

    new_attempt();
}
static void check_code(void)
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
            display_status("OPEN", COLOR_OK);
            buzzer_play_melody();
            printf("  -> ВІДКРИТО\n");
            printf("Натисни кнопку, щоб зачинити.\n");
            fflush(stdout);
        }
        else
        {
            printf("  -> НЕВІРНО\n");
            display_status("WRONG CODE", COLOR_ERR);
            vTaskDelay(pdMS_TO_TICKS(1000));
            reset_code();
        }
    }
}

static void reset_code(void)
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

void new_attempt(void)
{
    if (current_state != LOCKED)
    {
        dir = 0;
        pos = 0;
        digit = 0;

        for (int i = 0; i < CODE_LEN; i++)
        {
            entered[i] = 0;
        }

        printf("Спроба %d/%d. Код: ", tries_used + 1, RETRIES);
        display_attempt(tries_used + 1, RETRIES);
    }
    else
    {
        printf("Спроби вичерпано. Сейф заблоковано до перезавантаження.\n");
        display_locked();
    }
    fflush(stdout);
}

static void show_digit(bool replace)
{
    if (replace)
    {
        printf("\b");
    }
    printf("%d", digit);
    fflush(stdout);
    display_digit(pos, digit);
}

void on_tick(int tick_dir)
{
    if (current_state != ENTER)
        return;

    if (dir == 0)
    {
        dir = tick_dir;
        digit = 0;
        show_digit(false);

        return;
    }
    if (tick_dir == dir)
    {
        digit = (digit + 1) % 10;
        show_digit(true);

        return;
    }

    entered[pos++] = digit;
    display_confirm(pos - 1, entered[pos - 1]);

    if (pos == CODE_LEN)
    {
        check_code();
        return;
    }
    dir = tick_dir;
    digit = 0;
    printf(" ");
    show_digit(false);
}

void on_button(void)
{
    if (current_state == OPEN)
    {
        printf("Сейф зачинено.\n");
        tries_used = 0;
        current_state = ENTER;
        new_attempt();
        return;
    }
    if (current_state == LOCKED)
    {
        return;
    }
    printf(" -> RESET\n");
    reset_code();
}
