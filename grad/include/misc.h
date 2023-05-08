/* Find "length" of a two-dimensional array */
#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

#define for_each_enemy(i) for (i = 0; i < sizeof(enemy) / sizeof(int); i++)

#define LOAD -1
#define PLAY 0
#define WIN 1
#define LOSE 2
