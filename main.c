#include "Includes.h"

typedef struct{
    uint scale;
    Length len;
    bool **cell;
    bool **next;
}Gen;

int imin(const int a, const int b)
{
    return a < b ? a : b;
}

int imax(const int a, const int b)
{
    return a > b ? a : b;
}

uint scale(const Length len)
{
    const Length win = getWindowLen();
    return imin(win.x/len.x, win.y/len.y);
}

void genFree(Gen gen)
{
    if(!gen.cell)
        return;
    for(uint x = 0; x < gen.len.x; x++)
        free(gen.cell[x]);
    free(gen.cell);
}

Gen genInit(const Length len)
{
    Gen ret = {
        .len = len,
        .cell = calloc(len.x, sizeof(bool*)),
        .next = calloc(len.x, sizeof(bool*)),
        .scale = scale(len)
    };
    for(uint x = 0; x < len.x; x++){
        ret.cell[x] = calloc(len.y, sizeof(bool));
        for(uint y = 0; y < len.y; y++)
            ret.cell[x][y] = rand() & 1;
        ret.next[x] = calloc(len.y, sizeof(bool));
    }

    return ret;
}

int iwrap(const int n, const int min, const int max)
{
    if(n < min)
        return max-1;
    if(n >= max)
        return min;
    return n;
}

bool valid(const Coord pos, const Length len)
{
    return pos.x < len.x && pos.y < len.y && pos.x >= 0 && pos.y >= 0;
}

uint adj(const Gen gen, const Coord pos)
{
    uint total = 0;
    for(Direction d = 0; d < 4; d++){
        Coord card = coordShift(pos, d, 1);
        card.x = iwrap(card.x, 0, gen.len.x);
        card.y = iwrap(card.y, 0, gen.len.y);
        Coord diag = coordShift(card, dirROR(d), 1);
        diag.x = iwrap(diag.x, 0, gen.len.x);
        diag.y = iwrap(diag.y, 0, gen.len.y);
        if(!valid(card, gen.len)){
            fprintf(stderr, "card:(%2i,%2i) invalid\n", card.x, card.y);
            exit(EXIT_FAILURE);
        }
        if(!valid(diag, gen.len)){
            fprintf(stderr, "diag:(%2i,%2i) invalid\n", diag.x, diag.y);
            exit(EXIT_FAILURE);
        }
        total += gen.cell[card.x][card.y];
        total += gen.cell[diag.x][diag.y];
    }
    return total;
}

Gen nextGen(const Gen cur)
{
    for(uint y = 0; y < cur.len.y; y++){
        for(uint x = 0; x < cur.len.x; x++){
            const uint neighbors = adj(cur, (const Coord){.x = x, .y = y});
            if(cur.cell[x][y] && (neighbors == 2 || neighbors == 3))
                cur.next[x][y] = true;
            else if(!cur.cell[x][y] && neighbors == 3)
                cur.next[x][y] = true;
            else
                cur.next[x][y] = false;
        }
    }
    for(uint x = 0; x < cur.len.x; x++)
        memcpy(cur.cell[x], cur.next[x], sizeof(bool) * cur.len.y);

    return cur;
}

void drawGen(const Gen cur)
{
    for(uint y = 0; y < cur.len.y; y++){
        for(uint x = 0; x < cur.len.x; x++){
            const Coord pos = coordMul((const Coord){.x=x,.y=y}, cur.scale);
            setColor(BLACK);
            fillSquareCoord(pos, cur.scale);
            if(cur.cell[x][y])
                setColor(RED);
            else
                setColor(GREY);
            fillSquareCoordResize(pos, cur.scale, -2);
        }
    }
}

Gen onClick(Gen gen, const Coord clickPos)
{
    const Coord pos = coordDiv(clickPos, gen.scale);
    if(!valid(pos, gen.len))
        return gen;
    gen.cell[pos.x][pos.y] = !gen.cell[pos.x][pos.y];
    return gen;
}

int main(void)
{
    Length window = {800, 600};
    Length len = {32, 24};
    init();
    setWindowLen(window);
    Gen gen = genInit(len);
    printf("scale: %u\n", gen.scale);
    uint delay = 250;
    uint next = getTicks();
    bool pause = false;
    while(1){
        Ticks t = frameStart();

        const Length newwin = getWindowLen();
        if(!coordSame(window, newwin)){
            window = newwin;
            gen.scale = scale(gen.len);
        }

        if(keyPressed(SDL_SCANCODE_ESCAPE)){
            genFree(gen);
            return 0;
        }
        if(keyPressed(SDL_SCANCODE_SPACE)){
            pause = !pause;
        }
        if(mouseBtnPressed(MOUSE_L)){
            gen = onClick(gen, mouse.pos);
        }
        if(keyPressed(SDL_SCANCODE_DOWN)){
            delay -= delay/4;
            delay = delay < 1 ? 1 : delay;
        }
        if(keyPressed(SDL_SCANCODE_UP)){
            delay += delay/4;
            delay = delay >= 2000 ? 2000 : delay;
        }
        if(keyPressed(SDL_SCANCODE_RETURN))
            next = 0;
        if((!pause || keyPressed(SDL_SCANCODE_RETURN)) && getTicks() > next){
            gen = nextGen(gen);
            next = getTicks()+delay;
        }
        drawGen(gen);

        frameEnd(t);
    }
    return 0;
}
