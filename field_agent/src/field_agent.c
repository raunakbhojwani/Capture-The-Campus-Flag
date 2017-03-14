#include <pebble.h>
#include <stdbool.h>

// Local includes
#include "mission.h"
#include "pebble_strtok.h"
#include "pin_window.h"
#include "dialog_message_window.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ICONS 0
#define NUM_FIRST_MENU_ITEMS 4

#define NUM_GAME_MENU_SECTIONS 1
#define NUM_GAME_MENU_ICONS 0
#define NUM_GAME_MENU_ITEMS 3

#define NUM_STATUS_MENU_ITEMS 9


// window pointers declaration
static Window *s_starting_window;
static Window *s_team_window;
static Window *s_game_window;
static Window *s_status_window;

// menulayer pointers declaration
static MenuLayer *s_menu_layer;
static MenuLayer *s_game_menu_layer;
static MenuLayer *s_team_menu_layer;
static MenuLayer *s_status_menu_layer;

//in_game variable declarations
static char player_name[10];
static char game_id[10] = "0";
static char pebble_id[35];
static char team_name[20];
static char guide_id[10];
static char num_remain_code_drops[10];
static char num_friendly_ops[10];
static char num_foe_ops[10];
static char time_remaining[10];

static char capture_id[8];
static char code_id[8];

static char latitude[80];
static char longitude[80];

static char status_req[] = "1";

static char location_msg[8191];
static char neutralize_msg[8191];
static char capture_msg[8191];
static char game_over_msg[240]; 
static char codedrop_near_msg[240];
static char opponent_near_msg[240];

static bool is_capture = false;
static bool is_neutralize = false;

// static bool is_first_time = true;


/**tokenizeInput**/
/* Takes in a string, tokenizes it and returns a character array
 * 
 */
static int tokenize_input(char* message, char** msgArr, char* DELIM)
{
  // Check params
  if (message == NULL){
    return 0;
  }

  if (msgArr == NULL){
    return 0;
  }

  int wordcount = 0; 
  char* word = NULL;

  for (word = pebble_strtok(message, DELIM); word != NULL; word = pebble_strtok(NULL, DELIM)){
    // save in list of words
    char* newWord = word;
    msgArr[wordcount++] = newWord;
  }
  return wordcount;
}

static void send_app_message(int key, char* msg) {

  // Begin dictionary
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if(result == APP_MSG_OK) {
    // Add a key-value pair
    dict_write_cstring(iter, key, msg);

    // Send the message!
    result = app_message_outbox_send();

    if(result != APP_MSG_OK) {
      LOG("ERROR SENDING MESSAGE");
    }
    else {
      LOG("Message Sent!!");
    } 
  }
  else if (result == APP_MSG_BUSY) {
    LOG("RESULT = APP MSG BUSY");
  }
}

