/*! @mainpage ProyectoFinal
 *
 * @section genDesc General Description:
            La aplicacion consiste en la lectura de la señal de pulso por PPG, su conversion ADC y su posterior procesamiento
            en el que se calcula la frecuencia cardíaca actual BPM(Beats per minute) y el tiempo entre latidos IBI(Inter-Beats-Interval)
            cada vez que se detecta un latido se envia por la UART el dato acerca del intervalo entre latidos, el cual es recibido por 
            MATLAB, donde se almacenan estos datos en un vector hasta que se reciba un dato de fin de recoleccion. En este momento se procesan
            los datos en MATLAB, se obtiene el tacograma, y se analizan las bandas de bajas y altas frecuencias (LF y HF, respectivamente) y se 
            realiza la division de LF/HF, el cual, si se encuentra debajo de un umbral preestablecido MATLAB consdiera como somnolencia y envía
            por la UART un 1 o un 0 si el valor es mayor al umbral. Ante esta interrupcion, si el dato recibido es un "1" se activa el buzzer, el 
            cual solo se apaga una vez se presiona la tecla 1.
            Al mismo tiempo, se muestra en el display información sobre la frecuencia cardíaca actual y la señal.
            Con la tecla 1 se inicializa el sistema y se envia por la UART una "S", la cual le indica a MATLAB que inicie la recoleccion de datos. 
            1 munuto despues de que se envío la "S", se envia por la UART una "E" que señaliza fin de recoleccion de datos y da inicio al procesamiento
            de los datos recolectados para determinar la presencia o no de somnolencia. Luego de que se envía la "E" se envía una "S" nuevamente para continuar
            con la recolección de datos. Este proceso se repite cada 1 minuto.
            Tambien con la tecla 1, si el buzzer se encuentra encendido, lo apaga. Por otra parte, la tecla 2 se utiliza para apagar el sistema, y se envia
            tamben por la UART una "F" la cual señaliza fin del programa.
 *          
 *          
 *
 * @section hardConn Hardware Connection
 *
 * |  Pulse_Sensor  |     ESP-EDU   |
 * |:--------------:|:--------------|
 * | 	Signal   	|	 CH1		|
 * | 	VCC     	|	 3V3		|
 * | 	GND      	|	 GND		|
 * |:--------------:|:--------------|
 * |   	Display		|    ESP-EDU	|
 * |:--------------:|:--------------|
 * | 	SDO/MISO 	|	 GPIO_22	|
 * | 	LED		 	| 	 3V3		|
 * | 	SCK		 	| 	 GPIO_20	|
 * | 	SDI/MOSI 	| 	 GPIO_21	|
 * | 	DC/RS	 	| 	 GPIO_9		|
 * | 	RESET	 	| 	 GPIO_18	|
 * | 	CS		 	| 	 GPIO_19	|
 * | 	GND		 	| 	 GND		|
 * | 	VCC		 	| 	 3V3		|
 * |:--------------:|:--------------|
 * |   	Buzzer		|    ESP-EDU	|
 * |:--------------:|:--------------|
 * | 	VCC     	|	 GPIO_3		|
 * | 	GND      	|	 GND		|

 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/04/2024 | Document creation		                         |
 *
 * @author Joaquin Palacio
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "pulse_sensor.h"
#include "timer_mcu.h"
#include "buzzer.h"
#include "buzzer_melodies.h"
#include "uart_mcu.h"
#include "switch.h"
#include "led.h"
#include <string.h>
#include "sys/time.h"
#include "iir_filter.h"
#include "gpio_mcu.h"
#include "rtc_mcu.h"
#include "ili9341.h"
#include "roll_plot.h"
#include "heart_pic.h"

/*==================[macros and definitions]=================================*/
#define CHUNK               16 
#define LIGHT_BLUE_COLOR    0x0B2F


/** @def FS
 * @brief Frecuencia en Hz a la que se muestrea la sañal analógica
*/
#define FS 200 

