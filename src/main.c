/**
 *******************************************************************************
 *  Author: <a href="mailto:debuti@gmail.com">Borja Garcia</a>
 * Program: notepad
 * Descrip: Notepad for pebble
 * Version: 0.2
 *    Date: 20131219
 * License: This program doesn't require any license since it's not intended to
 *          be redistributed. In such case, unless stated otherwise, the purpose
 *          of the author is to follow GPLv3.
 * Versions: 
 *          0.2 (20131219)
 *           - Pebble 2.0 supported
 *          0.1.3 (20131104)
 *           - Added clock window pop combination
 *          0.1.2 (20131026)
 *           - Added auto scroll
 *           - Added long_click scroll
 *           - Added multi_click scroll
 *          0.1.1 (20131026)
 *           - Added clock
 *           - Fixed bugs
 *          0.1.0 (20131025)
 *           - Initial release
 *******************************************************************************
 */

//TODO:Password protected notes
//TODO:NoteDef struct {Title, Password, Font}

///////////////////////////INCLUDES///////////////////////////
#include "mini-printf.h"
#include "pebble.h"
#include <time.h>
#include "pebble-log.h"
	
///////////////////////////DECLARATIONS///////////////////////////
//CONSTANTS
#define LARGE FONT_KEY_GOTHIC_24
#define MEDIUM FONT_KEY_GOTHIC_18
#define SMALL FONT_KEY_GOTHIC_14
#define UP 1
#define DOWN 2
#define AUTO 3
	
//Config this to fit your needs. 
// Remember to add the resources and change NUM_NOTES 
#define NUM_NOTES 2
#define FONT_TYPE SMALL
#define PIXELS_PER_CLICK 100
#define PIXELS_PER_LONG_CLICK 6
#define LONG_CLICK_DELAY 100
#define PIXELS_PER_AUTO_SCROLL 3
#define AUTO_SCROLL_DELAY 100
#define TEXT_BUFFER_LEN 10000
#define ALLOW_FAKE_CLOCK 1

//More constants
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS NUM_NOTES

	
//GLOBALS
char note_view[TEXT_BUFFER_LEN];
uint32_t note_selected;
size_t note_selected_size;
bool long_click_running = false;
bool auto_scroll_running = false;
AppTimer *timer_handle;

//WINDOWS
// This is the main window, shows a list of notes
Window *main_window;
// This is a menu layer, you have more control than with a simple menu layer
MenuLayer *menu_layer;

// This is the note window, shows only one note
Window *note_window;
// This is a scroll layer to handle big texts
ScrollLayer *scroll_layer;
// This is the note itself
TextLayer *text_layer;

// This is the fake clock window, to hide the note if necessary hehe
Window *clock_window;
TextLayer *clock_text;
#define TIME_STR_BUFFER_BYTES 32
char s_time_str_buffer[TIME_STR_BUFFER_BYTES];
int state_machine = 0; //UP+DOWN+UP+DOWN+SELECT to exit clock





///////////////////////////    CODE   ///////////////////////////
///////////////////////////CLOCK WINDOW///////////////////////////

#if ALLOW_FAKE_CLOCK == 1
void up_single_click_clock_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###up_single_click_clock_window_handler: %d###", state_machine);
	switch (state_machine) {
		case 0: state_machine = 1; break;
		case 2: state_machine = 3; break;
		default: state_machine = 0; break;
	}
}

void down_single_click_clock_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###down_single_click_clock_window_handler: %d###", state_machine);
	switch (state_machine) {
		case 1: state_machine = 2; break;
		case 3: state_machine = 4; break;
		default: state_machine = 0; break;
	}
}

void select_single_click_clock_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###select_single_click_clock_window_handler: %d###", state_machine);
    if (state_machine == 4) {
	    window_stack_pop(true);
	}
	state_machine = 0;
}

void back_single_click_clock_window_handler(ClickRecognizerRef recognizer, void *context) {
}

