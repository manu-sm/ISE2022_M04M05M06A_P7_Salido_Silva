//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* Señales */
#define signal_print_1_line 				0x01  // 001
#define signal_print_2_line  				0x02	// 010

/**
  \fn          init (void)
  \brief       Función que inicializa el driver SPI y configura el lcd para poder usarlo.
*/
void init (void);

/**
  \fn          retardo_1us (void)
  \brief       Función que realiza un retardo de 1 micro segundo para el reseteo del lcd
*/
void retardo_1us (void);

/**
  \fn          retardo_1ms (void)
  \brief       Función que realiza un retardo de 1 mili segundo para el reseteo del lcd
*/
void retardo_1ms (void);

/**
  \fn          LCD_reset (void)
  \brief       Función que se encarga de realizar el reset necesario para que el LCD funcione.
*/
void LCD_reset (void);

/**
  \fn          copy_to_lcd(void)
  \brief       Función que se encarga de pintar en el lcd una secuencia de texto almacenada en el array @param buffer_LCD[]
*/
void copy_to_lcd(void);


void buffer_clear ();

int Init_lcd (void);

