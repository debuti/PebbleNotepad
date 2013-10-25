/**
 *******************************************************************************
 *  Author: <a href="mailto:debuti@gmail.com">Borja Garcia</a>
 * Program: notepad
 * Descrip: Notepad for pebble
 * Version: 0.0.0
 *    Date: 20131024
 * License: This program doesn't require any license since it's not intended to
 *          be redistributed. In such case, unless stated otherwise, the purpose
 *          of the author is to follow GPLv3.
 * Version: 
 *          0.0.0 (20131024)
 *           - Initial release
 *******************************************************************************
 */

///////////////////////////INCLUDES///////////////////////////
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "mini-printf.h"

	
	
///////////////////////////DECLARATIONS///////////////////////////
#define MY_UUID { 0x3B, 0x1C, 0x09, 0x0A, 0x31, 0xED, 0x46, 0x14, 0xA1, 0xF7, 0x91, 0x93, 0xAC, 0xDD, 0x61, 0x3E }
PBL_APP_INFO(MY_UUID,
             "Notepad", 
             "ganian.tk",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

//CONSTANTS
#define LARGE FONT_KEY_GOTHIC_24
#define MEDIUM FONT_KEY_GOTHIC_18
#define SMALL FONT_KEY_GOTHIC_14
	
//Config this to fit your needs. 
// Remember to add the resources and change NUM_NOTES 
#define NUM_NOTES 2
#define FONT_TYPE MEDIUM
#define PIXELS_PER_CLICK 130;

//More constants
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS NUM_NOTES

	
//GLOBALS
uint32_t note_selected;
	

//WINDOWS
// This is the main window, shows a list of notes
Window main_window;
// This is a menu layer, you have more control than with a simple menu layer
MenuLayer menu_layer;
// This is the note window, shows only one note
Window note_window;
// This is a scroll layer to handle big texts
ScrollLayer scroll_layer;
// This is the note itself
TextLayer text_layer;


///////////////////////////NOTE WINDOW///////////////////////////
void up_single_click_handler(ClickRecognizerRef recognizer, Window *winder) {
	GPoint 	offset = scroll_layer_get_content_offset(&scroll_layer);
	offset.y = offset.y + PIXELS_PER_CLICK;
	scroll_layer_set_content_offset	(&scroll_layer,
									 offset,
									 true);
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *winder) {
	GPoint 	offset = scroll_layer_get_content_offset(&scroll_layer);
	offset.y = offset.y - PIXELS_PER_CLICK;
	scroll_layer_set_content_offset	(&scroll_layer,
									 offset,
									 true);
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *winder) {
	//Init or pause down scrolling

	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###select_single_click_handler: Entering###\n");
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###select_single_click_handler: Exiting###\n");

}


void config_provider(ClickConfig **config, Window *winder)
{
    config[BUTTON_ID_UP]->click.handler = (ClickHandler)up_single_click_handler;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler)down_single_click_handler;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler)select_single_click_handler;
    //config[BUTTON_ID_BACK]->click.handler = (ClickHandler)back_single_click_handler;
	//TODO: Multiclick select sets a plain clock, to exit that screen Up+Down
	//TODO: Longpress on up goes to the top, down to the bottom
	
    /*config[BUTTON_ID_UP]->multi_click.handler = (ClickHandler)up_multi_click_handler;
    config[BUTTON_ID_DOWN]->multi_click.handler = (ClickHandler)down_multi_click_handler;
    config[BUTTON_ID_SELECT]->multi_click.handler = (ClickHandler)select_multi_click_handler;
    config[BUTTON_ID_BACK]->multi_click.handler = (ClickHandler)back_multi_click_handler;
    config[BUTTON_ID_UP]->long_click.handler = (ClickHandler)up_long_click_handler;
    config[BUTTON_ID_DOWN]->long_click.handler = (ClickHandler)down_long_click_handler;
    config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler)select_long_click_handler;
    config[BUTTON_ID_BACK]->long_click.handler = (ClickHandler)back_long_click_handler;*/
}


void note_window_load(Window *me) { 
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###note_window_load: Entering###\n");
	
	
	const GRect max_text_bounds = GRect(0, 0, 144, 2000); // 2000 lines of text
	
	// Initialize the scroll layer
	scroll_layer_init(&scroll_layer, 
					  me->layer.bounds); // Window is 144x168
	
	// This binds the scroll layer to the window so that up and down map to scrolling
	// You may use scroll_layer_set_callbacks to add or override interactivity
	//scroll_layer_set_click_config_onto_window(&scroll_layer, 
	//										  me);
	
	// Set the initial max size
	scroll_layer_set_content_size(&scroll_layer, 
								  max_text_bounds.size);
	
	// Initialize the text layer and load text
	text_layer_init(&text_layer, 
					max_text_bounds);
	/*
	#define TEXT_BUFFER_LEN 20
	char note_view[TEXT_BUFFER_LEN];
	char read_buffer[TEXT_BUFFER_LEN];	
	resource_load_byte_range(resource_get_handle(note_selected),
							 0,
							 (uint8_t*)read_buffer, // Pointer to note char array
							 TEXT_BUFFER_LEN);
	mini_snprintf(note_view, 
				  TEXT_BUFFER_LEN, 
				  "%s", 
				  read_buffer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###note_window_load: Loaded preview %s###\n", 
			note_view);
	*/	
	//Transform 0x0d 0x0a to 0x20 0x\n
	//text_transform(note_view);
	
	//Set text to the layer
	text_layer_set_text(&text_layer, 
						"Julandronooo\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\nJulandrona\n");
						//note_view);
		/*
	// Change the font to a nice readable one
	text_layer_set_font(&text_layer, 
						fonts_get_system_font(FONT_TYPE));

	// Trim text layer and scroll content to fit text box
	GSize max_size = text_layer_get_max_used_size(app_get_current_graphics_context(), 
												  &text_layer);
	text_layer_set_size(&text_layer, 
						max_size);
						
    const int vert_scroll_text_padding = 4;
	scroll_layer_set_content_size(&scroll_layer, 
								  GSize(144, max_size.h + vert_scroll_text_padding));
	*/
	// Add the layers for display
	scroll_layer_add_child(&scroll_layer, 
						   &text_layer.layer);
	
	layer_add_child(&me->layer, //Root layer of the window
					&scroll_layer.layer);
	
    window_set_click_config_provider(&note_window, 
									 (ClickConfigProvider)config_provider);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###note_window_load: Exiting###\n");
}


