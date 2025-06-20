#include <stdio.h>
#include "generic_list.h"

typedef struct {
    char letter;
    int x;
    int y;
} Bob;

Bob *new_bob(char letter, int x, int y);
int is_even(const Bob *bob, const void *args);
void print_bob(Bob *bob, void *args);
void make_odd(Bob *bob, void *args);

main()
{
    Generic_queue bobqueue;
    Generic_list even_ones;
    Bob *bob;
    char c;
    int i, j;

    initialize_queue(&bobqueue);

    for (c='a', i=1, j=1; i<9; c++, i++, j+=2) {
        printf("Creating and queueing Bob %c) %d, %d.\n", c, i, j);
        enqueue(bobqueue, new_bob(c, i, j));
    }

    even_ones = all_such_that(bobqueue,
                              (int(*)(const void*,const void*))is_even, NULL);

    printf("\nThese are the even Bobs:\n\n");
    perform_on_list(even_ones, (void(*)(void*,void*))print_bob, NULL);

    perform_on_list(even_ones, (void(*)(void*,void*))make_odd, NULL);

    printf("\nNow they are odd:\n\n");
    perform_on_list(even_ones, (void(*)(void*,void*))print_bob, NULL);

    printf("\nThe last Bob in this list is:\n\n");
    print_bob(peek_at_end(even_ones), NULL);

    destroy_list(&even_ones);

    printf("\nNow let's empty our original queue.\n");
    printf("The appropriate Bobs have been changed here as well.\n\n");

    while (bob = dequeue(bobqueue)) {
        print_bob(bob, NULL);
        free(bob);
    }

    destroy_queue(&bobqueue);
}

/****************************************************************************/

Bob *
new_bob(char letter, int x, int y)
{
    Bob *bob;

    bob = (Bob *) malloc(sizeof(Bob));
    if (!bob) {
        fprintf(stderr, "Error allocating memory for a Bob.\n");
        exit(EXIT_FAILURE);
    }

    bob->letter = letter;
    bob->x = x;
    bob->y = y;

    return bob;
}

/****************************************************************************/

int
is_even(const Bob *bob, const void *args)
{
    if ((bob->x + bob->y)%2 == 0)
        return 1;  /* TRUE, this Bob is even. */
    else
        return 0;  /* FALSE, this Bob is odd. */
}

/****************************************************************************/

void
print_bob(Bob *bob, void *args)
{
    printf("%c) %d + %d = %d\n", bob->letter, bob->x, bob->y, bob->x + bob->y);
}

/****************************************************************************/

void
make_odd(Bob *bob, void *args)
{
    bob->y++;
}

/****************************************************************************/
