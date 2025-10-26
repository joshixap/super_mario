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

char g_map[MAP_HEIGHT][MAP_WIDTH + 1];
GameObject g_mario;
GameObject *g_bricks = NULL;
int g_brick_count;
GameObject *g_moving = NULL;
int g_moving_count;
int g_level = 1;
int g_score;
int g_max_level;

void clear_map();
void show_map();
void set_object_pos(GameObject *obj, float x_pos, float y_pos);
void init_object(GameObject *obj, float x_pos, float y_pos, float width, float height, char type);
void player_dead();
void vert_move_object(GameObject *obj);
void delete_moving(int index);
void mario_collision();
void horizon_move_object(GameObject *obj);
bool is_pos_in_map(int x, int y);
void set_cursor(int x, int y);
void create_level(int level);
void horizon_move_map(float dx);
bool is_collision(GameObject o1, GameObject o2);
void put_object_on_map(GameObject obj);
void put_score_on_map();
GameObject *get_new_brick();
GameObject *get_new_moving();



void clear_map() {
	for (int i = 0; i < MAP_WIDTH; i++) {
		g_map[0][i] = ' ';
	}
	g_map[0][MAP_WIDTH] = '\0';

	for (int j = 1; j < MAP_HEIGHT; j++) {
		sprintf(g_map[j], g_map[0]);
	}
}

