/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct square {
	int x;
	int y;
	int val[9];
	int count;
	struct square *next;
};

unsigned int board[9][9] = {
	{1, 0, 0, 0, 0, 5, 0, 0, 3},
	{7, 4, 0, 8, 3, 0, 2, 1, 0},
	{9, 3, 0, 1, 0, 0, 6, 0, 0},
	{8, 0, 0, 0, 0, 0, 0, 0, 7},
	{2, 0, 0, 0, 0, 0, 0, 0, 4},
	{5, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 1, 0, 0, 2, 0, 3, 0},
	{6, 0, 9, 0, 0, 4, 0, 5, 2},
	{0, 0, 0, 7, 0, 0, 0, 0, 0},
};

void print_board()
{
	int i, j;

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			printf("%d ", board[i][j]);
		}
		printf("\n");
	}
}

void compute_options(struct square *sq)
{
	int vals[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	int i, j, val;
	int min_x = 0, min_y = 0;
	int max_x = 2, max_y = 2;
	int mod_x, mod_y;
	int start_x, start_y, end_x, end_y;

	/* Check in row */
	for (i = 0; i < 9; i++) {
		val = board[sq->y][i];
		if (val)
			vals[val - 1] = 0;
	}

	/* Check in col */
	for (i = 0; i < 9; i++) {
		val = board[i][sq->x];
		if (val)
			vals[val - 1] = 0;
	}

	/* Check in subgrid */
	mod_x = sq->x % 3;
	mod_y = sq->y % 3;

	start_x = sq->x - mod_x;
	start_y = sq->y - mod_y;

	end_x = sq->x + (max_x - mod_x);
	end_y = sq->y + (max_y - mod_y);

	for (i = start_y; i <= end_y; i++)
		for (j = start_x; j <= end_x; j++) {
			val = board[i][j];
			if (val)
				vals[val - 1] = 0;
		}

	for (i = 0; i < 9; i++) {
		if (vals[i])
			sq->val[sq->count++] = vals[i];
	}
}

/*
 * Insertion sorting in ascending order of sq->count.
 * Keeping it this way means we have lesser options to try
 * initially and for each right choice, the resulting
 * set of options for other squares reduces drastically.
 * Search space prunning.
 */
void insert_square_sorted(struct square **head, struct square *sq)
{
	struct square *trav = *head;
	struct square *prev;
	struct square *tmp;

	if (!(*head)) {
		*head = sq;
		trav = *head;
		trav->next = NULL;
	} else {
		while (trav) {
			if (trav->count <= sq->count) {
				prev = trav;
				trav = trav->next;
			} else
				break;
		}

		/* Sq is the last node. */
		if (!trav) {
			prev->next = sq;
			sq->next = NULL;
		} else {
			/*
			 * Insert node in place of trav
			 * and shift the list.
			 */
			tmp = malloc(sizeof(*tmp));
			memcpy(tmp, trav, sizeof(*trav));
			memset(trav, 0, sizeof(*trav));
			memcpy(trav, sq, sizeof(*trav));
			trav->next = tmp;
			free(sq);
		}
	}
}

void create_empty_sq_options(struct square **head)
{
	int i,j;
	struct square *sq;

	printf("Creating options for empty squares\n");
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			if (board[i][j] == 0) {
				sq = (struct square *) malloc(sizeof(*sq));
				if (!sq) {
					printf("Out of mem\n");
					return;
				}

				sq->x = j;
				sq->y = i;
				sq->count = 0;
				/* Compute possible options for sq->val */
				compute_options(sq);
				insert_square_sorted(head, sq);
			}
		}
	}
}

void print_options(struct square *head)
{
	int i;
	struct square *trav;

	printf("Printing options list\n");

	trav = head;
	while (trav) {
		printf("(%d, %d)", trav->x, trav->y);
		printf("options:[cnt: %d]", trav->count);
		for (i = 0; i < trav->count; i++)
			printf(" %d", trav->val[i]);
		trav = trav->next;
		printf("\n");
	}
}

/* Check if val exists in row + col of the board */
int check_row_col(int x, int y, int val)
{
	int i, j;

	/* Check in col */
	for (i = 0; i < 9; i++) {
		if (val == board[i][x])
			return -1;
	}

	/* Check in row */
	for (i = 0; i < 9; i++) {
		if (val == board[y][i])
			return -1;
	}
	return 0;
}

void solve_board()
{
	struct square *trav, *tmp, *prev;
	struct square *head = NULL;
	int ret, val, i;

	printf("Solving board\n");
	print_board();
	create_empty_sq_options(&head);
	print_options(head);

	/* Start from head and pick one sq at a time */
	trav = head;
	while (trav) {
		if (!trav->count) {
			/*
			 * Del the whole list of options.
			 * If the first sq has 0 options, then no point
			 * in looking further.
			 */
			tmp = head;
			prev = tmp;
			while (tmp) {
				tmp = tmp->next;
				free(prev);
				prev = tmp;
			}
			return;
		} else if (trav->count == 1) {
			/* If this is only one option for this square, try it. */
			val = trav->val[0];
			ret = check_row_col(trav->x, trav->y, val);
			if (!ret) {
				/* It works. So, move to the next empty square options. */
				board[trav->y][trav->x] = val;
				trav = trav->next;
			} else {
				/* Failed. Time to roll back choices. */
				printf("Board has err (%d, %d): val=%d\n", trav->x, trav->y, val);
				/* Undo the choices made thus far from *head. */
				tmp = head;
				while (tmp != trav) {
					board[tmp->y][tmp->x] = 0;
					tmp = tmp->next;
				}
				return;
			}
		} else {
			/* Try all the possible options for this square, until one fails above. */
			for (i = 0; i < trav->count; i++) {
				val = trav->val[i];
				printf("Checking: (%d, %d) : %d, i:%d, count:%d\n", trav->x, trav->y, val, i, trav->count);
				ret = check_row_col(trav->x, trav->y, val);
				if (!ret) {
					board[trav->y][trav->x] = val;
					printf("Trying: (%d, %d) : %d, i:%d, count:%d\n", trav->x, trav->y, val, i, trav->count);
					solve_board();
					/* One of these choices failed. So backtrack the board and try the next choice. */
					board[trav->y][trav->x] = 0;
					printf("backtracking: (%d, %d) : %d, i:%d, count:%d \n", trav->x, trav->y, val, i, trav->count);
					print_board();
				}
			}

			/* Tried all possible choices for this square. None worked, so roll back all choices until *trav */
			tmp = head;
			while (tmp != trav) {
				printf("Rolling back: (%d, %d)\n", tmp->x, tmp->y);
				board[tmp->y][tmp->x] = 0;
				tmp = tmp->next;
			}
			return;
		}
	}

	/* All empty squares had only one option which worked! */
	printf("Done!\n");
	print_board();
	exit(0);
}

int main(void)
{
	print_board();
	solve_board();
	printf("Failed to solve. :(\n");
	print_board();
	return 1;
}
