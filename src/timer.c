#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static AppTimer *timer_handle;
int prevY = 10000000;
int prevX = 10000000;
int prevZ = 10000000;
bool started = false;
double time_ = 0.0;

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

void inc(void* data){
	time_ += 0.01;
	static char* s_buffer;

  // Compose string of all data for 3 samples
  ftoa(s_buffer, time_, 3);
	
	APP_LOG(APP_LOG_LEVEL_INFO, "%f", time_);

  //Show the data
  text_layer_set_text(s_time_layer, s_buffer);
}

void start(){
	timer_handle = app_timer_register(100, inc, NULL);
}

void stop(){
	app_timer_cancel(timer_handle);
	time_ = 0.0;
}

static void data_handler(AccelData *data, uint32_t num_samples) {
	// Long lived buffer
	if (prevY == 10000000 && prevX == 10000000 && prevZ == 10000000){
		prevY = data[0].y;
		prevX = data[0].x;
		prevZ = data[0].z;
	}

	else if ((abs(data[0].y-data[1].y) < THRESHOLDL && abs(data[2].y-data[3].y) > THRESHOLDH)){
		if(started){
			started = false;
			APP_LOG(APP_LOG_LEVEL_INFO, "end");
			APP_LOG(APP_LOG_LEVEL_INFO, "\n%d\n%d\n", abs(data[0].y-data[1].y), abs(data[2].y-data[3].y));
			vibes_short_pulse();
			//start();
		}else{
			started = true;
			APP_LOG(APP_LOG_LEVEL_INFO, "start");
			APP_LOG(APP_LOG_LEVEL_INFO, "\n%d\n%d\n", abs(data[0].y-data[1].y), abs(data[2].y-data[3].y));
			vibes_long_pulse();
			//stop();
		}
		prevY = data[0].y;
		prevX = data[0].x;
		prevZ = data[0].z;
		psleep(1000);
	}
}


static void main_window_load(Window *window){
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
	
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
	//accel_data_service_subscribe(4, data_handler);

	//accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
	
	s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	window_stack_push(s_main_window, true);
}

void deinit(){
	
}

int main(){
	init();
	start();
	app_event_loop();
	deinit();
	return 0;
}