void show_map() {
	g_map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = '\0';

	for (int j = 0; j < MAP_HEIGHT; j++) {
		printw("%s", g_map[j]);
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

void player_dead() {
	napms(500);
	create_level(g_level);
}

void vert_move_object(GameObject *obj) {
	obj->is_fly = true;
	obj->vert_speed += 0.05;
	set_object_pos(obj, obj->x, obj->y + obj->vert_speed);

	for (int i = 0; i < g_brick_count; i++) {
		if (is_collision(*obj, g_bricks[i])) {
			if (obj->vert_speed > 0) {
				obj->is_fly = false;
			}

			if ((g_bricks[i].type == '?') && (obj->vert_speed < 0) && (obj == &g_mario)) {
				g_bricks[i].type = '-';
				init_object(get_new_moving(), g_bricks[i].x, g_bricks[i].y - 3, 3, 2, '$');
				g_moving[g_moving_count - 1].vert_speed = -0.7;
			}

			obj->y = obj->y - obj->vert_speed;
			obj->vert_speed = 0;

			if (g_bricks[i].type == '+') {
				g_level++;
				if (g_level > g_max_level) {
					g_level = 1;
				}
				napms(500);
				create_level(g_level);
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

void mario_collision() {
	for (int i = 0; i < g_moving_count; i++) {
		if (is_collision(g_mario, g_moving[i])) {
			if (g_moving[i].type == 'o') {
				if ((g_mario.is_fly == true) && (g_mario.vert_speed > 0) && (g_mario.y + g_mario.height < g_moving[i].y + g_moving[i].height * 0.5)) {
					g_score += 50;
					delete_moving(i);
					i--;
					continue;
				} else {
					player_dead();
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

void horizon_move_object(GameObject *obj) {
	obj[0].x += obj[0].horiz_speed;

	for (int i = 0; i < g_brick_count; i++) {
		if (is_collision(obj[0], g_bricks[i])) {
			obj[0].x -= obj[0].horiz_speed;
			obj[0].horiz_speed = -obj[0].horiz_speed;
			return;
		}
	}

	if (obj[0].type == 'o') {
		GameObject tmp = *obj;
		vert_move_object(&tmp);

		if (tmp.is_fly == true) {
			obj[0].x -= obj[0].horiz_speed;
			obj[0].horiz_speed = -obj[0].horiz_speed;
		}
	}
}

bool is_pos_in_map(int x, int y) {
	return ((x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

void put_object_on_map(GameObject obj) {
	int ix = (int)round(obj.x);
	int iy = (int)round(obj.y);
	int iwidth = (int)round(obj.width);
	int iheight = (int)round(obj.height);

	for (int i = ix; i < (ix + iwidth); i++) {
		for (int j = iy; j < (iy + iheight); j++) {
			if (is_pos_in_map(i, j)) {
				g_map[j][i] = obj.type;
			}
		}
	}
}

void set_cursor(int x, int y) {
	move(y, x);
}

void horizon_move_map(float dx) {
	g_mario.x -= dx;

	for (int i = 0; i < g_brick_count; i++) {
		if (is_collision(g_mario, g_bricks[i])) {
			g_mario.x += dx;
			return;
		}
	}
	g_mario.x += dx;

	for (int i = 0; i < g_brick_count; i++) {
		g_bricks[i].x += dx;
	}

	for (int i = 0; i < g_moving_count; i++) {
		g_moving[i].x += dx;
	}
}

bool is_collision(GameObject o1, GameObject o2) {
	return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) &&
		((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

GameObject *get_new_brick() {
	g_brick_count++;
	g_bricks = (GameObject*) realloc(g_bricks, sizeof(*g_bricks) * g_brick_count);
	return g_bricks + g_brick_count - 1;
}

GameObject *get_new_moving() {
	g_moving_count++;
	g_moving = (GameObject*) realloc(g_moving, sizeof(*g_moving) * g_moving_count);
	return g_moving + g_moving_count - 1;
}

void put_score_on_map() {
	char score_text[30];
	sprintf(score_text, "Score: %d", g_score);
	int len = strlen(score_text);

	for (int i = 0; i < len; i++) {
		g_map[1][i + 5] = score_text[i];
	}
}

void create_level(int level) {
	g_brick_count = 0;
	g_bricks = (GameObject*) realloc(g_bricks, 0);
	g_moving_count = 0;
	g_moving = (GameObject*) realloc(g_moving, 0);

	init_object(&g_mario, 39, 10, 3, 3, '@');
	g_score = 0;

	if (level == 1) {
		init_object(get_new_brick(), 20, 20, 40, 5, '#');
		init_object(get_new_brick(), 30, 10, 5, 3, '?');
		init_object(get_new_brick(), 50, 10, 5, 3, '?');
		init_object(get_new_brick(), 60, 15, 40, 10, '#');
		init_object(get_new_brick(), 60, 5, 10, 3, '-');
		init_object(get_new_brick(), 70, 5, 5, 3, '?');
		init_object(get_new_brick(), 75, 5, 5, 3, '-');
		init_object(get_new_brick(), 80, 5, 5, 3, '?');
		init_object(get_new_brick(), 85, 5, 10, 3, '-');
		init_object(get_new_brick(), 100, 20, 20, 5, '#');
		init_object(get_new_brick(), 120, 15, 10, 10, '#');
		init_object(get_new_brick(), 150, 20, 40, 5, '#');
		init_object(get_new_brick(), 210, 15, 10, 10, '+');

		init_object(get_new_moving(), 25, 10, 3, 2, 'o');
		init_object(get_new_moving(), 80, 10, 3, 2, 'o');
	}

	if (level == 2) {
		init_object(get_new_brick(), 20, 20, 40, 5, '#');
		init_object(get_new_brick(), 60, 15, 10, 10, '#');
		init_object(get_new_brick(), 80, 20, 20, 5, '#');
		init_object(get_new_brick(), 120, 15, 10, 10, '#');
		init_object(get_new_brick(), 150, 20, 40, 5, '#');
		init_object(get_new_brick(), 210, 15, 10, 10, '+');

		init_object(get_new_moving(), 25, 10, 3, 2, 'o');
		init_object(get_new_moving(), 80, 10, 3, 2, 'o');
		init_object(get_new_moving(), 65, 10, 3, 2, 'o');
		init_object(get_new_moving(), 120, 10, 3, 2, 'o');
		init_object(get_new_moving(), 160, 10, 3, 2, 'o');
		init_object(get_new_moving(), 175, 10, 3, 2, 'o');
	}

	if (level == 3) {
		init_object(get_new_brick(), 20, 20, 40, 5, '#');
		init_object(get_new_brick(), 80, 20, 15, 5, '#');
		init_object(get_new_brick(), 120, 15, 15, 10, '#');
		init_object(get_new_brick(), 160, 10, 15, 15, '+');

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

	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	getmaxyx(stdscr, rows, columns);

	create_level(g_level);

	do {
		clear_map();

		if ((g_mario.is_fly == false) && (key == ' ')) {
			g_mario.vert_speed = -1;
		}
		if (key == 'd' || key == 'D') {
			horizon_move_map(1);
		}
		if (key == 'a' || key == 'A') {
			horizon_move_map(-1);
		}

		if (g_mario.y > MAP_HEIGHT) {
			player_dead();
		}

		vert_move_object(&g_mario);
		mario_collision();

		for (int i = 0; i < g_brick_count; i++) {
			put_object_on_map(g_bricks[i]);
		}
		for (int i = 0; i < g_moving_count; i++) {
			vert_move_object(g_moving + i);
			horizon_move_object(g_moving + i);
			if (g_moving[i].y > MAP_HEIGHT) {
				delete_moving(i);
				i--;
				continue;
			}
			put_object_on_map(g_moving[i]);
		}

		put_object_on_map(g_mario);
		put_score_on_map();

		move(0, 0);
		show_map();

		napms(20);
		key = getch();
	} while (key != 27); // 27 = ESC

	clear_map();
	put_object_on_map(g_mario);
	show_map();
	napms(500);
	endwin();
	return 0;
}