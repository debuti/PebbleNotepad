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

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0x3B, 0x1C, 0x09, 0x0A, 0x31, 0xED, 0x46, 0x14, 0xA1, 0xF7, 0x91, 0x93, 0xAC, 0xDD, 0x61, 0x3E }
PBL_APP_INFO(MY_UUID,
             "Notepad", 
             "ganian.tk",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

//Config this!
#define NUM_NOTES 2

//Constants
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS NUM_NOTES


Window window;

// This is a menu layer
// You have more control than with a simple menu layer
MenuLayer menu_layer;

// You can draw arbitrary things in a menu item such as a background
HeapBitmap menu_background;


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


uint32_t row_to_resource(int row) {
	switch (row) {
		case 0:    return RESOURCE_ID_NOTE1;
		case 1:    return RESOURCE_ID_NOTE2;
/*		case 2:    return RESOURCE_ID_NOTE3;
		case 3:    return RESOURCE_ID_NOTE4;
		case 4:    return RESOURCE_ID_NOTE5;
		case 5:    return RESOURCE_ID_NOTE6;
		case 6:    return RESOURCE_ID_NOTE7;
		case 7:    return RESOURCE_ID_NOTE8;
		case 8:    return RESOURCE_ID_NOTE9;*/
	}
	return RESOURCE_ID_NOTE1;
}
	
	
// This is the menu item draw callback where you specify what each item should look like
void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	// Determine which section we're going to draw in
	switch (cell_index->section) {
	    case 0:
			if (cell_index->row < NUM_NOTES) {
				uint32_t resource_name = row_to_resource(cell_index->row);
				#define TITLE_BUFFER_LEN 10
				char note_title[TITLE_BUFFER_LEN];		
				char note_preview[TITLE_BUFFER_LEN];		
				sprintf(note_title, "Note %d", cell_index->row);
				resource_load_byte_range(resource_get_handle(resource_name),
										 0,
										 (uint8_t*)note_preview,  // Pointer to note1 char array
										 TITLE_BUFFER_LEN);
				menu_cell_basic_draw(ctx, 
									 cell_layer, 
									 note_title, 
									 (char*)note_preview, 
									 NULL);
		    }
            break;
	}
}


// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *me, MenuIndex *cell_index, void *data) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s %d", "Item selected", cell_index->row);
	// Use the row to specify which item will receive the select action
	switch (cell_index->row) {
        // This is the menu item with the cycling icon
        case 1:
            // After changing the icon, mark the layer to have it updated
            layer_mark_dirty(&me->scroll_layer.layer);
            break;
	}

}


// This initializes the menu upon window load
void window_load(Window *me) {
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

	// Add it to the window for display
	layer_add_child(&me->layer, 
					menu_layer_get_layer(&menu_layer));
}


void window_unload(Window *me) {
	// And cleanup the background
	heap_bitmap_deinit(&menu_background);
}


void handle_init(AppContextRef ctx) {
	// Load the resources
	resource_init_current_app(&APP_RESOURCES);

	// Initialize main window and push it to the front of the screen
	window_init(&window, 
				"Notepad");
	window_stack_push(&window, 
					  true /* Animated */);

	// Setup the window handlers
	window_set_window_handlers(&window, 
							   (WindowHandlers){
									.load = window_load,
								    .unload = window_unload,
                               }
							  );
}


void pbl_main(void *params) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", "Hola mundo");
	PebbleAppHandlers handlers = {
        .init_handler = &handle_init
	};
	app_event_loop(params, 
				   &handlers);
}