static void update_location() {
  // Begin dictionary
  DictionaryIterator *location_iter;
  AppMessageResult result = app_message_outbox_begin(&location_iter);

  if(result == APP_MSG_OK) {
    // Add a key-value pair
    dict_write_cstring(location_iter, AppKeyLocation, "0");

    // Send the message!
    result = app_message_outbox_send();

    if(result != APP_MSG_OK) {
      LOG("ERROR SENDING MESSAGE");
    }
    else {
      LOG("Message Sent!!");
    } 
  }
  else if (result == APP_MSG_BUSY) {
    LOG("RESULT != APP MSG BUSY");
  }
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
 
  Tuple *ready_tuple = dict_find(iterator, AppKeyJSReady);
  if(ready_tuple) {
      // Log the value sent as part of the received message.
      char* pebble_id_ptr = ready_tuple->value->cstring;
      snprintf(pebble_id, sizeof(pebble_id), "%s", pebble_id_ptr);
      LOG("Got AppKeyJSReady: %s", pebble_id);

      update_location();
  }

  Tuple *location_tuple = dict_find(iterator, AppKeyLocation);
  if(location_tuple) {
      // Log the value sent as part of the received message.
      char *location_str_ptr = location_tuple->value->cstring;
      LOG("Got AppKeyLocation: %s", location_str_ptr);

      char **msgArray = malloc(sizeof(char*) * 8);
      tokenize_input(location_str_ptr, msgArray, "|");

      snprintf(latitude, sizeof(latitude), "%s", msgArray[0]);
      snprintf(longitude, sizeof(longitude), "%s", msgArray[1]);
      free(msgArray);
      LOG("PEBBLE ID: %s", pebble_id);

  }

  Tuple *message_tuple = dict_find(iterator, AppKeyRecvMsg);
  if(message_tuple) {
      // Log the value sent as part of the received message.
      char *message_str = message_tuple->value->cstring;
      LOG("Got AppKeyRecvMsg: %s", message_str);

      char **msgArray = malloc(sizeof(char*) * 10);
      int arraylen = tokenize_input(message_str,msgArray, "|");

      for (int i =0; i<arraylen; i++) {
        LOG("msgArray: %s", msgArray[i]);
      }

      if (strcmp(msgArray[0], "GAME_STATUS") == 0) {
        snprintf(game_id, sizeof(game_id), "%s", msgArray[1]);
        snprintf(guide_id, sizeof(guide_id), "%s", msgArray[2]);
        snprintf(num_remain_code_drops, sizeof(num_remain_code_drops), "%s", msgArray[3]);
        snprintf(num_friendly_ops, sizeof(num_friendly_ops), "%s", msgArray[4]);
        snprintf(num_foe_ops, sizeof(num_foe_ops), "%s", msgArray[5]);
        snprintf(time_remaining, sizeof(time_remaining), "%s", msgArray[6]);
        LOG("GAME_STATUS|%s|%s|%s|%s|%s", game_id, guide_id, num_remain_code_drops, num_friendly_ops, num_foe_ops);
      }

      else if (strcmp(msgArray[0], "GS_CAPTURE_ID")== 0) {
        snprintf(capture_id, sizeof(capture_id), "%s", msgArray[2]);
        LOG("GS_CAPTURE_ID|%s|%s", game_id, capture_id);


        static char being_captured[200];
        snprintf(being_captured, sizeof(being_captured), "You are being captured! This is your capture ID: %s", capture_id);
        dialog_message_window_push(being_captured, true);

        //TODO: SOLVE SLEEP
        LOG("CAPTURING WINDOW LAUNCHED");
        // psleep(6000);

      }

      else if (strcmp(msgArray[0], "GA_HINT") == 0) {
        if (strcmp(game_id, msgArray[1]) == 0 && strcmp(team_name, msgArray[3]) == 0) {

          static char hint[200];
          snprintf(hint, sizeof(hint), "Incoming Hint: %s", msgArray[6]);

          dialog_message_window_push(hint, false);

        }
        LOG("%s", msgArray[4]);
      }

      else if (strcmp(msgArray[0], "GS_RESPONSE") == 0) {

        if (strcmp(msgArray[2], "MI_CAPTURE_SUCCESS") == 0) {
          LOG("SUCCESSFULLY CAPTURED");
          
          dialog_message_window_push("Opponent Successfully Captured!", false);
        }

        else if (strcmp(msgArray[2], "MI_CAPTURED") == 0) {
          LOG("YOU HAVE BEEN CAPTURED");
          
          window_stack_push(s_status_window, false);
          dialog_message_window_push("You have been captured!", false);
          window_stack_remove(s_game_window, false);
        }

        else if (strcmp(msgArray[2], "MI_NEUTRALIZED") == 0) {
          LOG("CODE DROP NEUTRALIZED");
          
          dialog_message_window_push("Code Drop Neutralized!", false);
        }

        else if(strcmp(msgArray[2], "MI_ERROR_INVALID_OPCODE") == 0) {
          LOG("INVALID OPCODE: %s", msgArray[3]);
        }

        else if (strcmp(msgArray[2], "MI_ERROR_INVALID_TEAMNAME") == 0) {
          LOG("INVALID TEAMNAME: %s", msgArray[3]);
        } 

        else if (strcmp(msgArray[2], "MI_ERROR_INVALID_GAME_ID") == 0) {
          LOG("INVALID GAME ID: %s", msgArray[3]);
        }

        else if (strcmp(msgArray[2], "MI_ERROR_INVALID_ID") == 0) {
          LOG("INVALID GUIDE OR PEBBLE ID: %s", msgArray[3]);
        }
      }

      else if (strcmp(msgArray[0], "GAME_OVER") == 0) {
        char* gameover_game_id = msgArray[1];
        if (strcmp(gameover_game_id, game_id) == 0) {
          snprintf(num_remain_code_drops, sizeof(num_remain_code_drops), "%s", msgArray[2]);
          LOG("GAME_OVER|%s|%s", game_id, num_remain_code_drops);
          // dialog_message_window_push("GAME OVER", false);


          if (msgArray[3] == NULL) {
            LOG("ERROR!!! NULL STUFF!");
          }
          else {
      	    char *team = pebble_strtok(msgArray[3],":");
      	    int teamNum = 0;
      	    char **teamArray = malloc(sizeof(char*) * strlen(msgArray[3]));  // don't forget to free this! idk how I did my freeing in a weird way for Arrays with words
	      	    
            while (team != NULL) {

              LOG("1");
      	      // allocate memory for the new word
      	      char* newTeam = malloc(sizeof(char*) *strlen(team)); // allocate memory
      	      strcpy(newTeam,team);
      	      teamArray[teamNum++] = newTeam;

      	      //do subfields
      	      char **teamSubField = malloc(sizeof(char*)*strlen(newTeam));
      	      tokenize_input(newTeam, teamSubField, ",");
      	      // do whatever you want using the subfield array and team datastructure
              LOG("NEW TEAM: %s, %s, %s, %s, %s", teamSubField[0], teamSubField[1],teamSubField[2],teamSubField[3],teamSubField[4]);

              snprintf(game_over_msg, sizeof(game_over_msg), "Game Over! Team: %s, Players: %s, Captures: %s, Captured: %s, Neutralized: %s.", teamSubField[0], teamSubField[1],teamSubField[2],teamSubField[3],teamSubField[4]);

              team = pebble_strtok(NULL, ":");

              free(teamSubField);
              free(newTeam);
	          }
            free(teamArray);
	         dialog_message_window_push(game_over_msg, false);
           window_stack_remove(s_game_window, false);
           window_stack_remove(s_game_window, false);
          }
        }
      }

      else if (strcmp(msgArray[0], "GS_RADAR") == 0) {
        LOG("I MADE IT TO RADAR");
        LOG("THIS IS ARRAYLEN %i", arraylen);
        if (arraylen == 5) {
          snprintf(opponent_near_msg, sizeof(opponent_near_msg), "Opponent is nearby! Name: %s Team: %s Lat: %s Long: %s", msgArray[4], msgArray[3], msgArray[1], msgArray[2]);
          dialog_message_window_push(opponent_near_msg, false);
        }

        else if (arraylen == 4) {
          snprintf(codedrop_near_msg, sizeof(codedrop_near_msg), "Code Drop nearby! Code: %s, Lat: %s, Long: %s", msgArray[1], msgArray[2], msgArray[3]);
          dialog_message_window_push(codedrop_near_msg, false);
        }
      }

      free(msgArray);

  }

  Tuple *error_tuple = dict_find(iterator, AppKeySendError);
  if(error_tuple) {
      // Log the value sent as part of the received message.
      char *error_str = error_tuple->value->cstring;
      LOG("Got AppKeySendError: %s", error_str);
  } 
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  LOG("Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  LOG("Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  LOG("Outbox send success!");
}

static void pin_complete_callback(PIN pin, void *context) {
  if (is_neutralize) {
    snprintf(code_id, sizeof(code_id), "%x%x%x%x", pin.digits[0], pin.digits[1], pin.digits[2], pin.digits[3]);
    snprintf(neutralize_msg, 8191, "FA_NEUTRALIZE|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, code_id);
    send_app_message(AppKeySendMsg, neutralize_msg);
    LOG("Neutralize Pin was %x %x %x %x", pin.digits[0], pin.digits[1], pin.digits[2], pin.digits[3]);
    LOG("NEUTRALIZE MESSAGE SENT!");
    // text_animation_window_push("Neutralize Message Sent!", "Press Back To Return");

    is_neutralize = false;
  }
  else if (is_capture) {
    snprintf(capture_id, sizeof(capture_id), "%x%x%x%x", pin.digits[0], pin.digits[1], pin.digits[2], pin.digits[3]);
    snprintf(capture_msg, 8191, "FA_CAPTURE|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, capture_id);
    send_app_message(AppKeySendMsg, capture_msg);
    LOG("Capturing Pin was %x %x %x %x", pin.digits[0], pin.digits[1], pin.digits[2], pin.digits[3]);
    LOG("CAPTURE MESSAGE SENT!");
    // text_animation_window_push("Capture Message Sent", "Press Back To Return");

    is_capture = false;
  }
  
  pin_window_pop((PinWindow*)context, true);
}



static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_FIRST_MENU_ITEMS;      
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Draw title text in the section header
  menu_cell_basic_header_draw(ctx, cell_layer, "Please choose a name");
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item we'll draw
    switch (cell_index->row) {
      case 0:
        // This is a basic menu item with a title and subtitle
        menu_cell_basic_draw(ctx, cell_layer, "Raunak", "Competitive warrior", NULL);
        break;
      case 1:
        // This is a basic menu icon with a cycling icon
        menu_cell_basic_draw(ctx, cell_layer, "Samuel", "Extraordinary mind", NULL);
        break;
      case 2: 
         menu_cell_basic_draw(ctx, cell_layer, "Max", "Brooding surprise", NULL);
        break;
      case 3:
        menu_cell_basic_draw(ctx, cell_layer, "Travis", "Intellectual force", NULL);
        break;
    }
}

static void team_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Draw title text in the section header
  menu_cell_basic_header_draw(ctx, cell_layer, "Please choose a team");
}

static void team_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item we'll draw
    switch (cell_index->row) {
      case 0:
        // This is a basic menu item with a title and subtitle
        menu_cell_basic_draw(ctx, cell_layer, "Artists", "Modern day maestros", NULL);
        break;
      case 1:
        // This is a basic menu icon with a cycling icon
        menu_cell_basic_draw(ctx, cell_layer, "Economists", "Supply and Demand", NULL);
        break;
      case 2: 
         menu_cell_basic_draw(ctx, cell_layer, "Hackers", "404 Page Not Found", NULL);
        break;
      case 3:
        menu_cell_basic_draw(ctx, cell_layer, "Literati", "Incredible intelligentsia", NULL);
        break;
    }
}

