//-----------------------------------------------------------------------------
// MEKA - config.h
// Configuration File Load/Save - Code
//-----------------------------------------------------------------------------

#ifndef __CONFIG__
#define __CONFIG__

#include "shared.h"
#include "app_filebrowser.h"
#include "app_tileview.h"
#include "blitintf.h"
#include "capture.h"
#include "config.h"
#include "debugger.h"
#include "fskipper.h"
#include "glasses.h"
#include "rapidfir.h"
#include "video.h"
#include "textbox.h"
#include "tvtype.h"
#include "libparse.h"


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

static FILE *       CFG_File;

static void  CFG_Write_Line(const char* fmt, ...)
{ 
	va_list args;
	va_start(args, fmt);
	vfprintf(CFG_File, fmt, args); 
	va_end(args);
	fprintf(CFG_File, "\n"); 
}
static void  CFG_Write_Int(const char *name, int value)
{ 
	fprintf(CFG_File, "%s = %d\n", name, value); 
}
static void  CFG_Write_Str(const char *name, const char *str) 
{ 
	fprintf(CFG_File, "%s = %s\n", name, str); 
}

static void  CFG_Write_StrEscape (const char *name, const char *str)
{
    char *str_escaped = parse_escape_string(str, NULL);
    if (str_escaped)
    {
        fprintf(CFG_File, "%s = %s\n", name, str_escaped);
        free(str_escaped);
    }
    else
    {
        fprintf(CFG_File, "%s = %s\n", name, str);
    }
}

static void		Font_FindByName(const char* name, int* out)
{
	for (int i = 0; i < FONTID_COUNT_; i++)
	{
		if (stricmp(name, Fonts[i].text_id) == 0)
		{
			*out = (int)Fonts[i].id;
			return;
		}
	}
}

// allegro fullscreen workaround //
// Define a flag so we know we're in fullscreen windowed mode
// and thus need to ignore subsequent invocations of "video_gui_resolution"
typedef struct {
    bool fullscreen_windowd_mode;
    int official_width;
    int official_height;
    int calculated_width;
    int calculated_height;
}fullscreen_windowed_mode_workaround;
static fullscreen_windowed_mode_workaround fullscreen_windowed_mode_instance;