/** @def CONFIG_TIMER_A
 * @brief tiempo en micro segundos del timer_A
*/
#define CONFIG_TIMER_A (1000*1000/FS)

/** @def TRESHOLD
 * @brief umbral para la deteccion de latidos (Vcc/2)
*/
#define TRESHOLD 1650

/** @def MUESTRAS_1_MIN 
 * @brief cantidad de muestras que se toman en 1 minuto de la señal analógica
*/
#define MUESTRAS_1_MIN (1000*1000*60)/ CONFIG_TIMER_A

/** @def PERIODO_REF 
 * @brief periodo refractario para evitar detecciones incorrectas
*/
#define PERIODO_REF (80)

/** @def GPIOBUZZ 
 * @brief GPIO del buzzer
*/
#define GPIOBUZZ GPIO_3

/*==================[internal data declaration]==============================*/


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
TaskHandle_t plot_task_handle = NULL;
TaskHandle_t process_signal_task_handle = NULL;
TaskHandle_t alarm_manage_task_handle = NULL;

/** @struct hr_monitor 
 * @brief monitor de frecuencia cardíaca
*/
HeartRateMonitor hr_monitor = 
{
    .ch = CH1,
    .sampleIntervalMs = (1000/FS),
    .threshSetting = TRESHOLD
};

/** @var ppg 
 * @brief variable en la que se almacena el valor actual de la señal
 * */
uint16_t ppg;

/** @var frecuencia_cardiaca 
 * @brief variable que almacena la frecuencia cardiaca actual
 * */
uint8_t frecuencia_cardiaca;

/** @var on 
 * @brief determina si la aplicación se encuentra en funcionamiento
 * */
bool on = false;

/** @var buzz 
 * @brief guarda informacion sobre si el buzzer se encuentra encendido
 * */
bool buzz;

/** @var somnolencia
 * @brief determina la presencia o no de somnolencia 
 * */
bool somnolencia = false;

/** @var somn
 * @brief caracter recibido por la UART que informa sobre el estado del sujeto ("0" = despierto, "1" = somnolencia) 
 * */
uint8_t somn;

/** @var count
 * @brief contador de la cantidad de muestras tomadas(se reinicia cada 1 minuto) 
 * */
uint32_t count = 0;

/** @var periodo_refractario
 * @brief variable que guarda el valor del periodo refractario, si es igual o menor a 0 quiere decir que se encuentra listo para detectar un nuevo latido 
 * */
int16_t periodo_refractario = 0;

/**
 * @fn void FuncTimerA(void *param)
 * @brief Notifica a la tarea de processSignal
 * @param[in] param puntero tipo void
 */
void FuncTimerA(void* param);

/**
 * @fn void FuncTimerA(void *param)
 * @brief Notifica a la tarea de plot task
 * @param[in] param puntero tipo void
 */
void FuncTimerB(void* param);

/**
 * @fn void FuncTimerA(void *param)
 * @brief funcion de callback para la interrupcion generada por la UART al recibir un mensaje
 * @param[in] param puntero tipo void
 */
void serial_interrupt(void* param);

/** 
* @brief realiza el procesamiento de la nueva muestra (frecuencia cardiaca, intervalo rr, etc).
* @param[in] pvParameter puntero tipo void
*/
static void ProcessSignal(void *pvParameters);

/** 
* @brief realiza el control de la alarma (buzzer)
* @param[in] pvParameter puntero tipo void
*/
static void AlarmManage(void *pvParameters);

/** 
* @brief se encarga de mostrar datos en el display
* @param[in] pvParameter puntero tipo void
*/
static void PlotTask(void *pvParameters);

/**
 * @fn static void LecturaSwitch1 ()
 * @brief Se ejecuta al presionarse la tecla 1, pone on en alto y envia una S por la UART para señalizar inicio de toma de datos, si el buzzer esta encendido, lo apaga
 */
