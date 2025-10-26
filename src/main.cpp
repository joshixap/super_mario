#include <cstring>
#include <curses.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 25

typedef struct GameObject {
	float x, y;
	float width, height;
	float vert_speed;
	bool is_fly;
	char type;
	float horiz_speed;
} GameObject;

GameObject *g_moving = NULL;
int g_moving_count;
int g_level = 1;
int g_score;
int g_max_level;

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]);
void show_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]);
void set_object_pos(GameObject *obj, float x_pos, float y_pos);
void init_object(GameObject *obj, float x_pos, float y_pos, float width, float height, char type);
void player_dead(GameObject *mario, GameObject **bricks, int *brick_count);
void vert_move_object(GameObject *obj, GameObject *mario, GameObject *bricks, int brick_count);
void delete_moving(int index);
void mario_collision(GameObject *mario, GameObject *bricks, int brick_count);
void horizon_move_object(GameObject *obj, GameObject *bricks, int brick_count);
bool is_pos_in_map(int x, int y);
void set_cursor(int x, int y);
void create_level(int level, GameObject *mario, GameObject **bricks, int *brick_count);
void horizon_move_map(float dx, GameObject *mario, GameObject *bricks, int brick_count);
bool is_collision(GameObject o1, GameObject o2);
void put_object_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1], GameObject obj);
void put_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]);
GameObject *get_new_brick(GameObject **bricks, int *brick_count);
GameObject *get_new_moving();

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]) {
	for (int i = 0; i < MAP_WIDTH; i++) {
		map[0][i] = ' ';
	}
	map[0][MAP_WIDTH] = '\0';

	for (int j = 1; j < MAP_HEIGHT; j++) {
		sprintf(map[j], map[0]);
	}
}

void show_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]) {
	map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = '\0';

	for (int j = 0; j < MAP_HEIGHT; j++) {
		printw("%s", map[j]);
	}

	refresh();
}

void set_object_pos(GameObject *obj, float x_pos, float y_pos) {
	obj->x = x_pos;
	obj->y = y_pos;
}

void init_object(GameObject *obj, float x_pos, float y_pos, float width, float height, char type) {
	set_object_pos(obj, x_pos, y_pos);
	obj->width = width;
	obj->height = height;
	obj->vert_speed = 0;
	obj->is_fly = false;
	obj->type = type;
	obj->horiz_speed = 0.2;
}

void player_dead(GameObject *mario, GameObject **bricks, int *brick_count) {
	napms(500);
	create_level(g_level, mario, bricks, brick_count);
}

void vert_move_object(GameObject *obj, GameObject *mario, GameObject *bricks, int brick_count) {
	obj->is_fly = true;
	obj->vert_speed += 0.05;
	set_object_pos(obj, obj->x, obj->y + obj->vert_speed);

	for (int i = 0; i < brick_count; i++) {
		if (is_collision(*obj, bricks[i])) {
			if (obj->vert_speed > 0) {
				obj->is_fly = false;
			}

			if ((bricks[i].type == '?') && (obj->vert_speed < 0) && (obj == mario)) {
				bricks[i].type = '-';
				init_object(get_new_moving(), bricks[i].x, bricks[i].y - 3, 3, 2, '$');
				g_moving[g_moving_count - 1].vert_speed = -0.7;
			}

			obj->y = obj->y - obj->vert_speed;
			obj->vert_speed = 0;

			if (bricks[i].type == '+') {
				g_level++;
				if (g_level > g_max_level) {
					g_level = 1;
				}
				napms(500);
				create_level(g_level, mario, &bricks, &brick_count);
			}
			break;
		}
	}
}

void delete_moving(int index) {
	g_moving_count--;
	g_moving[index] = g_moving[g_moving_count];
	g_moving = (GameObject*) realloc(g_moving, sizeof(*g_moving) * g_moving_count);
}