// Handle a variable assignment during configuration file loading
static void     Configuration_Load_Line (char *var, char *value)
{
	StrLower(var);

	// All input is changed to lower case for easier compare (apart from 'last_directory')
	if (strcmp(var, "last_directory") != 0)
		StrLower(value);

	// Select
	if (!strcmp(var, "frameskip_mode"))					{ fskipper.Mode = !strcmp(value, "unthrottled") ? FRAMESKIP_MODE_UNTHROTTLED : FRAMESKIP_MODE_THROTTLED; return; }
	if (!strcmp(var, "frameskip_throttle_speed"))		{ fskipper.Throttled_Speed = atoi(value); return; }
	if (!strcmp(var, "frameskip_unthrottled_frameskip")){ fskipper.Unthrottled_Frameskip = atoi(value); return; }
	if (!strcmp(var, "video_game_blitter"))				{ Blitters.blitter_configuration_name = strdup(value); return; }
	if (!strcmp(var, "sound_enabled"))					{ Sound.Enabled = (bool)atoi(value); return; }
	if (!strcmp(var, "sound_rate"))						{ const int n = atoi(value); if (n > 0) Sound.SampleRate = atoi(value); return; }
	if (!strcmp(var, "video_gui_resolution"))
	{
        // allegro fullscreen workaround //
        // Check to see if the fullscreen_windowed_mode_hack_flag flag is set.
        // If it is, bail out.
        // This is part of the allegro fullscreen bug workaround.  We need to use windowed fullscreen instead
        if (fullscreen_windowed_mode_instance.fullscreen_windowd_mode) {
            int x, y;
            if (sscanf(value, "%dx%d", &x, &y) == 2)
            {
                fullscreen_windowed_mode_instance.official_width = x;
                fullscreen_windowed_mode_instance.official_height = y;
            }
            return;
        }
        
		int x, y;
		if (sscanf(value, "%dx%d", &x, &y) == 2)
		{
			g_configuration.video_mode_gui_res_x = x;
			g_configuration.video_mode_gui_res_y = y;
		}
		return;
	}
	if (!strcmp(var, "video_gui_vsync"))				{ g_configuration.video_mode_gui_vsync = (bool)atoi(value); return; }
	if (!strcmp(var, "start_in_gui"))					{ g_configuration.start_in_gui = (bool)atoi(value); return; }
	if (!strcmp(var, "theme"))							{ Skins_SetSkinConfiguration(value); return; }
	if (!strcmp(var, "game_window_scale"))				{ g_configuration.game_window_scale = atof(value); if (g_configuration.game_window_scale < 0) g_configuration.game_window_scale = 1.0f; return; }
	if (!strcmp(var, "fb_uses_db"))						{ g_configuration.fb_uses_DB = (bool)atoi(value); return; }
	if (!strcmp(var, "fb_close_after_load"))			{ g_configuration.fb_close_after_load = (bool)atoi(value); return; }
	if (!strcmp(var, "fb_fullscreen_after_load"))		{ g_configuration.fullscreen_after_load = (bool)atoi(value); return; }
	if (!strcmp(var, "last_directory"))					{ snprintf(FB.current_directory, FILENAME_LEN, "%s", value); return; }
	if (!strcmp(var, "bios_logo"))						{ g_configuration.enable_BIOS = (bool)atoi(value); return; }
	if (!strcmp(var, "rapidfire"))						{ RapidFire = atoi(value); return; }
	if (!strcmp(var, "country"))						{ if (!strcmp(value, "jp")) g_configuration.country = COUNTRY_JAPAN; else g_configuration.country = COUNTRY_EXPORT; return; }
	if (!strcmp(var, "tv_type"))
	{
		if (strcmp(value, "ntsc") == 0)
			TV_Type_User = &TV_Type_Table[TVTYPE_NTSC];
		else
			if (strcmp(value, "pal") == 0 || strcmp(value, "secam") || strcmp(value, "pal/secam"))
				TV_Type_User = &TV_Type_Table[TVTYPE_PAL_SECAM];
		TVType_Update_Values();
		return;
	}
	if (!strcmp(var, "show_product_number"))			{ g_configuration.show_product_number = (bool)atoi(value); return; }
	if (!strcmp(var, "show_messages_fullscreen"))		{ g_configuration.show_fullscreen_messages = (bool)atoi(value); return; }
	if (!strcmp(var, "screenshot_template"))
	{ 
		// Note: Obsolete variable name, see below
		StrReplace(value, '*', ' ');
        g_configuration.capture_filename_template = strdup(value);
		return;
	}
	if (!strcmp(var, "screenshots_filename_template"))	{ g_configuration.capture_filename_template = strdup(value); return; }
	if (!strcmp(var, "screenshots_crop_align_8x8"))		{ g_configuration.capture_crop_align_8x8 = (bool)atoi(value); return; }
    if (!strcmp(var, "3dglasses_status"))				{ Glasses_Set_Status_initial(atoi(value)); return; }
    if (!strcmp(var, "3dglasses_mode"))					{ Glasses_Set_Mode(atoi(value)); return; }
	if (!strcmp(var, "3dglasses_com_port"))				{ Glasses_Set_ComPort(atoi(value)); return; }
	if (!strcmp(var, "iperiod"))						{ opt.IPeriod = atoi(value); return; }
	if (!strcmp(var, "iperiod_coleco"))					{ opt.IPeriod_Coleco = atoi(value); return; }
	if (!strcmp(var, "iperiod_sg1000_sc3000"))			{ opt.IPeriod_Sg1000_Sc3000 = atoi(value); return; }
	if (!strcmp(var, "nes_sucks"))						{ if (atoi(value) < 1) Quit_Msg("\n%s", Msg_Get(MSG_NES_Sucks)); return; }
	if (!strcmp(var, "sprite_flickering"))
	{
		if (strcmp(value, "auto") == 0)
			g_configuration.sprite_flickering = SPRITE_FLICKERING_AUTO;
		else if (strcmp(value, "yes") == 0)
			g_configuration.sprite_flickering = SPRITE_FLICKERING_ENABLED;
		else if (strcmp(value, "no") == 0)
			g_configuration.sprite_flickering = SPRITE_FLICKERING_NO;
		return;
	}
	if (!strcmp(var, "language"))						{ Lang_Set_by_Name(value); return; }
	if (!strcmp(var, "music_wav_filename_template"))	{ Sound.LogWav_FileName_Template = strdup(value); return; }
	if (!strcmp(var, "music_vgm_filename_template"))	{ Sound.LogVGM_FileName_Template = strdup(value); return; }
	if (!strcmp(var, "music_vgm_log_accuracy"))
	{
		if (strcmp(value, "frame") == 0)
			Sound.LogVGM_Logging_Accuracy = VGM_LOGGING_ACCURACY_FRAME;
		else if (strcmp(value, "sample") == 0)
			Sound.LogVGM_Logging_Accuracy = VGM_LOGGING_ACCURACY_SAMPLE;
		return;
	}
	if (!strcmp(var, "fm_enabled"))						{ Sound.FM_Enabled = (bool)atoi(value); return; }
	if (!strcmp(var, "video_gui_refresh_rate"))
	{
		if (!strcmp(value, "auto"))
			g_configuration.video_mode_gui_refresh_rate = 0;
		else
			g_configuration.video_mode_gui_refresh_rate = atoi(value);
		return;
	}
	if (!strcmp(var, "allow_opposite_directions"))		{ g_configuration.allow_opposite_directions = (bool)atoi(value); return; }
	if (!strcmp(var, "debugger_disassembly_display_labels")) { g_configuration.debugger_disassembly_display_labels = (bool)atoi(value); return; }
	if (!strcmp(var, "debugger_log"))					{ g_configuration.debugger_log_enabled = (bool)atoi(value); return; }
	if (!strcmp(var, "memory_editor_lines"))			{ g_configuration.memory_editor_lines = MAX(1, atoi(value)); return; }
	if (!strcmp(var, "memory_editor_columns"))			{ g_configuration.memory_editor_columns = MAX(1, atoi(value)); return; }
	if (!strcmp(var, "video_game_vsync"))				{ g_configuration.video_mode_game_vsync = (bool)atoi(value); return; }
	if (!strcmp(var, "screenshots_crop_scrolling_column")) { g_configuration.capture_crop_scrolling_column = (bool)atoi(value); return; }
	if (!strcmp(var, "screenshots_include_gui"))		{ g_configuration.capture_include_gui = (bool)atoi(value); return; }
	// allegro fullscreen workaround //
    // Due to some bug in the version of Allegro used in this build, we're using
    // a borderless windowed mode instead of fullscreen.
    // However, the "video_gui_resolution" config parameter needs to match the
    // screen's current resoultion for this to work.
    //if (!strcmp(var, "video_fullscreen"))				{ g_configuration.video_fullscreen = (bool)atoi(value); return; }
    if (!strcmp(var, "video_fullscreen")) {
        // Do what we said we wdoud do
        g_configuration.video_fullscreen = (bool)atoi(value);
        
        if (g_configuration.video_fullscreen) {
            // Calculate current resolution
            ALLEGRO_MONITOR_INFO info;
            al_get_monitor_info(0, &info);
            int w = info.x2 - info.x1;
            int h = info.y2 - info.y1;
            
            // In fullscreen mode, override the default resolution with the current screen resolution.
            g_configuration.video_mode_gui_res_x = w;
            g_configuration.video_mode_gui_res_y = h;
            
            // Set a flag so we know we're in fullscreen windowed mode
            // and thus need to ignore subsequent invocations of "video_gui_resolution"
            // Also, capture the calculated width and height for later use
            fullscreen_windowed_mode_instance.fullscreen_windowd_mode = true;
            fullscreen_windowed_mode_instance.calculated_width = w;
            fullscreen_windowed_mode_instance.calculated_height = h;
        }
        return;
    }
	if (!strcmp(var, "video_driver"))					{ g_configuration.video_driver = VideoDriver_FindByName(value); return; }

	// Fonts
	if (!strcmp(var, ("font_menus")))			{ Font_FindByName(value, &g_configuration.font_menus); }
	if (!strcmp(var, ("font_messages")))		{ Font_FindByName(value, &g_configuration.font_messages); }
	if (!strcmp(var, ("font_options")))			{ Font_FindByName(value, &g_configuration.font_options); }
	if (!strcmp(var, ("font_debugger")))		{ Font_FindByName(value, &g_configuration.font_debugger); }
	if (!strcmp(var, ("font_documentation")))	{ Font_FindByName(value, &g_configuration.font_documentation); }
	if (!strcmp(var, ("font_techinfo")))		{ Font_FindByName(value, &g_configuration.font_techinfo); }
	if (!strcmp(var, ("font_filebrowser")))		{ Font_FindByName(value, &g_configuration.font_filebrowser); }
	if (!strcmp(var, ("font_about")))			{ Font_FindByName(value, &g_configuration.font_about); }

	// Obsolete variables
	if (!strcmp(var, "fm_emulator"))			{}
	if (!strcmp(var, "opl_speed"))				{}
	if (!strcmp(var, "gui_video_depth"))		{}
	if (!strcmp(var, "gui_video_driver"))		{}
	if (!strcmp(var, "mario_is_a_fat_plumber"))	{}
	if (!strcmp(var, "video_game_depth"))		{}
	if (!strcmp(var, "frameskip_auto_speed"))	{}
	if (!strcmp(var, "frameskip_normal_speed"))	{}
	if (!strcmp(var, "sound_card"))				{}
	if (!strcmp(var, "video_game_triple_buffering"))	{}
	if (!strcmp(var, "video_game_page_flipping"))		{}
}

