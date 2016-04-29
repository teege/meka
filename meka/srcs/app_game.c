//-----------------------------------------------------------------------------
// MEKA - app_game.c
// Game screen applet - Code
//-----------------------------------------------------------------------------
// FIXME: very old code.
//-----------------------------------------------------------------------------

#include "shared.h"
#include "app_game.h"
#include "db.h"
#include "vdp.h"
#include "glasses.h"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

t_gui_box *  gamebox_instance;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void        gamebox_draw(t_gui_box *box, ALLEGRO_BITMAP *game_buffer)
{
	const float scale = g_configuration.game_window_scale;

	int     x_start = g_driver->x_start;
    int     y_start = g_driver->y_show_start;
    int     x_len   = g_driver->x_res;
    int     y_len   = g_driver->y_res;
    int     x_dst   = box->frame.pos.x;
    int     y_dst   = box->frame.pos.y;

    if ((g_driver->id == DRV_SMS) && (Mask_Left_8))
    {
        // Center screen when 8 left columns are masked
        // This not logical but looks good
        al_draw_filled_rectangle(x_dst, y_dst, x_dst + 4*scale, y_dst + y_len*scale, COLOR_BLACK);
        al_draw_filled_rectangle(x_dst + (x_len - 4)*scale, y_dst, x_dst + x_len*scale, y_dst + y_len*scale, COLOR_BLACK);
        x_len -= 8;
        x_start += 8;
        x_dst += 4*scale;
    }
    
    // Render "skipped frame" (not really skipped) on the opposing side.
    // For now, assuming side-by side rather than top/bottom.
    int tom_3d_pos_xstart = 0;
    int tom_3d_pos_xlen = 0;
    
    // If mustskipframe returns 1 twice, then we need to enter "render all frmes" mode
    // When mustskipframe begins to alternate, we need to exit "render all frames" mode
    int left_or_right = Glasses_Must_Skip_Frame();
    static int num_consec_skipped_frames = 0;
    static bool render_all_frames = false;
    
    if (left_or_right) {
        num_consec_skipped_frames++;
    } else {
        num_consec_skipped_frames=0;
    }
    if (num_consec_skipped_frames>1) {
        render_all_frames = true;
    } else {
        render_all_frames = false;
    }
    
    
    if (Glasses.Enabled==GLASSES_STATUS_ENABLED || (Glasses.Enabled==GLASSES_STATUS_AUTO && Glasses.GameSupports3d)) {
        switch (Glasses.Mode) {
            case GLASSES_MODE_SHOW_BOTH:
                if (fabs(scale-1.0f) < 0.001f) {
                    al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                } else {
                    al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                }
                break;
            case GLASSES_MODE_SHOW_ONLY_LEFT:
                if (!render_all_frames) {
                    if (!left_or_right) {
                        if (fabs(scale-1.0f) < 0.001f) {
                            al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                        } else {
                            al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                        }
                    }
                } else {
                    // Dead code?
                    if (fabs(scale-1.0f) < 0.001f) {
                        al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                    } else {
                        al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                    }
                }
                break;
            case GLASSES_MODE_SHOW_ONLY_RIGHT:
                if (!render_all_frames) {
                    if (!left_or_right) {
                        if (fabs(scale-1.0f) < 0.001f) {
                            al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                        } else {
                            al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                        }
                    }
                } else {
                    // Dead code?
                    if (fabs(scale-1.0f) < 0.001f) {
                        al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                    } else {
                        al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                    }
                }
                break;
            case GLASSES_MODE_SBS:
                if (!render_all_frames) {
                    if (!left_or_right) {
                        // left eye
                        tom_3d_pos_xstart = x_start/2;
                        tom_3d_pos_xlen = (x_len/2)-(x_start/2);
                        al_draw_scaled_bitmap (
                                               game_buffer,
                                               tom_3d_pos_xstart,              // hor position of image >: more to the left <: more to the right
                                               y_start,                        // vert offset >: higher position <: lower position
                                               x_len,                          // hor stretch >: more condensed <: less condensed (anchored from left)
                                               y_len,                          // vert stretch >: more condensed <: less condensed (anchored from top)
                                               x_dst-tom_3d_pos_xstart/2,      // hor position of frame >: more to the right <: more to the left
                                               y_dst,
                                               x_len*scale/2,
                                               y_len*scale,
                                               0x0000);
                    } else {
                        // right eye
                        tom_3d_pos_xstart = x_start/2;
                        tom_3d_pos_xlen = x_len+(x_start/2);
                        al_draw_scaled_bitmap (
                                               game_buffer,
                                               0,
                                               y_start,
                                               tom_3d_pos_xlen,
                                               y_len,
                                               x_dst+((tom_3d_pos_xlen/2)-tom_3d_pos_xstart)*scale,
                                               y_dst,
                                               tom_3d_pos_xlen*scale/2,
                                               y_len*scale,
                                               0x0000);
                    }
                } else {
                    // left eye
                    tom_3d_pos_xstart = x_start/2;
                    tom_3d_pos_xlen = (x_len/2)-(x_start/2);
                    al_draw_scaled_bitmap (
                                           game_buffer,
                                           tom_3d_pos_xstart,              // hor position of image >: more to the left <: more to the right
                                           y_start,                        // vert offset >: higher position <: lower position
                                           x_len,                          // hor stretch >: more condensed <: less condensed (anchored from left)
                                           y_len,                          // vert stretch >: more condensed <: less condensed (anchored from top)
                                           x_dst-tom_3d_pos_xstart/2,      // hor position of frame >: more to the right <: more to the left
                                           y_dst,
                                           x_len*scale/2,
                                           y_len*scale,
                                           0x0000);
                    // right eye
                    tom_3d_pos_xstart = x_start/2;
                    tom_3d_pos_xlen = x_len+(x_start/2);
                    al_draw_scaled_bitmap (
                                           game_buffer,
                                           0,
                                           y_start,
                                           tom_3d_pos_xlen,
                                           y_len,
                                           x_dst+((tom_3d_pos_xlen/2)-tom_3d_pos_xstart)*scale,
                                           y_dst,
                                           tom_3d_pos_xlen*scale/2,
                                           y_len*scale,
                                           0x0000);
                }
                break;
            default: //(GLASSES_MODE_COM_PORT)
                if (fabs(scale-1.0f) < 0.001f) {
                    al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
                } else {
                    al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
                }
                break;
        }
    } else {
        if (fabs(scale-1.0f) < 0.001f) {
            al_draw_bitmap_region(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, 0x0000);
        } else {
            al_draw_scaled_bitmap(game_buffer, x_start, y_start, x_len, y_len, x_dst, y_dst, x_len*scale, y_len*scale, 0x0000);
        }
    }
  
}