static uint16_t game_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_GAME_MENU_SECTIONS;
}

static uint16_t game_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_GAME_MENU_ITEMS;      
}

static int16_t game_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void game_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Draw title text in the section header
  menu_cell_basic_header_draw(ctx, cell_layer, "Mission Incomputable!");
}

static void game_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item we'll draw
    switch (cell_index->row) {
      case 0:
        // This is a basic menu item with a title and subtitle
        menu_cell_basic_draw(ctx, cell_layer, "NEUTRALIZE", "Neutralize a code drop", NULL);
        break;
      case 1:
        // This is a basic menu icon with a cycling icon
        menu_cell_basic_draw(ctx, cell_layer, "CAPTURE", "Capture an opponent", NULL);
        break;
      case 2: 
         menu_cell_basic_draw(ctx, cell_layer, "STATUS", "Request a status update", NULL);
        break;
    }
}


static uint16_t status_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_STATUS_MENU_ITEMS;      
}

static void status_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Draw title text in the section header
  menu_cell_basic_header_draw(ctx, cell_layer, "Game Status");
}

static void status_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item we'll draw
    switch (cell_index->row) {
      case 0:
        // This is a basic menu item with a title and subtitle
        menu_cell_basic_draw(ctx, cell_layer, player_name, "Player Name", NULL);
        break;
      case 1:
        // This is a basic menu icon with a cycling icon
        menu_cell_basic_draw(ctx, cell_layer, team_name, "Team Name", NULL);
        break;
      case 2: 
         menu_cell_basic_draw(ctx, cell_layer, num_remain_code_drops, "Code Drops Remaining", NULL);
        break;
      case 3:
        menu_cell_basic_draw(ctx, cell_layer, num_friendly_ops, "Teammates Remaining", NULL);
        break;
      case 4:
        menu_cell_basic_draw(ctx, cell_layer, num_foe_ops, "Opponents Remaining", NULL);
        break;
      case 5:
        menu_cell_basic_draw(ctx, cell_layer, time_remaining, "Minutes Remaining", NULL);
        break;
      case 6:
        menu_cell_basic_draw(ctx, cell_layer, game_id, "Game ID", NULL);
        break;
      case 7:
        menu_cell_basic_draw(ctx, cell_layer, guide_id, "Guide ID", NULL);
        break;
      case 8:
        menu_cell_basic_draw(ctx, cell_layer, "Update", "Update the game status", NULL);
        break;
    }
}