static void LecturaSwitch1();

/**
 * @fn static void LecturaSwitch2 ()
 * @brief Se ejecuta al presionarse la tecla 2, pone on en bajo y enfia una F por la UART para señalizar fin de toma de datos.
 */
static void LecturaSwitch2();
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void FuncTimerA(void* param)
{
    vTaskNotifyGiveFromISR(process_signal_task_handle, pdFALSE);   
}

void serial_interrupt(void* param)
{
    if(on)
    {
        UartReadByte(UART_PC, &somn);
        if(somn == '0')
        {
            somnolencia = 0;
        }
        else if (somn == '1')
        {
            somnolencia = 1;
            xTaskNotifyGive(alarm_manage_task_handle);
        }
    }
}

static void ProcessSignal(void *pvParameters) 
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if(on)
        {
            AnalogInputReadSingle(hr_monitor.ch, &hr_monitor.Signal);
            processLatestSample(&hr_monitor);
            if (count == 0)
            {
                UartSendString(UART_PC, "S\r\n");   
            }
            
            if(sawStartOfBeat(&hr_monitor))
            {
                if(periodo_refractario <= 0 )
                {
                    uint8_t *msg =  UartItoa((uint32_t)getInterBeatIntervalMs(&hr_monitor), 10);
                    UartSendString(UART_PC, (char*)msg);
                    UartSendString(UART_PC, "\r\n");
                    periodo_refractario = PERIODO_REF;
                }
            }
            periodo_refractario--;
            count++;

            if (count == MUESTRAS_1_MIN)
            {
                UartSendString(UART_PC, "E\r\n");
                count = 0;
            }            
        }
    }
}

static void AlarmManage(void *pvParameters) 
{   
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(on)
        {
            if(somnolencia)
            {
                BuzzerOn();
                buzz = true;
            }
        }
    }
}

static void LecturaSwitch1()
{
    if(!on)
    {
        on = true;
        UartSendString(UART_PC, "S\r\n");
    }

    if(buzz)
    {
        BuzzerOff();
        buzz = false;
    }  
}

static void LecturaSwitch2()
{
    if(on)
    {     
        on = false;
        UartSendString(UART_PC, "F\r\n");
    }
}

/**
 * @brief Función ejecutada en la interrupción del Timer
 * 
 */
void FuncTimerSenial(void* param){
    xTaskNotifyGive(plot_task_handle);
}

/**
 * @brief Tarea encargada de filtrar la señal y graficarla en
 * el display LCD.
 * 
 */
static void PlotTask(void *pvParameter){


        static uint8_t indice = 0;
        static char freq[] = "000";
        static char hour_min[] = "00:00";
        static bool beat = true;
        rtc_t actual_time;

        /* Configuración de área de gráfica */
        plot_t plot1 = {
            .x_pos = 0,
            .y_pos = 160,
            .width = 240,
            .height = 100,
            .x_scale = 30,
            .back_color = ILI9341_WHITE
        };
        RTPlotInit(&plot1); 
        /* Configuración de señal a graficar */
        signal_t ecg1 = {
            .y_scale = 40,
            .y_offset = 50,
            .color = ILI9341_RED,
            .x_prev = 0,
            .y_prev = 0
        };
        RTSignalInit(&plot1, &ecg1);

        while(true){
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            frecuencia_cardiaca = getBeatsPerMinute(&hr_monitor);
            ppg = getLatestSample(&hr_monitor);
            ppg = ppg - 1850;
            /* Filtrado de señal */

            /* Graficación de señal */
            for(uint8_t i=0; i<CHUNK; i++){
                RTPlotDraw(&ecg1, ppg);
            }
            indice +=CHUNK;

            if(indice == 0){
                /* Actualización de datos en display */
                ILI9341DrawString(20, 60, freq, &font_89, ILI9341_WHITE, ILI9341_WHITE);
                ILI9341DrawString(10, 8, hour_min, &font_30, LIGHT_BLUE_COLOR, LIGHT_BLUE_COLOR);
                sprintf(freq, "%03i", frecuencia_cardiaca);
                RtcRead(&actual_time);
                sprintf(hour_min, "%02i:%02i", actual_time.hour%MAX_HOUR, actual_time.min%MAX_MIN);
                ILI9341DrawString(20, 60, freq, &font_89, LIGHT_BLUE_COLOR, ILI9341_WHITE);
                ILI9341DrawString(10, 8, hour_min, &font_30, ILI9341_WHITE, LIGHT_BLUE_COLOR);
                if(beat){
                    ILI9341DrawPicture(170, 65, HEART_WIDTH, HEART_HEIGHT, heart);
                }else{
                    ILI9341DrawFilledRectangle(170, 65, 170+HEART_WIDTH, 65+HEART_HEIGHT, ILI9341_WHITE);
                }
                beat = !beat;
            }
        }
}
/*==================[external functions definition]==========================*/


