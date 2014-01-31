#include "../Library/scalar.h"
#include "../Library/vector.h"

enum ui_window_flags {
	ui_window_flags_visible           = (1 <<  0),
	ui_window_flags_no_events         = (1 <<  1),
	ui_window_flags_show_time         = (1 <<  2),
	ui_window_flags_show_coords       = (1 <<  3),
	ui_window_flags_no_wrap           = (1 <<  4),
	ui_window_flags_shadow            = (1 <<  5),
	ui_window_flags_want_enter        = (1 <<  6),
	ui_window_flags_natural_mat_scale = (1 <<  7),
	ui_window_flags_no_clip           = (1 <<  8),
	ui_window_flags_no_cursor         = (1 <<  9),
	ui_window_flags_menu_gui          = (1 << 10),
	ui_window_flags_modal             = (1 << 11),
	ui_window_flags_invert_rect       = (1 << 12)
};

enum ui_window_text_align {
	ui_window_text_align_left,
	ui_window_text_align_center,
	ui_window_text_align_right
};

struct ui_window {
	u4 flags;
	vec4 rect;
	vec4 fore_color;
	vec4 hover_color;
	vec4 back_color;
	vec4 border_color;
	vec4 mat_color;
	vec rotate;
	vec text_scale;
	const char *text;
	const char *background;
	
	vec force_aspect_width;
	vec force_aspect_height;
	vec mat_scale_x;
	vec mat_scale_y;
	vec border_size;
	enum ui_window_text_align text_align;
	vec text_align_x;
	vec text_align_y;
	vec2 shear;
	const char *name;
	const char *play;
	const char *font;
	
	void (*onTime)(struct ui_window *, u4);
	void (*onAction)(struct ui_window *);
	void (*onActionRelease)(struct ui_window *);
	void (*onNamedEvent)(struct ui_window *, const char *event);
	void (*onMouseEnter)(struct ui_window *);
	void (*onMouseExit)(struct ui_window *);
	void (*onActivate)(struct ui_window *);
	void (*onDeactivate)(struct ui_window *);
	void (*onEsc)(struct ui_window *);
	void (*onFrame)(struct ui_window *);
	void (*onTrigger)(struct ui_window *);
	void (*onEnter)(struct ui_window *);
	void (*onEnterRelease)(struct ui_window *);
};

enum ui_edit_flags {
	ui_edit_flags_numeric,
	ui_edit_flags_wrap,
	ui_edit_flags_read_only,
	ui_edit_flags_force_scroll,
	ui_edit_flags_password,
	ui_edit_flags_live_update
};

struct ui_edit_window {
	struct ui_window window;
	u4 max_chars;
	const char *source;
	const char *cvar;
	const char *cvar_group;
	const char *value;
};

enum ui_choice_flags {
	ui_choice_flags_live_update
};

enum ui_choice_type {
	ui_choice_type_array,
	ui_choice_type_string
};

struct ui_choice_window {
	u1 choice_type;
	u4 current_choice;
	const char *choices;
	const char *values;
	const char *gui;
	const char *cvar;
	const char *cvar_group;
};

void ui_set_focus(struct ui_window *);
void ui_show_cursor(u1);
void ui_sound(const char *);
