#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
double time_ = 0, mil = 0;
double user_time = 5.00;
time_t time_1, time_2;
uint16_t mil_1, mil_2;

void ftoa(char* str, double val, int precision) {
  //  start with positive/negative
  if (val < 0) {
    *(str++) = '-';
    val = -val;
  }
  //  integer value
  snprintf(str, 12, "%d", (int) val);
  str += strlen(str);
  val -= (int) val;
  //  decimals
  if ((precision > 0) && (val >= .00001)) {
    //  add period
    *(str++) = '.';
    //  loop through precision
    for (int i = 0;  i < precision;  i++)
      if (val > 0) {
        val *= 10;
        *(str++) = '0' + (int) (val + ((i == precision - 1) ? .5 : 0));
        val -= (int) val;
      } else
        break;
  }
  //  terminate
  *str = '\0';
}

void start( ) {
  /*for (int i = 0; i < 6000; i++){
    static char buf[] = "00000000000";
    ftoa(buf, time_, 3);
    text_layer_set_text(s_time_layer, buf);
    psleep(1);
    time_= time_ + .001;
  }*/
  
  time_ms(&time_1, &mil_1);
  psleep(5500);
  time_ms(&time_2, &mil_2);
  
  time_ = time_2 - time_1;
  mil = mil_2 - mil_1;
  //Extra .004 sec
  time_ = time_ + (mil/1000) - .004;
  
  static char buf[] = "00000000000";
  ftoa(buf, time_, 3);
  text_layer_set_text(s_time_layer, buf);
  
  if (time_ > user_time) {
    window_set_background_color(s_main_window, GColorRed);
    APP_LOG(APP_LOG_LEVEL_INFO, "Too high");
  }
}



static void main_window_load(Window *window){
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
	
  window_set_background_color(window, GColorGreen);
	text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text(s_time_layer, "0.000");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	
}

static void main_window_unload(Window *window){
	text_layer_destroy(s_time_layer);
}

void init(){
	s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	window_stack_push(s_main_window, true);
}

void deinit(){
	window_destroy(s_main_window);
}

int main(){
	init();
  start();
  app_event_loop();
	deinit();
	return 0;
}