// Load configuration file data from MEKA.CFG
void Configuration_Load()
{
    ConsolePrintf(Msg_Get(MSG_Config_Loading), g_env.Paths.ConfigurationFile);

    // Open and read file
	t_tfile *  tf;
    if ((tf = tfile_read(g_env.Paths.ConfigurationFile)) == NULL)
    {
        ConsolePrintf("%s\n", meka_strerror());
		opt.Setup_Interactive_Execute = true;
        return;
    }
    ConsolePrint("\n");

    // Parse each line
    int line_cnt = 0;
    for (t_list* lines = tf->data_lines; lines; lines = lines->next)
    {
        line_cnt += 1;

		char* line = (char*)lines->elem;
        if (StrIsNull(line))
            continue;

		char variable[256], val[256];
        if (parse_getword(variable, sizeof(variable), &line, "=", ';', PARSE_FLAGS_NONE))
        {
            parse_skip_spaces(&line);
            if (parse_getword(val, sizeof(val), &line, "", ';', PARSE_FLAGS_NONE))
                Configuration_Load_Line(variable, val);
        }
    }

    // Free file data
    tfile_free(tf);

	g_configuration.loaded_configuration_file = true;
}

// Save configuration file data to MEKA.CFG
void Configuration_Save()
{
    char   s1 [256];

    if (!(CFG_File = fopen(g_env.Paths.ConfigurationFile, "wt")))
        return;

    CFG_Write_Line (";");
    CFG_Write_Line ("; " MEKA_NAME " " MEKA_VERSION " - Configuration File");
    CFG_Write_Line (";");
    CFG_Write_Line ("");

    CFG_Write_Line ( "-----< FRAME SKIPPING >-----------------------------------------------------");
    if (fskipper.Mode == FRAMESKIP_MODE_THROTTLED)
        CFG_Write_Line  ("frameskip_mode = throttled");
    else
        CFG_Write_Line  ("frameskip_mode = unthrottled");
    CFG_Write_Int  ("frameskip_throttle_speed", fskipper.Throttled_Speed);
    CFG_Write_Int  ("frameskip_unthrottled_frameskip", fskipper.Unthrottled_Frameskip);
	CFG_Write_Line ("");

	CFG_Write_Line ( "-----< VIDEO >--------------------------------------------------------------");
	CFG_Write_Str  ("video_driver", g_configuration.video_driver->name);
	CFG_Write_Line ("(Available video drivers: opengl, directx)");
	CFG_Write_Int  ("video_fullscreen", g_configuration.video_fullscreen);

	CFG_Write_StrEscape("video_game_blitter", Blitters.current->name);
    CFG_Write_Line ("(See MEKA.BLT file to configure blitters/fullscreen modes)");
	CFG_Write_Int  ("video_game_vsync", g_configuration.video_mode_game_vsync);
    
    // allegro fullscreen workaround //
    // This is part of the allegro fullscreen bug workaround.
    if (fullscreen_windowed_mode_instance.fullscreen_windowd_mode) {
        sprintf        (s1, "%dx%d", fullscreen_windowed_mode_instance.official_width, fullscreen_windowed_mode_instance.official_height);
        CFG_Write_Str  ("video_gui_resolution", s1);
    } else {
        sprintf        (s1, "%dx%d", g_configuration.video_mode_gui_res_x, g_configuration.video_mode_gui_res_y);
        CFG_Write_Str  ("video_gui_resolution", s1);
    }
    if (g_configuration.video_mode_gui_refresh_rate == 0)
        CFG_Write_Str ("video_gui_refresh_rate", "auto");
    else
		CFG_Write_Int ("video_gui_refresh_rate", g_configuration.video_mode_gui_refresh_rate);
    CFG_Write_Line ("(Video mode refresh rate. Set 'auto' for default rate. Not all drivers");
    CFG_Write_Line (" support non-default rate. Customized values then depends on your video");
    CFG_Write_Line (" card and monitor. Setting to 60 (Hz) is usually a good thing as the screen");
    CFG_Write_Line (" will be refreshed at the same time as the emulated systems.)");
    CFG_Write_Int  ("video_gui_vsync", g_configuration.video_mode_gui_vsync);
    CFG_Write_Line ("(enable vertical synchronisation for fast computers)");
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< INPUTS >--------------------------------------------------------------");
    CFG_Write_Line ("(See MEKA.INP file to configure inputs sources)");
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< SOUND AND MUSIC >-----------------------------------------------------");
    CFG_Write_Int  ("sound_enabled", Sound.Enabled);
    CFG_Write_Int  ("sound_rate", Sound.SampleRate);
    CFG_Write_Int  ("fm_enabled", Sound.FM_Enabled);
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< GRAPHICAL USER INTERFACE CONFIGURATION >------------------------------");
    CFG_Write_Int  ("start_in_gui", g_configuration.start_in_gui);
    CFG_Write_StrEscape("theme", Skins_GetCurrentSkin()->name);
	fprintf(CFG_File, "game_window_scale = %.2f\n", g_configuration.game_window_scale); 
    CFG_Write_Int  ("fb_uses_db", g_configuration.fb_uses_DB);
    CFG_Write_Int  ("fb_close_after_load", g_configuration.fb_close_after_load);
    CFG_Write_Int  ("fb_fullscreen_after_load", g_configuration.fullscreen_after_load);
    CFG_Write_StrEscape  ("last_directory", FB.current_directory);
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< MISCELLANEOUS OPTIONS >-----------------------------------------------");
    CFG_Write_StrEscape  ("language", Messages.Lang_Cur->Name);
    CFG_Write_Int  ("bios_logo", g_configuration.enable_BIOS);
    CFG_Write_Int  ("rapidfire", RapidFire);
    CFG_Write_Str  ("country", (g_configuration.country == COUNTRY_EXPORT) ? "us/eu" : "jp");
    CFG_Write_Line ("(emulated machine country, either 'us/eu' or 'jp'");
    if (g_configuration.sprite_flickering & SPRITE_FLICKERING_AUTO)
        CFG_Write_Line ("sprite_flickering = auto");
    else
        CFG_Write_Str ("sprite_flickering", (g_configuration.sprite_flickering & SPRITE_FLICKERING_ENABLED) ? "yes" : "no");
    CFG_Write_Line ("(hardware sprite flickering emulator, either 'yes', 'no', or 'auto'");
    CFG_Write_Str  ("tv_type", (TV_Type_User->id == TVTYPE_NTSC) ? "ntsc" : "pal/secam");
    CFG_Write_Line ("(emulated TV type, either 'ntsc' or 'pal/secam'");
    //CFG_Write_Int  ("tv_snow_effect", effects.TV_Enabled);
    //CFG_Write_Str  ("palette", (g_configuration.palette_type == PALETTE_TYPE_MUTED) ? "muted" : "bright");
    //CFG_Write_Line ("(palette type, either 'muted' or 'bright'");
    CFG_Write_Int  ("show_product_number", g_configuration.show_product_number);
    CFG_Write_Int  ("show_messages_fullscreen", g_configuration.show_fullscreen_messages);
    CFG_Write_Int  ("allow_opposite_directions", g_configuration.allow_opposite_directions);
    CFG_Write_StrEscape  ("screenshots_filename_template", g_configuration.capture_filename_template);
	CFG_Write_Int  ("screenshots_crop_scrolling_column", g_configuration.capture_crop_scrolling_column);
	CFG_Write_Int  ("screenshots_crop_align_8x8", g_configuration.capture_crop_align_8x8);
	CFG_Write_Int  ("screenshots_include_gui", g_configuration.capture_include_gui);
    CFG_Write_StrEscape  ("music_wav_filename_template", Sound.LogWav_FileName_Template);
    CFG_Write_StrEscape  ("music_vgm_filename_template", Sound.LogVGM_FileName_Template);
    CFG_Write_Line ("(see documentation for more information about templates)");
    CFG_Write_Str  ("music_vgm_log_accuracy", (Sound.LogVGM_Logging_Accuracy == VGM_LOGGING_ACCURACY_FRAME) ? "frame" : "sample");
    CFG_Write_Line ("(either 'frame' or 'sample')");
    CFG_Write_Line ("");

	CFG_Write_Line ("-----< FONTS >---------------------------------------------------------------");
	CFG_Write_Line ("( available fonts:");
	for (int i = 0; i < FONTID_COUNT_; i++)
		CFG_Write_Line("  - %s, %d px%s", Fonts[i].text_id, Fonts[i].height, i+1==FONTID_COUNT_ ? " )" : "");
	CFG_Write_Str  ("font_menus",			Fonts[g_configuration.font_menus].text_id);
	CFG_Write_Str  ("font_messages",		Fonts[g_configuration.font_messages].text_id);
	CFG_Write_Str  ("font_options",			Fonts[g_configuration.font_options].text_id);
	CFG_Write_Str  ("font_debugger",		Fonts[g_configuration.font_debugger].text_id);
	CFG_Write_Str  ("font_documentation",	Fonts[g_configuration.font_documentation].text_id);
	CFG_Write_Str  ("font_techinfo",		Fonts[g_configuration.font_techinfo].text_id);
	CFG_Write_Str  ("font_filebrowser",		Fonts[g_configuration.font_filebrowser].text_id);
	CFG_Write_Str  ("font_about",			Fonts[g_configuration.font_about].text_id);
	CFG_Write_Line ("");

    CFG_Write_Line ("-----< 3-D GLASSES EMULATION >-----------------------------------------------");
    CFG_Write_Int  ("3dglasses_status", Glasses.Enabled);
    CFG_Write_Line ("('0' = 3d glasses disabled)");
    CFG_Write_Line ("('1' = 3d glasses enabled)");
    CFG_Write_Line ("('2' = 3d glasses auto)");
    CFG_Write_Int  ("3dglasses_mode", Glasses.Mode);
    CFG_Write_Line ("('0' = show both sides and become blind)");
    CFG_Write_Line ("('1' = play without 3-D Glasses, show only left side)");
    CFG_Write_Line ("('2' = play without 3-D Glasses, show only right side)");
    CFG_Write_Line ("('3' = uses real 3-D Glasses connected to COM port)");
    CFG_Write_Line ("('4' = uses real 3-D Glasses in 3D SBS)");
    // if (Glasses_Mode == GLASSES_MODE_COM_PORT)
    {
        CFG_Write_Int  ("3dglasses_com_port", Glasses.ComPort);
        CFG_Write_Line ("(this is on which COM port the Glasses are connected. Either 1 or 2)");
    }
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< EMULATION OPTIONS >---------------------------------------------------");
    CFG_Write_Int  ("iperiod", opt.IPeriod);
    CFG_Write_Int  ("iperiod_coleco", opt.IPeriod_Coleco);
    CFG_Write_Int  ("iperiod_sg1000_sc3000", opt.IPeriod_Sg1000_Sc3000);
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< DEBUGGING FUNCTIONNALITIES -------------------------------------------");
    CFG_Write_Int  ("debugger_disassembly_display_labels", g_configuration.debugger_disassembly_display_labels);
    CFG_Write_Int  ("debugger_log", g_configuration.debugger_log_enabled);
    CFG_Write_Int  ("memory_editor_lines", g_configuration.memory_editor_lines);
    CFG_Write_Int  ("memory_editor_columns", g_configuration.memory_editor_columns);
    CFG_Write_Line ("(preferably make columns a multiple of 8)");
    CFG_Write_Line ("");

    CFG_Write_Line ("-----< FACTS >---------------------------------------------------------------");
    CFG_Write_Line ("nes_sucks = 1");

    fclose (CFG_File);
}

