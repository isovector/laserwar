#include <allegro.h>
#include <math.h>
#include <iostream>

#define SHIPS 2
#define POWER_GAIN 3
#define IF_TILE(tile) if (abs(map[where.y][where.x]) == tile)
#define WIDTH 52
#define HEIGHT 39

typedef struct { int x, y; } Vector2;

typedef struct { int x1, x2, y1, y2; } Matrix;

typedef struct { Vector2 pos, dir; int color; } Ship;

Vector2 transform(Matrix a, Vector2 d) {
    return (Vector2) { a.x1 * d.x + a.x2 * d.y, a.y1 * d.x + a.y2 * d.y };
}

int clamp(int x, int low, int high) {
    return MAX(MIN(x, high), low);
}

Vector2 clamp(Vector2 d, int low, int high) {
    return (Vector2) { clamp(d.x, low, high), clamp(d.y, low, high) };
}

char tileset[] = {'.', '#', '/', '\\', '=', 'H', '+', '%', '-', '|', '*'};
int map[HEIGHT][WIDTH] = {0};
Vector2 pos = {0};

Vector2 DIR_W = {-1, 0}, DIR_E = {1, 0}, DIR_N = {0, -1}, 
        DIR_S = {0, 1}, DIR_NW = {-1, -1}, DIR_NE = {1, -1}, 
        DIR_SW = {-1, 1}, DIR_SE = {1, 1};

Ship ships[2];

inline int veccmp(Vector2 a, Vector2 b) {
    return a.x == b.x && a.y == b.y;
}

void laser(BITMAP *buffer, int color, Vector2 where, Vector2 dir, int power) {
    Vector2 from;

    while (power > 0) {
        from = where;
        where.x += dir.x;
        where.y += dir.y;

        if (!(where.x < WIDTH && where.y < HEIGHT && where.x >= 0 && where.y >= 0))
            break;

        IF_TILE(1)
            power = 0;
        else IF_TILE(2) {
            if (veccmp(dir, DIR_NE) || veccmp(dir, DIR_SW)) {
                power = 0;
                goto after;
            }
            dir = clamp(transform((Matrix) {0, -1, -1, 0}, dir), -1, 1);
            power += POWER_GAIN;
        } else IF_TILE(3) {
            if (veccmp(dir, DIR_NW) || veccmp(dir, DIR_SE)) {
                power = 0;
                goto after;
            }
            dir = clamp(transform((Matrix) {0, 1, 1, 0}, dir), -1, 1);
            power += POWER_GAIN;
        } else IF_TILE(4) {
            if (!veccmp(dir, DIR_W) && !veccmp(dir, DIR_E))
                power -= POWER_GAIN;
        } else IF_TILE(5) {
            if (!veccmp(dir, DIR_N) && !veccmp(dir, DIR_S))
                power -= POWER_GAIN;
        } else IF_TILE(6) {
            // double laser all the way
        } else IF_TILE(7) {
            power += 2 * POWER_GAIN;
        } else IF_TILE(8) {
            if (veccmp(dir, DIR_W) || veccmp(dir, DIR_E)) {
                power = 0;
                goto after;
            }
            dir = clamp(transform((Matrix) {1, 0, 0, -1}, dir), -1, 1);
            power += POWER_GAIN;
        } else IF_TILE(9) {
            if (veccmp(dir, DIR_N) || veccmp(dir, DIR_S)) {
                power = 0;
                goto after;
            }
            dir = clamp(transform((Matrix) {-1, 0, 0, 1}, dir), -1, 1);
            power += POWER_GAIN;
        } else IF_TILE(10) {
            dir = clamp(transform((Matrix) {1, -1, 1, 1}, dir), -1, 1);
            power += POWER_GAIN;
        }

    after:
        line(buffer, from.x * 12 + 4, from.y * 12 + 3, where.x * 12 + 4, where.y * 12 + 3, color);
        power--;
    }
}

int main(int argc, char *argv[]) {
    allegro_init();
    install_keyboard();
    install_timer();
    install_mouse();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
    BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
    enable_hardware_cursor();
    show_mouse(screen);

    ships = {{{5, 5}, DIR_S, makecol(255, 0, 0)}, {{24, 24}, DIR_W, makecol(0, 255, 0)}};

    do {
        clear_bitmap(buffer);

        if (key[KEY_LEFT]) pos.x--;
        else if (key[KEY_UP]) pos.y--;
        else if (key[KEY_RIGHT]) pos.x++;
        else if (key[KEY_DOWN]) pos.y++;
        else if (key[KEY_SPACE]) map[pos.y][pos.x] = 0;
        else if (key[KEY_R]) map[pos.y][pos.x] = 1;
        else if (key[KEY_Q] || key[KEY_C]) map[pos.y][pos.x] = 2;
        else if (key[KEY_E] || key[KEY_Z]) map[pos.y][pos.x] = 3;
        else if (key[KEY_V]) map[pos.y][pos.x] = 4;
        else if (key[KEY_B]) map[pos.y][pos.x] = 5;
        else if (key[KEY_F]) map[pos.y][pos.x] = 7;
        else if (key[KEY_G]) map[pos.y][pos.x] = 6;
        else if (key[KEY_W] || key[KEY_X]) map[pos.y][pos.x] = 8;
        else if (key[KEY_A] || key[KEY_D]) map[pos.y][pos.x] = 9;
        else if (key[KEY_S]) map[pos.y][pos.x] = 10;
        else if (key[KEY_1]) ships[0].pos = pos;
        else if (key[KEY_2]) ships[1].pos = pos;

        pos.x = clamp(pos.x, 0, WIDTH - 1);
        pos.y = clamp(pos.y, 0, HEIGHT - 1);

        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)

                textprintf_ex(buffer, font, x * 12, y * 12, makecol(0, 0, 255), -1, "%c", tileset[map[y][x]]);

        for (int i = 0; i < SHIPS; i++) {
            Ship s = ships[i];
            char pic[] = {0};
            if (veccmp(s.dir, DIR_W)) pic[0] = '<';
            else if (veccmp(s.dir, DIR_S)) pic[0] ='v';
            else if (veccmp(s.dir, DIR_E)) pic[0] = '>';
            else if (veccmp(s.dir, DIR_N)) pic[0] = '^';
            
            textout_ex(buffer, font, pic, s.pos.x * 12, s.pos.y * 12, s.color, 0);
            laser(buffer, s.color, s.pos, s.dir, 5);
        }

        textout_ex(buffer, font, "*", pos.x * 12, pos.y * 12, makecol(255, 0, 255), 0);

        blit(buffer, screen, 0, 0, 0, 0, 640, 480);

        readkey();
    } while (!key[KEY_ESC]);
    return 0;
}
END_OF_MAIN()