void note_window_unload(Window *me) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###note_window_unload: Entering###\n");
	
	

	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###note_window_unload: Exiting###\n");
}

///////////////////////////MAIN WINDOW///////////////////////////
// This function links numbers with resources
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
	

// This callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
  return NUM_MENU_SECTIONS;
}


// Each section has a number of items; we use a callback to specify this
// You can also dynamically add and remove items using this
uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	switch (section_index) {
        case 0:
            return NUM_FIRST_MENU_ITEMS;

        default:
            return 0;
	}
}


// A callback is used to specify the height of the section header
int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data) {
	// This is a define provided in pebble_os.h that you may use for the default height
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}


// Here we draw what each header is
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

	
// This is the menu item draw callback where you specify what each item should look like
void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, 
	//		  "\n###menu_draw_row_callback: Entering###\n");
	//APP_LOG(APP_LOG_LEVEL_DEBUG, 
	//		"\n###menu_draw_row_callback: Section %d, row %d###\n", 
	//		cell_index->section, 
	//		cell_index->row);
	
	// Determine which section we're going to draw in
	switch (cell_index->section) {
	    case 0:
			if (cell_index->row < NUM_NOTES) {
				uint32_t resource_name = row_to_resource(cell_index->row);
				
				//APP_LOG(APP_LOG_LEVEL_DEBUG, 
				//		"\n###menu_draw_row_callback: Resource_name %d###\n", 
				//		resource_name);
				
				#define TITLE_BUFFER_LEN 30
				char note_title[TITLE_BUFFER_LEN];		
				char note_preview[TITLE_BUFFER_LEN];	
				char read_buffer[TITLE_BUFFER_LEN];	
				
				mini_snprintf(note_title, 
							  TITLE_BUFFER_LEN, 
							  "Note %d", 
							  cell_index->row);
				// Retrieve resource
				resource_load_byte_range(resource_get_handle(resource_name),
										 0,
										 (uint8_t*)read_buffer,  // Pointer to note char array
										 TITLE_BUFFER_LEN);
				// mini_snprintf to add endline character
				mini_snprintf(note_preview, 
							  TITLE_BUFFER_LEN, 
							  "%s", 
							  read_buffer);
				
				//APP_LOG(APP_LOG_LEVEL_DEBUG, 
				//		"\n###menu_draw_row_callback: Loaded preview %s###\n", 
				//		note_preview);
				
				menu_cell_basic_draw(ctx, 
									 cell_layer, 
									 note_title, 
									 note_preview, 
									 NULL);
		    }
            break;
	}
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "\n###menu_draw_row_callback: Exiting###\n");
}


// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *me, MenuIndex *cell_index, void *data) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###menu_select_callback: Entering###\n");
	APP_LOG(APP_LOG_LEVEL_INFO, 
			"\n###menu_select_callback: Item selected section %d, row %d###\n", 
			cell_index->section, 
			cell_index->row);
	
	note_selected = row_to_resource(cell_index->row);
		
	// Initialize main window but dont push it
	window_init(&note_window, 
				"Note");
	// Setup window handlers
	window_set_window_handlers(&note_window, 
							   (WindowHandlers){
									.load = note_window_load,
								    .unload = note_window_unload,
                               }
							  );
	//TODO: Es posible que haya que usar los handlers de appear y dissapear
	//Push!
	window_stack_push(&note_window, 
					  true /* Animated */);

	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###menu_select_callback: Exiting###\n");
}


// This initializes the menu upon main_window load
void main_window_load(Window *me) {
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###main_window_load: Entering###\n");
	// Now we prepare to initialize the menu layer
	// We need the bounds to specify the menu layer's viewport size
	// In this case, it'll be the same as the window's
	GRect bounds = me->layer.bounds;

	// Initialize the menu layer
	menu_layer_init(&menu_layer, 
					bounds);

	// Set all the callbacks for the menu layer
	menu_layer_set_callbacks(&menu_layer, 
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
	menu_layer_set_click_config_onto_window(&menu_layer, 
											me);

	// Add it to the main_window for display
	layer_add_child(&me->layer, 
					menu_layer_get_layer(&menu_layer));

	window_deinit(&note_window);
			
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###main_window_load: Exiting###\n");
}


void main_window_unload(Window *me) {
}



///////////////////////////ENTRY POINT///////////////////////////
void handle_init(AppContextRef ctx) {
	// Load the resources
	resource_init_current_app(&APP_RESOURCES);

	// Initialize main window and push it to the front of the screen
	window_init(&main_window, 
				"Notepad");
	window_stack_push(&main_window, 
					  true /* Animated */);
	// Setup the window handlers
	window_set_window_handlers(&main_window, 
							   (WindowHandlers){
									.load = main_window_load,
								    .unload = main_window_unload,
                               }
							  );
	
	
}


void pbl_main(void *params) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, 
			"\n###Initializing notepad###\n");
	PebbleAppHandlers handlers = {
        .init_handler = &handle_init
	};
	app_event_loop(params, 
				   &handlers);
}
