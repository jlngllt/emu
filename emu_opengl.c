#include "emu.h"

#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

static int make_resources(void)
{
   return 1;
}

static void update_fade_factor(void)
{
}

void emu_process_input(int *k)
{
   *k = *k;
}


void emu_print_gfx(uint8_t *gfx, int32_t x0, int32_t y0)
{
}

static void render(void)
{
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
   glutSwapBuffers();
}

int32_t emu_init_gfx(void)
{

   int argc = 1;
   char *argv[1] = {(char*)"Something"};
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   glutInitWindowSize(400, 300);
   glutCreateWindow("Hello World");
   glutDisplayFunc(&render);
   glutIdleFunc(&update_fade_factor);
   glewInit();

   if (!GLEW_VERSION_2_0) {
      fprintf(stderr, "OpenGL 2.0 not available\n");
      return 1;
   }
   if (!make_resources()) {
      fprintf(stderr, "Failed to load resources\n");
      return 1;
   }

   return 0;
}

void emu_mainloop(st_emu *emu)
{
   glutMainLoop();
}

void emu_free_gfx()
{
}
