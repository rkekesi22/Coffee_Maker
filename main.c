#include <stdio.h>
#include <string.h>

typedef enum
{
    STATE_IDLE,
    STATE_VERIFY,
    STATE_PAY,
    STATE_DISPENSE,
    STATE_REPORT,
    STATE_OFF
} State;

typedef enum
{
    EVENT_NONE,
    EVENT_SELECT_ESPRESSO,
    EVENT_SELECT_LATTE,
    EVENT_SELECT_CAPPUCCINO,
    EVENT_REPORT,
    EVENT_PAYMENT_DONE,
    EVENT_PAYMENT_FAILED,
    EVENT_INTERRUPT,
    EVENT_POWER_OFF
} Event;

typedef struct
{
    int water;
    int coffee;
    int milk;
    float price_cents;
} Recipe;

typedef struct
{
    State state;
    Event event;

    int water;
    int coffee;
    int milk;
    float money_cents;

    const Recipe *selected;
} CoffeeMachine;

typedef struct
{
    float quarters;
    float dimes;
    float nickles;
    float pennies;
} Coins_t;

const Recipe ESPRESSO = {50, 18, 0, 1.50};
const Recipe LATTE = {200, 24, 150, 2.50};
const Recipe CAPPU = {250, 24, 100, 3.00};

void clear_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

void fsm_step(CoffeeMachine *cm, Coins_t *coins)
{
    switch (cm->state)
    {
    case STATE_IDLE:
        switch (cm->event)
        {
        case EVENT_SELECT_ESPRESSO:
            cm->selected = &ESPRESSO;
            cm->state = STATE_VERIFY;
            break;

        case EVENT_SELECT_LATTE:
            cm->selected = &LATTE;
            cm->state = STATE_VERIFY;
            break;

        case EVENT_SELECT_CAPPUCCINO:
            cm->selected = &CAPPU;
            cm->state = STATE_VERIFY;
            break;

        case EVENT_REPORT:
            cm->state = STATE_REPORT;
            break;

        case EVENT_POWER_OFF:
            cm->state = STATE_OFF;
            break;

        default:
            break;
        }
        break;

        // --------------------------------------

    case STATE_REPORT:
        printf("Water: %dml\n", cm->water);
        printf("Milk: %dml\n", cm->milk);
        printf("Coffee: %dg\n", cm->coffee);
        printf("Money: $%.2f\n", cm->money_cents);

        cm->state = STATE_IDLE;
        break;

        // --------------------------------------

    case STATE_VERIFY:
        if (cm->water < cm->selected->water ||
            cm->milk < cm->selected->milk ||
            cm->coffee < cm->selected->coffee)
        {
            printf("Not enough ingredients.\n");
            cm->state = STATE_IDLE;
        }
        else
        {
            cm->state = STATE_PAY;
        }
        break;

        // --------------------------------------

    case STATE_PAY:
    {
        float quarters, dimes, nickles, pennies;
        printf("Insert coins (q d n p): ");
        scanf("%f %f %f %f", &quarters, &dimes, &nickles, &pennies);

        clear_stdin();

        float sum = quarters * coins->quarters + dimes * coins->dimes + nickles * coins->nickles + pennies * coins->pennies;

        printf("Here is $%.2f in change.\n", sum);

        if (sum >= cm->selected->price_cents)
        {
            cm->money_cents += cm->selected->price_cents;
            cm->state = STATE_DISPENSE;
        }
        else
        {
            printf("Not enough money. Refunded.\n");
            cm->state = STATE_IDLE;
        }
        break;
    }

        // --------------------------------------

    case STATE_DISPENSE:
        printf("Here is your coffee. Enjoy!\n");

        cm->water -= cm->selected->water;
        cm->milk -= cm->selected->milk;
        cm->coffee -= cm->selected->coffee;

        cm->state = STATE_IDLE;

        break;

        // --------------------------------------

    case STATE_OFF:
        printf("Machine off.\n");
        break;
    }

    cm->event = EVENT_NONE;
}

Event read_event(void)
{
    char buf[32];
    printf("What would you like? (espresso/latte/cappuccino) ");
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = 0;

    if (strcmp(buf, "espresso") == 0)
        return EVENT_SELECT_ESPRESSO;
    if (strcmp(buf, "latte") == 0)
        return EVENT_SELECT_LATTE;
    if (strcmp(buf, "cappuccino") == 0)
        return EVENT_SELECT_CAPPUCCINO;
    if (strcmp(buf, "report") == 0)
        return EVENT_REPORT;
    if (!strcmp(buf, "off") == 0)
        return EVENT_POWER_OFF;

    return EVENT_NONE;
}

int main(void)
{
    CoffeeMachine cm = {
        .state = STATE_IDLE,
        .event = EVENT_NONE,
        .water = 300,
        .milk = 200,
        .coffee = 100,
        .money_cents = 0,
        .selected = NULL};

    Coins_t coins =
        {
            .quarters = 0.25,
            .dimes = 0.10,
            .nickles = 0.05,
            .pennies = 0.01};

    while (cm.state != STATE_OFF)
    {
        switch (cm.state)
        {
        case STATE_IDLE:
            cm.event = read_event();
            break;

        default:
            cm.event = EVENT_NONE;
            break;
        }

        fsm_step(&cm, &coins);
    }

    return 0;
}