void app_main(void) {
    timer_config_t timer_A = {
        .timer = TIMER_A,
        .period = CONFIG_TIMER_A,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

    timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = (1000*64),
        .func_p = FuncTimerSenial,
        .param_p = NULL
    };

    analog_input_config_t config_ADC = {
    .input = CH1,
    .mode = ADC_SINGLE,
    .func_p = NULL,
    .param_p = NULL,
    .sample_frec = 0
    };

    serial_config_t serial_port = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = &serial_interrupt,
		.param_p = NULL
	};

    UartInit(&serial_port);
    initPulseSensor(&hr_monitor);
    AnalogInputInit(&config_ADC);
    BuzzerInit(GPIOBUZZ);
    SwitchesInit();
    LedsInit();

    SwitchActivInt(SWITCH_1, &LecturaSwitch1, NULL);
    SwitchActivInt(SWITCH_2, &LecturaSwitch2, NULL);

    /* Configuración de display */
    ILI9341Init(SPI_1, GPIO_9, GPIO_18);
	ILI9341Rotate(ILI9341_Portrait_2);
	ILI9341Fill(ILI9341_WHITE);
    ILI9341DrawFilledRectangle(0, 0, 240, 40, LIGHT_BLUE_COLOR);
    ILI9341DrawFilledRectangle(0, 280, 240, 320, LIGHT_BLUE_COLOR);
    ILI9341DrawString(10, 290, "TIME10S", &font_22, ILI9341_WHITE, LIGHT_BLUE_COLOR);
    ILI9341DrawString(178, 290, "00:04", &font_22, ILI9341_WHITE, LIGHT_BLUE_COLOR);
    ILI9341DrawString(178, 120, "bpm", &font_22, LIGHT_BLUE_COLOR, ILI9341_WHITE);
    ILI9341DrawString(20, 60, "000", &font_89, LIGHT_BLUE_COLOR, ILI9341_WHITE);
    ILI9341DrawIcon(170, 8, ICON_BLUETOOTH, &icon_30, ILI9341_WHITE, LIGHT_BLUE_COLOR);
    ILI9341DrawIcon(200, 8, ICON_BAT_3, &icon_30, ILI9341_WHITE, LIGHT_BLUE_COLOR);

    printf("Iniciando sensor...\r\n");
    
    xTaskCreate(&PlotTask, "Plot", 4096, NULL, 3, &plot_task_handle);
    xTaskCreate(&ProcessSignal, "Process Signal", 4096, NULL, 4, &process_signal_task_handle);
    xTaskCreate(&AlarmManage, "Alarm Manage", 2048, NULL, 4, &alarm_manage_task_handle);
    
    TimerInit(&timer_A);
    TimerInit(&timer_senial);

    TimerStart(timer_senial.timer);
    TimerStart(timer_A.timer);

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}
/*==================[end of file]============================================*/






