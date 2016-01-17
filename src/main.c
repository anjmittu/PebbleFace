#include <pebble.h>


static Window *s_main_window;
static TextLayer *s_time_layer;
int prevX = 10000000;
bool started = false;
int wait = 0;

static void data_handler(AccelData *data, uint32_t num_samples) {
	if (wait > 0){
		wait--;
	}else{
		// Long lived buffer
		if (prevX == 10000000){
			prevX = data[0].x;
		}

		else if (abs(prevX-data[0].x) > 400){
			if(started){
				started = false;
				APP_LOG(APP_LOG_LEVEL_INFO, "end");
				APP_LOG(APP_LOG_LEVEL_INFO, "%d", data[0].x);
			}else{
				started = true;
				APP_LOG(APP_LOG_LEVEL_INFO, "start");
				APP_LOG(APP_LOG_LEVEL_INFO, "%d", data[0].x);
			}
			prevX = data[0].x;
			wait = 10;
		}
	}


  //Show the data
}

static void main_window_load(Window *window){
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
	
	text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	
}

static void main_window_unload(Window *window){
	text_layer_destroy(s_time_layer);
}

void init(){
	uint32_t num_samples = 1;
	accel_data_service_subscribe(num_samples, data_handler);

	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	
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
	app_event_loop();
	deinit();
	return 0;
}