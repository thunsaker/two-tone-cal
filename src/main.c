#include <pebble.h>
#include <ctype.h>
#include "main.h"

static Window *window_main;
	
#ifdef PBL_COLOR
	static uint8_t primary_color;
	static uint8_t back_color;
#endif
	
static Layer *layer_back;
static Layer *layer_calendar;
static TextLayer *text_time;
static TextLayer *text_date;
static PropertyAnimation *animation_slide_calendar;

static GFont font_retro_micro;
static GFont font_retro_small;
static GFont font_retro_large;

bool invert = false;
static int theme = -1;
struct tm *currentTime;
int tap_count = 0;
AppTimer *calendar_tap_timer;

static void redraw_all() {
	layer_mark_dirty(layer_back);
	layer_mark_dirty(layer_calendar);
	
	#ifdef PBL_COLOR
		text_layer_set_text_color(text_time, (GColor)back_color);
		text_layer_set_text_color(text_date, (GColor)back_color);
	#else
		text_layer_set_text_color(text_time, GColorWhite);
		text_layer_set_text_color(text_date, GColorWhite);
	#endif
}

static void set_colors() {
	#ifdef PBL_COLOR
		switch(theme) {
			case 0:
				back_color = GColorWhite.argb;
				primary_color = GColorBlack.argb;
				break;
			case 1:
				back_color = GColorBlack.argb;
				primary_color = GColorWhite.argb;
				break;
			case 2:
				back_color = GColorWhite.argb;
				primary_color = GColorWindsorTan.argb;
				break;
			case 3:
				back_color = GColorWhite.argb;
				primary_color = GColorDarkGray.argb;
				break;
			case 4:
				back_color = GColorWhite.argb;
				primary_color = GColorSunsetOrange.argb;
				break;
			case 5:
				back_color = GColorWhite.argb;
				primary_color = GColorBlueMoon.argb;
				break;
			case 6:
				back_color = GColorWhite.argb;
				primary_color = GColorIndigo.argb;
				break;
			case 7:
				back_color = GColorBlack.argb;
				primary_color = GColorCyan.argb;
				break;
			case 8:
				back_color = GColorWhite.argb;
				primary_color = GColorFashionMagenta.argb;
				break;
			default:
				back_color = GColorWhite.argb;
				primary_color = GColorJaegerGreen.argb;
				break;
	    }
	
		window_set_background_color(window_main, (GColor)back_color);
	#else
		window_set_background_color(window_main, GColorWhite);
	#endif
}

static void transition_animation(bool reverse) {
	GRect start = GRect(0, 100, 144, 168);
	GRect finish = GRect(0, 64, 144, 168);
	
	if(reverse) {
		animation_slide_calendar = 
			property_animation_create_layer_frame(layer_calendar, &finish, &start);
	} else {
		animation_slide_calendar = 
			property_animation_create_layer_frame(layer_calendar, &start, &finish);
	}
	
	animation_schedule((Animation*)animation_slide_calendar);
}


static void calendar_return_tick() {
	transition_animation(true);
}

static void calendar_tap_tick() {
	tap_count = 0;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	if(tap_count < 1) {
		tap_count++;
		calendar_tap_timer = app_timer_register(3000, calendar_tap_tick, NULL);
	} else {
		app_timer_cancel(calendar_tap_timer);
		app_timer_register(5000, calendar_return_tick, NULL);
		transition_animation(false);
		tap_count = 0;
	}
}

static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char buffer_time[] = "00:00";
	static char buffer_date[] = "DAY 00";
	
	if(clock_is_24h_style() == true) {
		strftime(buffer_time, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		strftime(buffer_time, sizeof("00:00"), "%I:%M", tick_time);
	}
	strftime(buffer_date, sizeof("DAY 00"), "%a %d", tick_time);
	buffer_date[1] = toupper((unsigned char)buffer_date[1]);
	buffer_date[2] = toupper((unsigned char)buffer_date[2]);
	
	text_layer_set_text(text_time, buffer_time);
	text_layer_set_text(text_date, buffer_date);
	
	srand(temp);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	if (units_changed & MONTH_UNIT) {
		layer_mark_dirty(layer_calendar);
	}
	*currentTime = *tick_time;
	
	theme = rand() % 10;
	
	set_colors();
	redraw_all();
}

