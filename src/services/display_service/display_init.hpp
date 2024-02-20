#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl.h"
#include <SDL2/SDL.h>
#include "../libraries/lv_drivers/display/monitor.h"
#include "../libraries/lv_drivers/indev/mouse.h"
//#include "../libraries/lv_drivers/display/fbdev.h"
//#include "../libraries/lv_drivers/indev/evdev.h"

#include "../config/config.h"
#include "kiosk.hpp"

namespace display{
    /**
     * A task to measure the elapsed time for LVGL
     * @param data unused
     * @return never return
     */
    static int tick_thread(void *data) {
    (void)data;

    while(1) {
        SDL_Delay(5);
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
    }

    int lvgl_init(){

        lv_init();
        monitor_init();
        SDL_CreateThread(tick_thread, "tick", NULL);

        //fbdev_init();

        //evdev_init();

        /*A small buffer for LittlevGL to draw the screen's content*/
        static lv_color_t buf_1[DISP_BUF_SIZE];
        static lv_color_t buf_2[DISP_BUF_SIZE];

        /*Initialize a descriptor for the buffer*/
        static lv_disp_draw_buf_t disp_buf;
        lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, DISP_BUF_SIZE);

        /*Initialize and register a display driver*/
        /*Create a display*/
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv); /*Basic initialization*/
        disp_drv.draw_buf = &disp_buf;
        disp_drv.flush_cb = monitor_flush;
        disp_drv.hor_res = MONITOR_HOR_RES;
        disp_drv.ver_res = MONITOR_VER_RES;
        disp_drv.antialiasing = 1;
        lv_disp_t* disp = lv_disp_drv_register(&disp_drv);

        mouse_init();
        static lv_indev_drv_t indev_drv_1;
        lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
        indev_drv_1.type = LV_INDEV_TYPE_POINTER;

        /*This function will be called periodically (by the library) to get the mouse position and state*/
        indev_drv_1.read_cb = mouse_read;
        lv_indev_t* mouse_indev = lv_indev_drv_register(&indev_drv_1);

        /*
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.draw_buf   = &disp_buf;
        disp_drv.flush_cb   = fbdev_flush;
        disp_drv.hor_res    = HOR_RES;
        disp_drv.ver_res    = VER_RES;
        lv_disp_drv_register(&disp_drv);

        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = evdev_read;
        lv_indev_drv_register(&indev_drv);
        */

        return (0);
    }

    void splash(){
        lv_obj_t * logo = lv_img_create(lv_scr_act());
        lv_img_dsc_t pillo_logo;
        pillo_logo.header.cf = LV_IMG_CF_TRUE_COLOR;
        pillo_logo.header.always_zero = 0;
        pillo_logo.header.reserved = 0;
        pillo_logo.header.w = 512;
        pillo_logo.header.h = 512;
        pillo_logo.data_size = 262144 * LV_COLOR_SIZE / 8;
        pillo_logo.data = pillo_logo_map;

        lv_img_set_src(logo, &pillo_logo);
        lv_obj_align(logo, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t* spinner = lv_spinner_create(lv_scr_act(), 1000, 30);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0x3e03ff), LV_PART_INDICATOR);
        lv_obj_set_size(spinner, 60, 60);
        lv_obj_set_align(spinner, LV_ALIGN_CENTER);
        lv_obj_set_pos(spinner, 0, VER_RES/3);
        
        for(int i = 0; i < 500; i++) {
            lv_task_handler();
            usleep(5000);
        }

        lv_obj_del(spinner);
        lv_obj_del(logo);
    }

    void display_init(){
        lvgl_init();
        
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

        splash();
        
        while(1){
            main_menu::menu_app();
        }
    }
}