//-----------------------------------------------------------------------------

static void     Param_Check(int *current, const char *msg)
{
    if ((*current) + 1 >= g_env.argc)
        Quit_Msg("%s", msg);
    (*current)++;
}

void    Command_Line_Parse()
{
	const char* params[] =
	{
		"HELP", "?",
		"NOELEPHANT", "LOG", "LOAD",
		"SETUP",
		"DEBUG_INFOS",
		NULL
	};

	for (int i = 1; i != g_env.argc; i++)
	{
		const char *s = g_env.argv[i];
		if (s[0] == '-'
#if defined(ARCH_WIN32)
			|| s[0] == '/'
#endif
			)
		{
			int j;
			for (j = 0; params[j]; j++)
				if (!stricmp (s + 1, params[j]))
					break;
			switch (j)
			{
			case 0: // HELP
			case 1: Command_Line_Help();
				break;
			case 2: // NOELEPHANT
				break;
			case 3: // LOG
				Param_Check(&i, Msg_Get(MSG_Log_Need_Param));
				TB_Message.log_filename = strdup(g_env.argv[i]);
				break;
			case 4: // LOAD
				Param_Check(&i, Msg_Get(MSG_Load_Need_Param));
				opt.State_Load = atoi(g_env.argv[i]);
				break;
			case 5: // SETUP
				opt.Setup_Interactive_Execute = true;
				break;
				// Private Usage
			case 6: // DEBUG_INFOS
				g_env.debug_dump_infos = true;
				if (TB_Message.log_filename == NULL)
					TB_Message.log_filename = strdup("debuglog.txt");
				break;
			default:
				ConsolePrintf(Msg_Get(MSG_Error_Param), s);
				ConsolePrint("\n--\n");
				Command_Line_Help();
				return;
			}
		}
		else
		{
			// FIXME: specifying more than one ROM ?
			strcpy(g_env.Paths.MediaImageFile, s);
			//MessageBox(NULL, s, s, 0);
		}
	}
}

void    Command_Line_Help(void)
{
    // Note: this help screen is not localized.
    Quit_Msg(
        #ifdef ARCH_WIN32
        "Syntax: MEKAW [rom] [options] ...\n"
        #else
        "Syntax: MEKA [rom] [option1] ...\n"
        #endif
        ""                                                                   "\n" \
        "Where [rom] is a valid rom image to load. Available options are:"   "\n" \
        ""                                                                   "\n" \
        "  -HELP -?         : Display command line help"                     "\n" \
        "  -SETUP           : Start with the setup screen"                   "\n" \
        "  -EURO -US        : Emulate an European/US system for this session""\n" \
        "  -JAP -JP -JPN    : Emulate a Japanese system for this session"    "\n" \
        "  -DEBUG           : Enable debugging features"                     "\n" \
        "  -LOAD <n>        : Load savestate <n> on startup"                 "\n" \
        "  -LOG <file>      : Log message to file <file> (appending it)"     "\n" \
        "  -NOELEPHANT      : Just what it says"                             "\n"
        );
    //   "  -NIRV            : Speed up emulation dramatically"          "\n"
}

//-----------------------------------------------------------------------------

#endif