void mario_collision(GameObject *mario, GameObject *bricks, int brick_count) {
	for (int i = 0; i < g_moving_count; i++) {
		if (is_collision(*mario, g_moving[i])) {
			if (g_moving[i].type == 'o') {
				if ((mario->is_fly == true) && (mario->vert_speed > 0) && (mario->y + mario->height < g_moving[i].y + g_moving[i].height * 0.5)) {
					g_score += 50;
					delete_moving(i);
					i--;
					continue;
				} else {
					player_dead(mario, &bricks, &brick_count);
				}
			}
			if (g_moving[i].type == '$') {
				g_score += 100;
				delete_moving(i);
				i--;
				continue;
			}
		}
	}
}

void horizon_move_object(GameObject *obj, GameObject *bricks, int brick_count) {
	obj[0].x += obj[0].horiz_speed;

	for (int i = 0; i < brick_count; i++) {
		if (is_collision(obj[0], bricks[i])) {
			obj[0].x -= obj[0].horiz_speed;
			obj[0].horiz_speed = -obj[0].horiz_speed;
			return;
		}
	}

	if (obj[0].type == 'o') {
		GameObject tmp = *obj;
		vert_move_object(&tmp, NULL, bricks, brick_count);

		if (tmp.is_fly == true) {
			obj[0].x -= obj[0].horiz_speed;
			obj[0].horiz_speed = -obj[0].horiz_speed;
		}
	}
}

