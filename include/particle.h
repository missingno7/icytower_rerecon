#ifndef ICYTOWER_PARTICLE_H
#define ICYTOWER_PARTICLE_H
#include <allegro.h>

typedef struct {
    int intensity;
    fixed x, y;
    fixed sx, sy;
    int color;
} Tparticle;

int create_particle(Tparticle *p, int x, int y);
void update_particle(Tparticle *p);
void reset_particles(Tparticle *p);
#endif