static void starting_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    case 0:
      snprintf(player_name, sizeof(player_name), "%s", "Raunak");
      break;
    case 1:
      snprintf(player_name, sizeof(player_name), "%s", "Samuel");
      break;
    case 2:
      snprintf(player_name, sizeof(player_name), "%s", "Max");
      break;
    case 3:
      snprintf(player_name, sizeof(player_name), "%s", "Travis");
      break;
  }
  window_stack_push(s_team_window, true);
}

static void team_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    case 0:
      snprintf(team_name, sizeof(team_name), "%s", "Artists");
      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0");
      break;
    case 1:
      snprintf(team_name, sizeof(team_name), "%s", "Economists");
      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0");
      break;
    case 2:
      snprintf(team_name, sizeof(team_name), "%s", "Hackers");

      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0");
      break;
    case 3:
      snprintf(team_name, sizeof(team_name), "%s", "Literati");

      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0");
      break;
  }
  window_stack_push(s_game_window, true);
  window_stack_remove(s_starting_window, false);
  window_stack_remove(s_team_window, false);
}

static void game_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  if (cell_index->row == 0) {
    is_neutralize = true;

    PinWindow *neutralize_pin_window = pin_window_create((PinWindowCallbacks) {
      .pin_complete = pin_complete_callback
    }, false);

    pin_window_push(neutralize_pin_window, true);
  }
  else if (cell_index->row == 1) {
    snprintf(capture_msg, 8191, "FA_CAPTURE|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, "0");
    send_app_message(AppKeySendMsg, capture_msg);

    is_capture = true;

    PinWindow *capture_pin_window = pin_window_create((PinWindowCallbacks) {
      .pin_complete = pin_complete_callback
    }, true);

    pin_window_push(capture_pin_window, true);
  }

  else if (cell_index->row == 2) {
    strcpy(status_req, "1");

    snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
    send_app_message(AppKeySendMsg, location_msg);

    strcpy(status_req, "0");  

    window_stack_push(s_status_window, true);
  }

}

