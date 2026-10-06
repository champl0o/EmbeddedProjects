#ifndef SAFE_H
#define SAFE_H

#define RETRIES 3
#define CODE_LEN 4

typedef enum
{
    ENTER,
    OPEN,
    LOCKED
} safe_state_t;

void safe_init(void);
void new_attempt(void);
void on_tick(int tick_dir);
void on_button(void);

#endif // SAFE_H