bool is_pos_in_map(int x, int y) {
	return ((x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

void put_object_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1], GameObject obj) {
	int ix = (int)round(obj.x);
	int iy = (int)round(obj.y);
	int iwidth = (int)round(obj.width);
	int iheight = (int)round(obj.height);

	for (int i = ix; i < (ix + iwidth); i++) {
		for (int j = iy; j < (iy + iheight); j++) {
			if (is_pos_in_map(i, j)) {
				map[j][i] = obj.type;
			}
		}
	}
}

void set_cursor(int x, int y) {
	move(y, x);
}

void horizon_move_map(float dx, GameObject *mario, GameObject *bricks, int brick_count) {
	mario->x -= dx;

	for (int i = 0; i < brick_count; i++) {
		if (is_collision(*mario, bricks[i])) {
			mario->x += dx;
			return;
		}
	}
	mario->x += dx;

	for (int i = 0; i < brick_count; i++) {
		bricks[i].x += dx;
	}

	for (int i = 0; i < g_moving_count; i++) {
		g_moving[i].x += dx;
	}
}

bool is_collision(GameObject o1, GameObject o2) {
	return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) &&
		((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

GameObject *get_new_brick(GameObject **bricks, int *brick_count) {
	(*brick_count)++;
	*bricks = (GameObject*) realloc(*bricks, sizeof(**bricks) * (*brick_count));
	return (*bricks) + (*brick_count) - 1;
}

GameObject *get_new_moving() {
	g_moving_count++;
	g_moving = (GameObject*) realloc(g_moving, sizeof(*g_moving) * g_moving_count);
	return g_moving + g_moving_count - 1;
}

void put_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]) {
	char score_text[30];
	sprintf(score_text, "Score: %d", g_score);
	int len = strlen(score_text);

	for (int i = 0; i < len; i++) {
		map[1][i + 5] = score_text[i];
	}
}

void create_level(int level, GameObject *mario, GameObject **bricks, int *brick_count) {
	*brick_count = 0;
	*bricks = (GameObject*) realloc(*bricks, 0);
	g_moving_count = 0;
	g_moving = (GameObject*) realloc(g_moving, 0);

	init_object(mario, 39, 10, 3, 3, '@');
	g_score = 0;

	if (level == 1) {
		init_object(get_new_brick(bricks, brick_count), 20, 20, 40, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 30, 10, 5, 3, '?');
		init_object(get_new_brick(bricks, brick_count), 50, 10, 5, 3, '?');
		init_object(get_new_brick(bricks, brick_count), 60, 15, 40, 10, '#');
		init_object(get_new_brick(bricks, brick_count), 60, 5, 10, 3, '-');
		init_object(get_new_brick(bricks, brick_count), 70, 5, 5, 3, '?');
		init_object(get_new_brick(bricks, brick_count), 75, 5, 5, 3, '-');
		init_object(get_new_brick(bricks, brick_count), 80, 5, 5, 3, '?');
		init_object(get_new_brick(bricks, brick_count), 85, 5, 10, 3, '-');
		init_object(get_new_brick(bricks, brick_count), 100, 20, 20, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 120, 15, 10, 10, '#');
		init_object(get_new_brick(bricks, brick_count), 150, 20, 40, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 210, 15, 10, 10, '+');

		init_object(get_new_moving(), 25, 10, 3, 2, 'o');
		init_object(get_new_moving(), 80, 10, 3, 2, 'o');
	}

	if (level == 2) {
		init_object(get_new_brick(bricks, brick_count), 20, 20, 40, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 60, 15, 10, 10, '#');
		init_object(get_new_brick(bricks, brick_count), 80, 20, 20, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 120, 15, 10, 10, '#');
		init_object(get_new_brick(bricks, brick_count), 150, 20, 40, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 210, 15, 10, 10, '+');

		init_object(get_new_moving(), 25, 10, 3, 2, 'o');
		init_object(get_new_moving(), 80, 10, 3, 2, 'o');
		init_object(get_new_moving(), 65, 10, 3, 2, 'o');
		init_object(get_new_moving(), 120, 10, 3, 2, 'o');
		init_object(get_new_moving(), 160, 10, 3, 2, 'o');
		init_object(get_new_moving(), 175, 10, 3, 2, 'o');
	}

	if (level == 3) {
		init_object(get_new_brick(bricks, brick_count), 20, 20, 40, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 80, 20, 15, 5, '#');
		init_object(get_new_brick(bricks, brick_count), 120, 15, 15, 10, '#');
		init_object(get_new_brick(bricks, brick_count), 160, 10, 15, 15, '+');

		init_object(get_new_moving(), 25, 10, 3, 2, 'o');
		init_object(get_new_moving(), 50, 10, 3, 2, 'o');
		init_object(get_new_moving(), 80, 10, 3, 2, 'o');
		init_object(get_new_moving(), 90, 10, 3, 2, 'o');
		init_object(get_new_moving(), 120, 10, 3, 2, 'o');
		init_object(get_new_moving(), 130, 10, 3, 2, 'o');
	}

	g_max_level = 3;
}

int main() {
	int rows, columns;
	int key = 0;
	char map[MAP_HEIGHT][MAP_WIDTH + 1];
	GameObject mario;
	GameObject *bricks = NULL;
	int brick_count = 0;

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	getmaxyx(stdscr, rows, columns);

	create_level(g_level, &mario, &bricks, &brick_count);

	do {
		clear_map(map);

		if ((mario.is_fly == false) && (key == ' ')) {
			mario.vert_speed = -1;
		}
		if (key == 'd' || key == 'D') {
			horizon_move_map(1, &mario, bricks, brick_count);
		}
		if (key == 'a' || key == 'A') {
			horizon_move_map(-1, &mario, bricks, brick_count);
		}

		if (mario.y > MAP_HEIGHT) {
			player_dead(&mario, &bricks, &brick_count);
		}

		vert_move_object(&mario, &mario, bricks, brick_count);
		mario_collision(&mario, bricks, brick_count);

		for (int i = 0; i < brick_count; i++) {
			put_object_on_map(map, bricks[i]);
		}
		for (int i = 0; i < g_moving_count; i++) {
			vert_move_object(g_moving + i, &mario, bricks, brick_count);
			horizon_move_object(g_moving + i, bricks, brick_count);
			if (g_moving[i].y > MAP_HEIGHT) {
				delete_moving(i);
				i--;
				continue;
			}
			put_object_on_map(map, g_moving[i]);
		}

		put_object_on_map(map, mario);
		put_score_on_map(map);

		move(0, 0);
		show_map(map);

		napms(20);
		key = getch();
	} while (key != 27); // 27 = ESC

	clear_map(map);
	put_object_on_map(map, mario);
	show_map(map);
	napms(500);
	endwin();

	// освобождение памяти bricks
	free(bricks);

	return 0;
}