static void status_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

  // Use the row to specify which item we'll draw
    if (cell_index->row == 8) {
      strcpy(status_req, "1");

      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0");  

      window_stack_remove(s_status_window, false);
      window_stack_push(s_status_window, true);
    }
  
}

#ifdef PBL_ROUND 
static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) { 
  if (menu_layer_is_index_selected(menu_layer, cell_index)) {
    switch (cell_index->row) {
      case 0:
        return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
        break;
      default:
        return MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT;
    }
  } else {
    return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
  }
}
#endif

static void status_window_load(Window *window) {
   //initialize window layers for name selection window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer and set its callbacks
  s_status_menu_layer = menu_layer_create(bounds);
  menu_layer_set_highlight_colors(s_status_menu_layer, GColorRed, GColorWhite);
  menu_layer_set_callbacks(s_status_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = status_menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(status_menu_draw_header_callback, NULL),
    .draw_row = status_menu_draw_row_callback,
    .select_click = status_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_status_menu_layer, window);

  //add the menu layer as the child of the window layer
  layer_add_child(window_layer, menu_layer_get_layer(s_status_menu_layer));
}

static void status_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_status_menu_layer);

}


static void game_window_load(Window *window) {
  //initialize window layers for gameplay window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer and set its callbacks
  s_game_menu_layer = menu_layer_create(bounds);
  menu_layer_set_highlight_colors(s_game_menu_layer, GColorRed, GColorWhite);
  menu_layer_set_callbacks(s_game_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = game_menu_get_num_sections_callback,
    .get_num_rows = game_menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(game_menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(game_menu_draw_header_callback, NULL),
    .draw_row = game_menu_draw_row_callback,
    .select_click = game_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_game_menu_layer, window);

  //add the menu layer as the child of the window layer
  layer_add_child(window_layer, menu_layer_get_layer(s_game_menu_layer));
}

static void game_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_game_menu_layer);

}