void        gamebox_compute_size(int *x, int *y)
{
    *x = (g_driver->x_res * g_configuration.game_window_scale) - 1;
    *y = (g_driver->y_res * g_configuration.game_window_scale) - 1;
}

t_gui_box * gamebox_create(int x, int y)
{
    t_frame frame;
    frame.pos.x = x;
    frame.pos.y = y;
    gamebox_compute_size(&frame.size.x, &frame.size.y);

	t_gui_box *box = gui_box_new(&frame, "--");
    if (box == NULL)
        return (NULL);

    box->type = GUI_BOX_TYPE_GAME;
    box->flags |= GUI_BOX_FLAGS_TAB_STOP;
	box->flags |= GUI_BOX_FLAGS_ALLOW_RESIZE;
	box->size_min.x = (g_driver->x_res/2)-1;
	box->size_min.y = (g_driver->y_res/2)-1;
	box->size_step.x = g_driver->x_res/4;
	box->size_step.y = g_driver->y_res/4;
	box->size_fixed_ratio = true;

    gui_box_clip_position(box);
    gamebox_rename_all();

    return (box);
}

void	gamebox_resize_all()
{
    for (t_list* boxes = gui.boxes; boxes != NULL; boxes = boxes->next)
    {
        t_gui_box* box = (t_gui_box*)boxes->elem;
        if (box->type == GUI_BOX_TYPE_GAME)
        {
			box->size_min.x = (g_driver->x_res/2)-1;
			box->size_min.y = (g_driver->y_res/2)-1;
			box->size_step.x = g_driver->x_res/4;
			box->size_step.y = g_driver->y_res/4;
			int sx, sy;
            gamebox_compute_size(&sx, &sy);
			gui_box_resize(box, sx, sy, false);
        }
    }
    gui.info.must_redraw = TRUE;
}

void	gamebox_rename_all()
{
    const char *new_name;
    
	if (DB.current_entry)
	{
        new_name = DB_Entry_GetCurrentName(DB.current_entry);
	}
    else
    {
        if (g_machine_flags & MACHINE_CART_INSERTED)
            new_name = Msg_Get(MSG_DB_Name_Default);
        else
            new_name = Msg_Get(MSG_DB_Name_NoCartridge);
    }

    for (t_list* boxes = gui.boxes; boxes != NULL; boxes = boxes->next)
    {
        t_gui_box* box = (t_gui_box*)boxes->elem;
        if (box->type == GUI_BOX_TYPE_GAME)
            gui_box_set_title(box, new_name);
    }
}

//-----------------------------------------------------------------------------
