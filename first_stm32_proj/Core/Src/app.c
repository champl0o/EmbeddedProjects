#include "app.h"
#include "main.h"

uint32_t last_btn_clicked;
bool btn_clicked;

void setup(){
	last_btn_clicked = HAL_GetTick();
	btn_clicked = false;
}

void loop(){
	uint32_t now = HAL_GetTick();

	if (now - last_btn_clicked >= DEBOUNCE_MS){
	  if (HAL_GPIO_ReadPin(Button_GPIO_Port, Button_Pin) == 0)
	  {
		  btn_clicked = true;
		  last_btn_clicked = HAL_GetTick();
	  }
	  else if (btn_clicked == true)
	  {
		  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		  btn_clicked = false;
	  }
	}
}