void clock_config_provider(Window *window) {

    window_single_click_subscribe(BUTTON_ID_UP, up_single_click_clock_window_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_clock_window_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_clock_window_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_clock_window_handler);
	
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
      // Handle tick only if current window is the clock
        if (window_stack_get_top_window() == clock_window) {
                strftime(s_time_str_buffer, 
                                   TIME_STR_BUFFER_BYTES, 
                                   "%I %M %p", 
                                   tick_time);
                text_layer_set_text(clock_text, s_time_str_buffer);
        }
    }
	
void clock_window_load(Window *me) {

	Layer *clock_window_layer = window_get_root_layer(clock_window);
	
	// Format text leayer
	clock_text = text_layer_create(GRect(40, 30, 64, 138));
	text_layer_set_overflow_mode(clock_text,
								 GTextOverflowModeWordWrap);
	text_layer_set_text_alignment(clock_text, 
								  GTextAlignmentCenter);
    text_layer_set_font(clock_text, 
						fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));

	// Fill clock text
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
    strftime(s_time_str_buffer, TIME_STR_BUFFER_BYTES, "%I %M %p", now);
	text_layer_set_text(clock_text, 
						s_time_str_buffer);
						
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	
	layer_add_child(clock_window_layer, 
					text_layer_get_layer(clock_text));
	
    window_set_click_config_provider(clock_window, 
									 (ClickConfigProvider)clock_config_provider);

}
void clock_window_unload(Window *me) {
	tick_timer_service_unsubscribe();
	text_layer_destroy(clock_text);
	window_destroy(clock_window);
}


#endif


	
///////////////////////////NOTE WINDOW///////////////////////////
  /**
   *  Goes one screen up
   */
void up_single_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	GPoint offset = scroll_layer_get_content_offset(scroll_layer);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###up_single_click_note_window_handler: offset.y %d###", offset.y);
	offset.y = offset.y + PIXELS_PER_CLICK;
	scroll_layer_set_content_offset	(scroll_layer,
									 offset,
									 true);
}

  /**
   *  Goes one screen down
   */
void down_single_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	GPoint offset = scroll_layer_get_content_offset(scroll_layer);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###down_single_click_note_window_handler: offset.y %d###", offset.y);
	offset.y = offset.y - PIXELS_PER_CLICK;
	scroll_layer_set_content_offset	(scroll_layer,
									 offset,
									 true);
}

  /**
   *  Goes to the top
   */
void up_multi_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###up_multi_click_note_window_handler: Entering###");
	GPoint offset = scroll_layer_get_content_offset(scroll_layer);
	offset.y = 0;
	scroll_layer_set_content_offset	(scroll_layer,
									 offset,
									 true);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###up_multi_click_note_window_handler: Exiting###");
}	

  /**
   *  Goes to the bottom
   */
void down_multi_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###down_multi_click_note_window_handler: Entering###");
	GSize size = scroll_layer_get_content_size(scroll_layer);
	GPoint offset = scroll_layer_get_content_offset(scroll_layer);
	offset.y = -1 * size.h;
	scroll_layer_set_content_offset	(scroll_layer,
									 offset,
									 true);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###down_multi_click_note_window_handler: Exiting###");
}	

  /**
   *  Handles ticks when autoscrolling or long pushing
   */
void handle_timer(void *data) {
	
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###handle_timer: long_click_running %d###", long_click_running);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###handle_timer: auto_scroll_running %d###", auto_scroll_running);
	
	int cookie = (int) data;
	
	if (long_click_running) {
		GPoint offset = scroll_layer_get_content_offset(scroll_layer);
		if (cookie == UP) {
			offset.y = offset.y + PIXELS_PER_LONG_CLICK;
		}
		if (cookie == DOWN) {		
			offset.y = offset.y - PIXELS_PER_LONG_CLICK;
		}
	
		scroll_layer_set_content_offset	(scroll_layer,
										 offset,
										 true);
		timer_handle = app_timer_register(LONG_CLICK_DELAY, handle_timer, data);
	}
	if (auto_scroll_running) {
		GPoint offset = scroll_layer_get_content_offset(scroll_layer);
		offset.y = offset.y - PIXELS_PER_AUTO_SCROLL;
		scroll_layer_set_content_offset	(scroll_layer,
										 offset,
										 true);
		timer_handle = app_timer_register(AUTO_SCROLL_DELAY, handle_timer, data);
	}
	
}

  /**
   *  Goes up smoothly
   */
