#ifndef CONFIG_H
#define CONFIG_H

#define RETRIES 3
#define CODE_LEN 4

typedef enum
{
    ENTER,
    OPEN,
    LOCKED
} safe_state_t;

void safe_init();
void new_attempt();

#endif // CONFIG_H
