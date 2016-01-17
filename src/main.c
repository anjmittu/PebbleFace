
#include <pebble.h>
#include <pebble.h>
#define THRESHOLDL 230
#define THRESHOLDM 450
#define THRESHOLDH 250

static Window *s_main_window;
static TextLayer *s_time_layer;
int prevY = 10000000;
int prevX = 10000000;
int prevZ = 10000000;
bool started = false;
double time_ = 0, mil = 0;
double user_time = 5.00;
time_t time_1, time_2;
uint16_t mil_1, mil_2;
int i = 0;
bool STARTED = false ;
int wait = 0;

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

static void data_handler(AccelData *data, uint32_t num_samples) {
  if (wait > 0){wait--; return;}
  if (STARTED == false){
    //Start reading
  	if (prevY == 10000000 && prevX == 10000000 && prevZ == 10000000){
  		prevY = data[0].y;
  		prevX = data[0].x;
  		prevZ = data[0].z;
      return;
  	}
    //Calibrating or steadying
    if (i < 40 && !(abs(prevY - data[0].y) < THRESHOLDL && abs(prevX - data[0].x) < THRESHOLDL && abs(prevZ - data[0].z) < THRESHOLDL)) {
      //DISPLAY NOT READY
      APP_LOG(APP_LOG_LEVEL_INFO, "not ready");
      i = 0;
      prevY = data[0].y;
  		prevX = data[0].x;
  		prevZ = data[0].z;
      return;
    }
    else { 
      //DISPLAY READY
      i++ ;
      APP_LOG(APP_LOG_LEVEL_INFO, "%d" , i );
    }
    if (i >= 30 && i <= 56 && (abs(prevY - data[0].y) > THRESHOLDH  || abs(prevX - data[0].x) > THRESHOLDH || abs(prevZ - data[0].z) > THRESHOLDH)){
      //TIMER START
      time_ms(&time_1, &mil_1);
  		vibes_short_pulse();
      i = 0;
      STARTED = true;
      wait = 75;
    }
    else if (i >= 55) { i = 30 ; return;}
    else {
      prevY = data[0].y;
  		prevX = data[0].x;
  		prevZ = data[0].z;
      return;
    }
  }
  if ( (abs(prevY - data[0].y) > THRESHOLDM && abs(prevX - data[0].x) > THRESHOLDM && abs(prevZ - data[0].z) > THRESHOLDM) ) {
    //Timer stop
    time_ms(&time_2, &mil_2);
    STARTED = false;
    vibes_long_pulse();
    
    time_ = time_2 - time_1;
    mil = mil_2 - mil_1;
    //Extra .004 sec
    time_ = time_ + (mil/1000) - .004;
  
    static char buf[] = "00000000000";
    ftoa(buf, time_, 3);
    text_layer_set_text(s_time_layer, buf);
  
    if (time_ > user_time) {
      if (watch_info_get_model() == WATCH_INFO_MODEL_PEBBLE_ORIGINAL) {
        window_set_background_color(s_main_window, GColorWhite);
      } else {
        window_set_background_color(s_main_window, GColorRed);
      }
    } else {
      if (watch_info_get_model() == WATCH_INFO_MODEL_PEBBLE_ORIGINAL) {
        window_set_background_color(s_main_window, GColorWhite);
      } else {
        window_set_background_color(s_main_window, GColorGreen);
      }
    }
  }
  
  
  prevY = data[0].y;
	prevX = data[0].x;
	prevZ = data[0].z;
  
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  accel_data_service_unsubscribe();
}

static void main_window_load(Window *window){
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
	
  if (watch_info_get_model() == WATCH_INFO_MODEL_PEBBLE_ORIGINAL) {
    window_set_background_color(window, GColorWhite);
  } else {
    window_set_background_color(window, GColorGreen);
  }
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
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

void init(){
	accel_data_service_subscribe(1, data_handler);

	accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
	
	s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
  window_set_click_config_provider(s_main_window, click_config_provider);
  
	window_stack_push(s_main_window, true);
}

void deinit(){
	
}

int main(){
	init();
	app_event_loop();
	deinit();
	return 0;
}