void up_long_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	long_click_running = true;
	timer_handle = app_timer_register(LONG_CLICK_DELAY, handle_timer, (void *)UP);
}

  /**
   *  Goes down smoothly
   */
void down_long_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	long_click_running = true;
	timer_handle = app_timer_register(LONG_CLICK_DELAY, handle_timer, (void *)DOWN);
}

  /**
   *  Stops all the smoothiness
   */
void release_long_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	long_click_running = false;
}

  /**
   *  Starts/Stops autoscrolling
   */
void select_single_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###select_single_click_note_window_handler: auto_scroll_running %d###", auto_scroll_running);
	auto_scroll_running = !auto_scroll_running;
	if (auto_scroll_running) {
		//window_set_status_bar_icon(&note_window,
		//							 AUTO );
		timer_handle = app_timer_register(100, handle_timer, (void *)AUTO);
	}
	else {
		//window_set_status_bar_icon(&note_window,
		//							 NORMAL );
		app_timer_cancel(timer_handle);
	}
}

  /**
   *  If enabled, goes to fake clock (tm)
   */
void select_multi_click_note_window_handler(ClickRecognizerRef recognizer, void *context) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###select_multi_click_note_window_handler: Entering###");
		
#if ALLOW_FAKE_CLOCK == 1
	// Format and push window
	clock_window = window_create();
	window_set_fullscreen(clock_window,
						  true);
	
	window_set_window_handlers(clock_window, 
							   (WindowHandlers){
									.load   = clock_window_load,
								    .unload = clock_window_unload,
                               }
							  );  
							  
	window_stack_push(clock_window, 
					  true);
#endif
	
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###select_multi_click_note_window_handler: Exiting###");
}

  /**
   *  Set up all bottom handlers stuff
   */
void note_config_provider(Window *window) {
    window_single_click_subscribe(BUTTON_ID_UP, up_single_click_note_window_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_note_window_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_note_window_handler);

	window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 10, 500, true, select_multi_click_note_window_handler);
	window_multi_click_subscribe(BUTTON_ID_UP, 2, 10, 100, true, up_multi_click_note_window_handler);
	window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 10, 100, true, down_multi_click_note_window_handler);
 
    window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_note_window_handler, release_long_click_note_window_handler);
    window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_note_window_handler, release_long_click_note_window_handler);
	
}


  /**
   *  Load the note window
   */
void note_window_load(Window *me) { 
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_load: Entering###");
	
	const GRect max_text_bounds = GRect(0, 0, 144, 20000); // 20000 pixels of text
	
	// Initialize the scroll layer
	Layer *note_window_layer = window_get_root_layer(me);
    GRect bounds = layer_get_bounds(note_window_layer);
	scroll_layer = scroll_layer_create(bounds); // Window is 144x168
	
	// Set the initial max size
	scroll_layer_set_content_size(scroll_layer, 
								  max_text_bounds.size);
	
	// Initialize the text layer and load text
	text_layer = text_layer_create(max_text_bounds);
	
	note_selected_size = resource_size(resource_get_handle(note_selected));
	if (TEXT_BUFFER_LEN < note_selected_size) note_selected_size = TEXT_BUFFER_LEN;
	note_selected_size = resource_load(resource_get_handle(note_selected),
									   (uint8_t*)note_view,
									   note_selected_size);
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_load: Readed resource bytes: %d ###", note_selected_size);
	
	//Transform 0x0d 0x0a to 0x20 0x\n
	//text_transform(note_view);
	
	//Set text to the layer
	text_layer_set_text(text_layer, 
						note_view);
		
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_load: post text_layer_set_text ###");
	
	// Change the font to a nice readable one
	text_layer_set_font(text_layer, 
						fonts_get_system_font(FONT_TYPE));

	// Trim text layer and scroll content to fit text box
	GSize max_size = text_layer_get_content_size(text_layer);
	text_layer_set_size(text_layer, 
						max_size);
						
    const int vert_scroll_text_padding = 4;
	scroll_layer_set_content_size(scroll_layer, 
								  GSize(144, max_size.h + vert_scroll_text_padding));
	
	// Add the layers for display
	scroll_layer_add_child(scroll_layer, 
						   text_layer_get_layer(text_layer));
	
	layer_add_child(note_window_layer, //Root layer of the window
					scroll_layer_get_layer(scroll_layer));
	
		//window_set_status_bar_icon(&note_window,
		//							 NORMAL );
	
    window_set_click_config_provider(note_window, 
									 (ClickConfigProvider)note_config_provider);
	
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_load: Exiting###");
}

  /**
   *  Unload the note window
   */