static void team_window_load(Window *window) {
  //initialize window layers for name selection window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer and set its callbacks
  s_team_menu_layer = menu_layer_create(bounds);
  menu_layer_set_highlight_colors(s_team_menu_layer, GColorRed, GColorWhite);
  menu_layer_set_callbacks(s_team_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(team_menu_draw_header_callback, NULL),
    .draw_row = team_menu_draw_row_callback,
    .select_click = team_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_team_menu_layer, window);

  //add the menu layer as the child of the window layer
  layer_add_child(window_layer, menu_layer_get_layer(s_team_menu_layer));
}

static void team_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_team_menu_layer);

}


static void starting_window_load(Window *window) {
  //initialize window layers for name selection window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer and set its callbacks
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorWhite);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL),
    .draw_row = menu_draw_row_callback,
    .select_click = starting_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  //add the menu layer as the child of the window layer
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void starting_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);

  if ((tick_time->tm_sec == 7 || tick_time->tm_sec == 22 || tick_time->tm_sec == 37 || tick_time->tm_sec == 52) && (accel.x > 1 || accel.y > 1 || accel.z > 1)) {
    update_location();
  }


  if ((tick_time->tm_sec % 15 == 0) && strlen(player_name) != 0 && strlen(team_name) != 0) {

      snprintf(location_msg, 8191, "FA_LOCATION|%s|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, latitude, longitude, status_req);
      LOG("%s", location_msg);

      send_app_message(AppKeySendMsg, location_msg);

      strcpy(status_req, "0"); 
  }
}


static void init() {
  // Create Window elements and assign to pointers
  s_starting_window = window_create();  //name selection menu
  s_team_window = window_create();    // team selection menu
  s_game_window = window_create();    // gameplay menu
  s_status_window = window_create();
  LOG("windows created!");

  //  // Use this provider to add button click subscriptions
	// window_set_click_config_provider(s_starting_window, click_config_provider);

  // Set the background color of both the windows
  window_set_background_color(s_starting_window, GColorWhite);
  window_set_background_color(s_game_window, GColorWhite);
  window_set_background_color(s_team_window, GColorWhite);
  window_set_background_color(s_status_window, GColorWhite);

  // Set handlers to manage the elements inside the name seletion Window
  window_set_window_handlers(s_starting_window, (WindowHandlers) {
    .load = starting_window_load,
    .unload = starting_window_unload
  });

  // Set handlers to manage the elements inside the team seletion Window
  window_set_window_handlers(s_team_window, (WindowHandlers) {
    .load = team_window_load,
    .unload = team_window_unload
  });

  // Set handlers to manage the elements inside the gameplay Window
  window_set_window_handlers(s_game_window, (WindowHandlers) {
    .load = game_window_load,
    .unload = game_window_unload
  });

  // Set handlers to manage the elements inside the status Window
  window_set_window_handlers(s_status_window, (WindowHandlers) {
    .load = status_window_load,
    .unload = status_window_unload
  });



  LOG("handlers set!");

  // Show the name selection Window on the watch, with animated=true when game begins
  window_stack_push(s_starting_window, true);
  LOG("name selection window pushed!");

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  accel_data_service_subscribe(0, NULL);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  LOG("APP MESSAGE OPEN!");
}

static void deinit() {
  // Destroy Window
  window_destroy(s_starting_window);
  window_destroy(s_team_window);
  window_destroy(s_game_window);

  tick_timer_service_unsubscribe();

  accel_data_service_unsubscribe();


}

/************************************* main ***********************************/

int main(void) {
  init();
  LOG("field_agent initialized!");
  app_event_loop();
  LOG("field_agent deinitialized!");
  deinit();
}