void draw_layer_back(Layer *cell_layer, GContext *ctx) {
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, (GColor)primary_color);
		graphics_fill_rect(ctx, GRect(0,0,144,TIME_HEIGHT), 0, GCornerNone);
	#else
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0,0,144,TIME_HEIGHT), 0, GCornerNone);
	#endif
}

const char daysOfWeek[7][3] = {"S", "M", "T", "W", "T", "F", "S"};

// How many days are/were in the month
int daysInMonth(int mon, int year)
{
    mon++;

    // April, June, September and November have 30 Days
    if (mon == 4 || mon == 6 || mon == 9 || mon == 11) {
        return 30;
    } else if (mon == 2) {
        // Deal with Feburary & Leap years
        if (year % 400 == 0) {
            return 29;
        } else if (year % 100 == 0) {
            return 28;
        } else if (year % 4 == 0) {
            return 29;
        } else {
            return 28;
        }
    } else {
        // Most months have 31 days
        return 31;
    }
}

struct tm *get_time() {
    time_t tt = time(0);
    return localtime(&tt);
}

void draw_layer_calendar(Layer *cell_layer, GContext *ctx) {
	#ifdef PBL_COLOR
		graphics_context_set_stroke_color(ctx, (GColor)primary_color);
		graphics_context_set_text_color(ctx, (GColor)primary_color);
		graphics_context_set_fill_color(ctx, (GColor)back_color);
		graphics_fill_rect(ctx, GRect(0,0,144,168), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, (GColor)primary_color);
	#else
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(0,0,144,168), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
	
	int mon = currentTime->tm_mon;
	int year = currentTime->tm_year + 1900;
	int daysThisMonth = daysInMonth(mon, year);

	int calendar[21];
	int cellNum = 0;
	int daysVisPrevMonth = 0;
    int daysPriorToToday = 7 + currentTime->tm_wday;
    int today = currentTime->tm_mday;

    if (daysPriorToToday >= today) {
        // We're showing more days before today than exist this month
        int daysInPrevMonth = daysInMonth(mon - 1, year); // year only matters for February, which will be the same 'from' March

        // Number of days we'll show from the previous month
        daysVisPrevMonth = daysPriorToToday - today + 1;

        for (int i = 0; i < daysVisPrevMonth; i++, cellNum++) {
            calendar[cellNum] = daysInPrevMonth + i - daysVisPrevMonth + 1;
        }
    }

    int firstDayShownThisMonth = daysVisPrevMonth + today - daysPriorToToday;

    // the current day's cell... we'll style this special
    int currentDay = cellNum + today - firstDayShownThisMonth;

    // Add days from this month and the next.
    int day = firstDayShownThisMonth;
    for (; cellNum < 21; cellNum++) {
        calendar[cellNum] = day;

        day++;
        if (day > daysThisMonth) {
            // Start at the beginning of next month.
            // We don't care how many days are in next month because
            // we will always show less than two weeks of it.
            day = 1;
        }
    }

    // ---------------------------
    // Now that we've calculated which days go where, we'll move on to the display logic.
    // ---------------------------

    // Cell geometry

    int left = 2;      // position of left side of left column
    int bottom = ROW_BASELINE + 12;    // position of bottom of bottom row
    int d = 7;      // number of columns (days of the week)
    int lw = 20;    // width of columns
    int w = 3;      // always display 3 weeks: previous, current, next

    int bh = 20;    // How tall rows should be depends on how many weeks there are

    int right = left + d * lw; // position of right side of right column
    int top = bottom - w * bh; // position of top of top row
    int cw = lw - 1; // width of textarea
    int cl = left + 1;
    int ch = bh - 1;

	/*
    // Draw the grid.
    if (grid) {
        graphics_context_set_stroke_color(ctx, foreground);

        // horizontal lines
        for (int i = 1; i <= w; i++) {
            graphics_draw_line(ctx, GPoint(left, bottom - i * bh), GPoint(right, bottom - i * bh));
        }

        // vertical lines
        for (int i = 1; i < d; i++) {
            graphics_draw_line(ctx, GPoint(left + i * lw, top), GPoint(left + i * lw, bottom));
        }
    }
	*/

    // Draw days of week
    for (int i = 0; i < 7; i++) {
        // Adjust labels by specified offset.
        int day = i;
        if (day > 6) {
            day -= 7;
        }

        graphics_draw_text(
            ctx,
            daysOfWeek[day],
            font_retro_micro,
            GRect(cl + i * lw, -5, cw, 20),
            GTextOverflowModeWordWrap,
            GTextAlignmentCenter,
            NULL);
    }

    // Fill in the cells with the month days
    cellNum = 0;
    for (int wknum = 0; wknum < 3; wknum++) {
        for (int dow = 0; dow < 7; dow++) {
            // Is this today?  If so prep special today style
            int fh = 18;

            if (cellNum == currentDay) {
				#ifdef PBL_COLOR
					graphics_context_set_text_color(ctx, (GColor)back_color);
				#else
					graphics_context_set_text_color(ctx, GColorWhite);
				#endif

				graphics_fill_rect(ctx,
								   GRect(
									   (left + dow * lw + 1) - 3,
									   (top + bh * wknum + 1) + 4,
									   cw + 3,
									   ch + 3),
								   0,
								   GCornerNone);
            } else {
                // Normal (non-today) style
				#ifdef PBL_COLOR
					graphics_context_set_text_color(ctx, (GColor)primary_color);
				#else
					graphics_context_set_text_color(ctx, GColorBlack);
				#endif
            }

            // Draw the day
            char date_text[3];
            snprintf(date_text, sizeof(date_text), "%d", calendar[cellNum]);
            graphics_draw_text(
                ctx,
                date_text,
                font_retro_micro,
                GRect(
                    cl + dow * lw,
                    top + bh / 2 + bh * wknum - fh / 2,
                    cw,
                    fh),
                GTextOverflowModeWordWrap,
                GTextAlignmentCenter,
                NULL);

            cellNum++;
        }
    }
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);
	
	// TODO: Add configuration stuff here - colors and such
  	set_colors();
	
	layer_back = layer_create(GRect(0,0, bounds.size.w, TIME_HEIGHT));
	layer_set_update_proc(layer_back, draw_layer_back);
	layer_add_child(window_layer, layer_back);
	
	// Info: Font Source - http://tenbytwenty.com/?xxxx_posts=munro
	font_retro_large = fonts_load_custom_font(
		resource_get_handle(RESOURCE_ID_FONT_MUNRO_59));
	font_retro_small = fonts_load_custom_font(
		resource_get_handle(RESOURCE_ID_FONT_MUNRO_27));
	font_retro_micro = fonts_load_custom_font(
		resource_get_handle(RESOURCE_ID_FONT_MUNRO_20));

	text_time = text_layer_create(GRect(7,-2,bounds.size.w - 7, 62));
	text_layer_set_background_color(text_time, GColorClear);
	
	text_date = text_layer_create(GRect(6,64,bounds.size.w - 6, 30));
	text_layer_set_background_color(text_date, GColorClear);
	
	#ifdef PBL_COLOR
		text_layer_set_text_color(text_time, (GColor)back_color);
		text_layer_set_text_color(text_date, (GColor)back_color);
	#else
		text_layer_set_text_color(text_time, GColorWhite);
		text_layer_set_text_color(text_date, GColorWhite);
	#endif
	
	text_layer_set_font(text_time, font_retro_large);
	text_layer_set_font(text_date, font_retro_small);
	text_layer_set_text_alignment(text_time, GTextAlignmentCenter);
	text_layer_set_text_alignment(text_date, GTextAlignmentCenter);
	
	layer_add_child(layer_back,
					text_layer_get_layer(text_time));
	layer_add_child(layer_back,
					text_layer_get_layer(text_date));
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	layer_calendar = layer_create(GRect(0,100, bounds.size.w, bounds.size.h));
	layer_set_update_proc(layer_calendar, draw_layer_calendar);
	layer_add_child(window_layer, layer_calendar);
}

static void window_unload(Window *window) { }

static void init(void) {
	window_main = window_create();
	window_set_window_handlers(window_main, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window_main, true);
	update_time();
	currentTime = get_time();
	accel_tap_service_subscribe(tap_handler);
}

static void deinit(void) {
  	animation_unschedule_all();
	layer_destroy(layer_back);
	layer_destroy(layer_calendar);
	text_layer_destroy(text_time);
	text_layer_destroy(text_date);
	fonts_unload_custom_font(font_retro_large);
	fonts_unload_custom_font(font_retro_small);
	window_destroy(window_main);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}