void note_window_unload(Window *me) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_unload: Entering###");
	 
	int note_view_len = strlen(note_view);
	for (int i=0; i<note_view_len; i++) {
      note_view[i] = 0;
    }
    text_layer_destroy(text_layer);
    scroll_layer_destroy(scroll_layer);
    window_destroy(note_window);
	
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###note_window_unload: Exiting###");
}
	

///////////////////////////MAIN WINDOW///////////////////////////

  /**
   *  This function links numbers with resources
   */
uint32_t row_to_resource(int row) {
	switch (row) {
#if NUM_NOTES > 0
		case 0:    return RESOURCE_ID_NOTE0;
#endif		
#if NUM_NOTES > 1
		case 1:    return RESOURCE_ID_NOTE1;
#endif		
#if NUM_NOTES > 2
		case 2:    return RESOURCE_ID_NOTE2;
#endif		
#if NUM_NOTES > 3
		case 3:    return RESOURCE_ID_NOTE3;
#endif		
#if NUM_NOTES > 4
		case 4:    return RESOURCE_ID_NOTE4;
#endif		
#if NUM_NOTES > 5
		case 5:    return RESOURCE_ID_NOTE5;
#endif		
#if NUM_NOTES > 6
		case 6:    return RESOURCE_ID_NOTE6;
#endif		
#if NUM_NOTES > 7
		case 7:    return RESOURCE_ID_NOTE7;
#endif		
#if NUM_NOTES > 8
		case 8:    return RESOURCE_ID_NOTE8;
#endif		
#if NUM_NOTES > 9
		case 9:    return RESOURCE_ID_NOTE9;
#endif		
	}
	return RESOURCE_ID_NOTE0;
}
	
  /**
   *  This callback is used to specify the amount of sections of menu items
   *  With this, you can dynamically add and remove sections
   */
uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
  return NUM_MENU_SECTIONS;
}


  /**
   *  Each section has a number of items; we use a callback to specify this
   *  You can also dynamically add and remove items using this
   */
uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	switch (section_index) {
        case 0:
            return NUM_FIRST_MENU_ITEMS;

        default:
            return 0;
	}
}


  /**
   *  A callback is used to specify the height of the section header
   */
int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data) {
	// This is a define provided in pebble_os.h that you may use for the default height
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}


  /**
   *  Here we draw what each header is
   */
void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	// Determine which section we're working with
	switch (section_index) {
        case 0:
            // Draw title text in the section header
            menu_cell_basic_header_draw(ctx,
										cell_layer, 
										"Your notes");
            break;
	}
}

	
  /**
   *  This is the menu item draw callback where you specify what each item should look like
   */
