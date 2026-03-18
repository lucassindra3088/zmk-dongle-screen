#include "layer_roller.h"

#include <ctype.h>
#include <zmk/display.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/keymap.h>

#include <fonts.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static char layer_names_buffer[256] = {0};

static int layer_select_id[6] = {2, 4, 3, 1, 0};

static int layer_display_order[6] = {4, 3, 0, 2, 1};

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_roller_state {
    uint8_t index;
};

static void layer_roller_set_sel(lv_obj_t *roller, struct layer_roller_state state) {
    if (state.index == 1) {
        lv_obj_set_style_text_color(roller, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_SELECTED);
    } else if (state.index == 4) {
        lv_obj_set_style_text_color(roller, lv_palette_main(LV_PALETTE_GREEN), LV_PART_SELECTED);
    } else {
        lv_obj_set_style_text_color(roller, lv_color_white(), LV_PART_SELECTED);
    }
    lv_roller_set_selected(roller, layer_select_id[state.index], LV_ANIM_ON);
}

static void layer_roller_update_cb(struct layer_roller_state state) {
    struct zmk_widget_layer_roller *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        layer_roller_set_sel(widget->obj, state);
    }
}

static struct layer_roller_state layer_roller_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_roller_state){
        .index = index,
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_roller, struct layer_roller_state, layer_roller_update_cb,
                            layer_roller_get_state)
ZMK_SUBSCRIPTION(widget_layer_roller, zmk_layer_state_changed);

int zmk_widget_layer_roller_init(struct zmk_widget_layer_roller *widget, lv_obj_t *parent) {
    widget->obj = lv_roller_create(parent);
    lv_obj_set_size(widget->obj, 240, 80);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_black());
    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_text_line_space(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_obj_add_style(widget->obj, &style, 0);

    lv_obj_set_style_text_align(widget->obj, LV_ALIGN_LEFT_MID, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_40, LV_PART_SELECTED);   
    lv_obj_set_style_text_color(widget->obj, lv_color_white(), LV_PART_SELECTED);

    lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->obj, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_MAIN);

    layer_names_buffer[0] = '\0';
    char *ptr = layer_names_buffer;

    for (int i = 0; i < ZMK_KEYMAP_LAYERS_LEN; i++) {
        const char *layer_name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(layer_display_order[i]));
        if (layer_name) {
            if (i > 0) {
                strcat(ptr, "\n");
                ptr += strlen(ptr);
            }

            if (layer_name && *layer_name) {
                #if IS_ENABLED(CONFIG_LAYER_ROLLER_ALL_CAPS)
                while (*layer_name) {
                    *ptr = toupper((unsigned char)*layer_name);
                    ptr++;
                    layer_name++;
                }
                *ptr = '\0';
                #else
                strcat(ptr, layer_name);
                ptr += strlen(layer_name);
                #endif
            } else {
                char index_str[2];
                snprintf(index_str, sizeof(index_str), "%d", i);
                strcat(ptr, index_str);
                ptr += strlen(index_str);
            }
        }
    }

    lv_roller_set_options(widget->obj, layer_names_buffer, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(widget->obj, 3);
    
    lv_obj_set_style_anim_time(widget->obj, 400, 0);
    
    sys_slist_append(&widgets, &widget->node);
    
    widget_layer_roller_init();
    return 0;
}

lv_obj_t *zmk_widget_layer_roller_obj(struct zmk_widget_layer_roller *widget) {
    return widget->obj;
}
