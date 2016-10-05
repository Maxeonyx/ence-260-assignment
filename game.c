#include "system.h"
#include "tinygl.h"
#include "task.h"
#include "../../fonts/font5x7_1.h"

static void display_task (__unused__ void *data) {
    tinygl_update ();
}


int main (void)
{
    system_init ();

    tinygl_init (250);
    tinygl_font_set (&font5x7_1);
    tinygl_text_speed_set (2);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);

    tinygl_text ("hi world");

    task_t tasks[] =
    {
      {.func = display_task, .period = TASK_RATE / 250}
    };

    task_schedule (tasks, ARRAY_SIZE (tasks));

}