void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###menu_draw_row_callback: Entering###");
	// Determine which section we're going to draw in
	
	switch (cell_index->section) {
	    case 0:
			if (cell_index->row < NUM_NOTES) {
				uint32_t resource_name = row_to_resource(cell_index->row);
								
				#define TITLE_BUFFER_LEN 30
				char note_title[TITLE_BUFFER_LEN];		
				char note_preview[TITLE_BUFFER_LEN];	
				char read_buffer[TITLE_BUFFER_LEN];	
				
				// Retrieve resource
				resource_load(resource_get_handle(resource_name),
							  (uint8_t*)read_buffer,
							  TITLE_BUFFER_LEN);	
				
				// mini_snprintf to add endline character to note_preview
				mini_snprintf(note_preview, 
							  TITLE_BUFFER_LEN, 
							  "%s", 
							  read_buffer);
	
				// mini_snprintf to add endline character and format to note_title
				mini_snprintf(note_title, 
							  TITLE_BUFFER_LEN, 
							  "Note %d (%dB)", 
							  cell_index->row,
							  resource_size(resource_get_handle(resource_name)));
				
				menu_cell_basic_draw(ctx, 
									 cell_layer, 
									 note_title, 
									 note_preview, 
									 NULL);
		    }
            break;
	}
	
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###menu_draw_row_callback: Exiting###");
}


  /**
   *  Here we capture when a user selects a menu item
   */
void menu_select_callback(MenuLayer *me, MenuIndex *cell_index, void *data) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###menu_select_callback: Entering###");
	app_log(APP_LOG_LEVEL_INFO, "main.c", 0, "###menu_select_callback: Item selected section %d, row %d###", cell_index->section, cell_index->row);

	note_selected = row_to_resource(cell_index->row);
		
	// Initialize main window but dont push it
	note_window = window_create();
	
	// Setup window handlers
	window_set_window_handlers(note_window, 
							   (WindowHandlers){
									.load = note_window_load,
								    .unload = note_window_unload,
                               }
							  );
	
	//Push!
	window_stack_push(note_window, 
					  true);

	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###menu_select_callback: Exiting###");
}

  /**
   *  This initializes the menu upon main_window load
   */
void main_window_load(Window *me) {
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###main_window_load: Entering###");
	
	// Now we prepare to initialize the menu layer
    Layer *main_window_layer = window_get_root_layer(me);
    GRect bounds = layer_get_bounds(main_window_layer);
    
	// Initialize the menu layer
	menu_layer = menu_layer_create(bounds);

	// Set all the callbacks for the menu layer
	menu_layer_set_callbacks(menu_layer, 
							 NULL, 
							 (MenuLayerCallbacks){
								.get_num_sections = menu_get_num_sections_callback,
								.get_num_rows = menu_get_num_rows_callback,
								.get_header_height = menu_get_header_height_callback,
								.draw_header = menu_draw_header_callback,
								.draw_row = menu_draw_row_callback,
								.select_click = menu_select_callback,
	                         }
							);

	// Bind the menu layer's click config provider to the window for interactivity
	menu_layer_set_click_config_onto_window(menu_layer, 
											me);

	// Add it to the main_window for display
	layer_add_child(main_window_layer, 
					menu_layer_get_layer(menu_layer));
			
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###main_window_load: Exiting###");
}

  /**
   *  This unload the main window
   */
void main_window_unload(Window *me) {
    menu_layer_destroy(menu_layer);
    window_destroy(main_window);
}


///////////////////////////ENTRY POINT///////////////////////////
  /**
   *  Main
   */
void init() {	
    app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###init: Entering###");
	
	// Initialize main window and push it to the front of the screen
	main_window = window_create();
	
	// Setup the window handlers
	window_set_window_handlers(main_window, 
							   (WindowHandlers){
									.load   = main_window_load,
								    .unload = main_window_unload,
                               }
							  );  
							  
    window_stack_push(main_window, true);
							  
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###init: Exiting###");
}

void deinit() {	
    app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###deinit: Entering###");
	app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###deinit: Exiting###");
}

  /**
   *  Main
   */
int main(void) {
    app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###main: Entering###");
	
	init();
	app_event_loop();
	deinit();
	
    app_log(APP_LOG_LEVEL_DEBUG, "main.c", 0, "###main: Exiting###");
}

